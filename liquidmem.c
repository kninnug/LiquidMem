/**
 * LiquidMem: mildly more efficient memory management.
 * @author  Marco Gunnink <marco@kninnug.nl>
 * @date    2016-03-17
 * @version 1.0.0
 * @file    liquidmem.c
 *
 * See README.md for quick-start info and liquidmem.h for doc-comments.
 *
 * License: MIT
 *
 * Copyright (c) 2016 Marco Gunnink
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 * 
 * The software is provided "as is", without warranty of any kind, express or
 * implied, including but not limited to the warranties of merchantability,
 * fitness for a particular purpose and noninfringement. In no event shall the
 * authors or copyright holders be liable for any claim, damages or other
 * liability, whether in an action of contract, tort or otherwise, arising from,
 * out of or in connection with the software or the use or other dealings in
 * the software.
 */

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <limits.h>
#include <stddef.h>
#include <string.h>

#include "bitarray.h"

#include "liquidmem.h"

/*
 * Bath functions
 */

membath_s * membath_init(membath_s * bath, size_t size, size_t itemSize){
	bath->size = size;
	bath->itemSize = itemSize;
	bath->length = 0;
	bath->firstFree = 0;
	
	size_t bitSize = bitArray_size(bath->size);
	bath->useMap = calloc(bitSize, sizeof *bath->useMap);
	if(!bath->useMap){
		return NULL;
	}
	
	bath->data = malloc(size * itemSize);
	if(!bath->data){
		return NULL;
	}
	
	return bath;
}

membath_s * membath_make(size_t size, size_t itemSize){
	membath_s * ret = malloc(sizeof *ret);
	if(!ret){
		return NULL;
	}
	
	return membath_init(ret, size, itemSize);
}

membath_s * membath_reset(membath_s * bath){
	bath->length = 0;
	bath->firstFree = 0;
	
	bitArray_zeroe(bath->useMap, bath->size);
	
	return bath;
}

membath_s * membath_clear(membath_s * bath){
	free(bath->data);
	free(bath->useMap);
	
	bath->data = NULL;
	return bath;
}

void membath_free(membath_s * bath){
	membath_clear(bath);
	free(bath);
}

void * membath_alloc(membath_s * bath){
	if(bath->length >= bath->size){
		return NULL;
	}
	
	size_t item = bath->firstFree;
	bitArray_set(bath->useMap, item);
	for(size_t i = item; i < bath->size; i++){
		if(!bitArray_test(bath->useMap, i)){
			bath->firstFree = i;
			break;
		}
	}
	
	bath->length++;
	
	return bath->data + item * bath->itemSize;
}

membath_s * membath_release(membath_s * bath, void * vptr){
	char * ptr = vptr;
	char * end = bath->data + bath->size * bath->itemSize;
	
#ifdef USE_INTPTR
	intptr_t * iptr = vptr,
		* start = (intptr_t *)bath->data,
		* iend = (intptr_t *)end;
	
	if(iptr < start || iptr > iend){
		return NULL;
	}
#else /* !USE_INTPTR */
	if(ptr < bath->data || ptr > end){
		return NULL;
	}
#endif /* USE_INTPTR */
	
	ptrdiff_t itemOffset = ptr - bath->data;
	
	if(itemOffset < 0 || itemOffset > bath->size * bath->itemSize){
		return NULL;
	}
	
	size_t item = itemOffset / bath->itemSize;
	if(item < bath->firstFree){
		bath->firstFree = item;
	}
	
	bitArray_clear(bath->useMap, item);
	bath->length--;
	
	return bath;
}

/*
 * Pool functions
 */

mempool_s * mempool_init(mempool_s * pool, size_t bathSize, size_t itemSize){
	pool->length = 1;
	pool->bathSize = bathSize;
	pool->itemSize = itemSize;
	
	pool->baths = malloc(pool->length * sizeof *pool->baths);
	if(!pool->baths){
		return NULL;
	}
	
	if(!membath_init(pool->baths, bathSize, itemSize)){
		return NULL;
	}
	
	return pool;
}

mempool_s * mempool_make(size_t bathSize, size_t itemSize){
	mempool_s * ret = malloc(sizeof *ret);
	if(!ret){
		return NULL;
	}
	
	return mempool_init(ret, bathSize, itemSize);
}

mempool_s * mempool_reset(mempool_s * pool){
	while(pool->length --> 1){
		membath_clear(pool->baths + pool->length);
	}
	
	pool->length = 1;
	
	membath_s * bths = realloc(pool->baths, pool->length * sizeof *pool->baths);
	if(!bths){       // The realloc shouldn't ever alloc more than previous
		return NULL; // size, so this shouldn't really happen. Still...
	}
	
	pool->baths = bths;
	membath_reset(pool->baths);
	
	return pool;
}

mempool_s * mempool_clear(mempool_s * pool){
	while(pool->length--){
		membath_clear(pool->baths + pool->length);
	}
	
	free(pool->baths);
	pool->length = 0;
	pool->baths = NULL;
	
	return pool;
}

void mempool_free(mempool_s * pool){
	mempool_clear(pool);
	free(pool);
}

void * mempool_alloc(mempool_s * pool){
	void * ret = membath_alloc(pool->baths + pool->length - 1);
	
	if(ret){
		return ret;
	}
	
	size_t len = ++pool->length;
	membath_s * bths = realloc(pool->baths, len * sizeof *pool->baths);
	
	if(!bths){
		return NULL;
	}
	pool->baths = bths;
	
	membath_init(pool->baths + len - 1, pool->bathSize, pool->itemSize);
	
	ret = membath_alloc(pool->baths + len - 1);
	
	return ret;
}

mempool_s * mempool_release(mempool_s * pool, void * ptr){
	if(!ptr){
		return NULL;
	}
	
	for(size_t i = 0; i < pool->length; i++){
		membath_s * bath = pool->baths + i;
		if(membath_release(bath, ptr) == bath){
			return pool;
		}
	}
	
	return NULL;
}

/*
 * Creek functions
 */

memcreek_s * memcreek_init(memcreek_s * creek, size_t size){
	creek->size = size;
	creek->length = 0;
	
	creek->data = malloc(size);
	if(!creek->data){
		return NULL;
	}
	
	return creek;
}

memcreek_s * memcreek_make(size_t size){
	memcreek_s * ret = malloc(sizeof *ret);
	
	if(!ret){
		return NULL;
	}
	
	return memcreek_init(ret, size);
}

memcreek_s * memcreek_clear(memcreek_s * creek){
	free(creek->data);
	creek->data = NULL;
	creek->length = 0;
	
	return creek;
}

void memcreek_free(memcreek_s * creek){
	memcreek_clear(creek);
	free(creek);
}

memcreek_s * memcreek_reset(memcreek_s * creek){
	creek->length = 0;
	
	return creek;
}

void * memcreek_alloc(memcreek_s * creek, size_t sz){
	if(creek->size - creek->length < sz){
		return NULL;
	}
	
	void * ret = creek->data + creek->length;
	creek->length += sz;
	
	return ret;
}

/*
 * River functions
 */

memriver_s * memriver_init(memriver_s * riv, size_t creekSize){
	riv->creekSize = creekSize;
	riv->length = 1;
	
	riv->creeks = malloc(riv->length * sizeof *riv->creeks);
	if(!riv->creeks){
		return NULL;
	}
	memcreek_init(riv->creeks, creekSize);
	
	return riv;
}

memriver_s * memriver_make(size_t creekSize){
	memriver_s * ret = malloc(sizeof *ret);
	
	if(!ret){
		return NULL;
	}
	
	return memriver_init(ret, creekSize);
}

memriver_s * memriver_clear(memriver_s * riv){
	while(riv->length--){
		memcreek_clear(riv->creeks + riv->length);
	}
	
	free(riv->creeks);
	riv->creeks = NULL;
	riv->length = 0;
	
	return riv;
}

void memriver_free(memriver_s * riv){
	memriver_clear(riv);
	free(riv);
}

memriver_s * memriver_reset(memriver_s * riv){
	while(riv->length --> 1){
		memcreek_clear(riv->creeks + riv->length);
	}
	
	riv->length = 1;
	
	memcreek_s * crks = realloc(riv->creeks, riv->length * sizeof *riv->creeks);
	if(!crks){       // The realloc shouldn't ever alloc more than previous
		return NULL; // size, so this shouldn't really happen. Still...
	}
	
	riv->creeks = crks;
	memcreek_reset(riv->creeks);
	
	return riv;
}

static memcreek_s * addCreek(memriver_s * riv, size_t size){
	size_t len = ++riv->length;
	memcreek_s * crks = realloc(riv->creeks, len * sizeof *riv->creeks);
	
	if(!crks){
		return NULL;
	}
	riv->creeks = crks;
	memcreek_init(riv->creeks + len - 1, size);
	
	return riv->creeks + len - 1;
}

void * memriver_alloc(memriver_s * riv, size_t size){
	void * ret = NULL;
	
	// size requested exceeds creek size: allocate 1 creek of exactly that size
	if(size > riv->creekSize){
		memcreek_s * crk = addCreek(riv, size);
		if(crk){
			return memcreek_alloc(crk, size);
		}else{
			return NULL;
		}
	}
	
	// go over the creeks last to first to find a spot (last creek is more 
	// likely not to be full)
	for(size_t i = riv->length; i > 0; i--){
		ret = memcreek_alloc(riv->creeks + i - 1, size);
		if(ret){
			break;
		}
	}
	
	// no existing creek has enough space available, make a new one
	if(!ret){
		memcreek_s * crk = addCreek(riv, riv->creekSize);
		if(crk){
			return memcreek_alloc(crk, size);
		}else{
			return NULL;
		}
	}
	
	return ret;
}
