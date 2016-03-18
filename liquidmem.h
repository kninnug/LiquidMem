/**
 * LiquidMem: mildly more efficient memory management.
 * @author  Marco Gunnink <marco@kninnug.nl>
 * @date    2016-03-17
 * @version 1.0.0
 * @file    liquidmem.h
 *
 * See README.md for quick-start info.
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

#ifndef MEMPOOLS_H
#define MEMPOOLS_H

/**
 * A bath holds items of fixed size. Items may be released and re-used or just
 * be feed when the bath is cleared/freed.
 */
typedef struct membath{
	/** The number of items. */
	size_t size;
	/** The size of the items. */
	size_t itemSize;
	/** The number of slots currently in use. */
	size_t length;
	
	/** The lowest indexed free slot. */
	size_t firstFree;
	/** A bit-array of the slots in use. */
	unsigned int * useMap;
	
	/** The slots. */
	char * data;
} membath_s;

/**
 * A pool manages a growing set of baths.
 */
typedef struct mempool{
	/** The number of baths. */
	size_t length;
	/** The size of the baths. */
	size_t bathSize;
	/** The size of the items. */
	size_t itemSize;
	
	/** The baths. */
	membath_s * baths;
} mempool_s;

/**
 * A creek holds items of variable size. Items remain allocated as long as the
 * creek is.
 */
typedef struct memcreek{
	/** The size of the creek. */
	size_t size;
	/** The occupied size. */
	size_t length;
	
	/** The memory. */
	char * data;
} memcreek_s;

/**
 * A river manages a growing set of creeks.
 */
typedef struct memriver{
	/** The number of creeks. */
	size_t length;
	/** The size of the creeks. */
	size_t creekSize;
	
	/** The creeks. */
	memcreek_s * creeks;
} memriver_s;

/**
 * Initialize a bath.
 * 
 * @param bath The bath to initialize.
 * @param size The number of items to hold.
 * @param itemSize The size of the items.
 * @return bath if successful, NULL on error.
 */
membath_s * membath_init(membath_s * bath, size_t size, size_t itemSize);
/**
 * Malloc and initialize a bath.
 *
 * @param size The number of items to hold.
 * @param itemSize The size of the items.
 * @return An initialized bath, NULL if unsuccessful.
 */
membath_s * membath_make(size_t size, size_t itemSize);
/**
 * Reset a bath: release all items.
 *
 * @param bath The bath to reset.
 * @return bath, or NULL if something went wrong.
 */
membath_s * membath_reset(membath_s * bath);
/**
 * De-initialize a bath: release all items and invalidate the storage.
 *
 * @param bath The bath to clear (or flush...).
 * @return bath, or NULL if something went wrong.
 */
membath_s * membath_clear(membath_s * bath);
/**
 * Clear and free a bath that was made with membath_make.
 *
 * @param bath The bath to free, must have been obtained with membath_make.
 */
void membath_free(membath_s * bath);
/**
 * Allocate an item from the bath.
 *
 * @param bath The bath to allocate from.
 * @return A pointer to an item, or NULL if allocation failed (for example: when
 *         the bath is full).
 */
void * membath_alloc(membath_s * bath);
/**
 * Release an item back to the bath.
 *
 * @param bath The bath to release to.
 * @param vptr The item to release, must have been obtained via membath_alloc on
 *             bath.
 * @return bath, or NULL if something went wrong.
 */
membath_s * membath_release(membath_s * bath, void * vptr);

/**
 * Initialize a pool.
 *
 * @param pool The pool to initialize.
 * @param bathSize The number of items per bath.
 * @param itemSize The size of the items.
 * @return pool if successful, NULL on error.
 */
mempool_s * mempool_init(mempool_s * pool, size_t bathSize, size_t itemSize);
/**
 * Malloc an initialize a pool.
 *
 * @param bathSize The number of items per bath.
 * @param itemSize The size of the items.
 * @return An initialized pool, NULL on error.
 */
mempool_s * mempool_make(size_t bathSize, size_t itemSize);
/**
 * Reset a pool: clear the baths to 1 and reset the last remaining one.
 *
 * @param pool The pool to reset.
 * @return pool, or NULL if something went wrong.
 */
mempool_s * mempool_reset(mempool_s * pool);
/**
 * Clear a pool: clear all baths and invalidate the pool.
 *
 * @param pool The pool to clear.
 * @return pool, or NULL if something went wrong.
 */
mempool_s * mempool_clear(mempool_s * pool);
/**
 * Clear an free a pool that was made with mempool_make.
 *
 * @param pool The pool to free, must have been obtained with mempool_make.
 */
void mempool_free(mempool_s * pool);
/**
 * Allocate an item from the pool.
 *
 * @param pool The pool to allocate from.
 * @return A pointer to an item, or NULL on error.
 */
void * mempool_alloc(mempool_s * pool);
/**
 * Release an item back to the pool.
 *
 * @param pool The pool to release to.
 * @param ptr The item to release, must have been obtained via mempool_alloc on
 *            pool.
 * @return pool, or NULL on error.
 */
mempool_s * mempool_release(mempool_s * pool, void * ptr);

/**
 * Initialize a creek.
 *
 * @param creek The creek to initialize.
 * @param size The size of the creek, in bytes.
 * @return creek if successful, NULL on error.
 */
memcreek_s * memcreek_init(memcreek_s * creek, size_t size);
/**
 * Malloc an initialize a creek.
 *
 * @param size The size of the creek, in bytes.
 * @return An initialized creek, or NULL on error.
 */
memcreek_s * memcreek_make(size_t size);
/**
 * Reset a creek: all items allocated become invalid.
 *
 * @param creek The creek to reset.
 * @return creek, or NULL if something went wrong.
 */
memcreek_s * memcreek_reset(memcreek_s * creek);
/**
 * De-initialize a creek: release all items and invalidate storage.
 *
 * @param creek The creek to clear.
 * @return creek, or NULL if something went wrong. 
 */
memcreek_s * memcreek_clear(memcreek_s * creek);
/**
 * Clear and free a creek that was made with memcreek_make.
 *
 * @param creek The creek to free, must have been obtained via memcreek_make.
 */
void memcreek_free(memcreek_s * creek);
/**
 * Allocate an item from the creek.
 *
 * @param creek The creek to allocate from.
 * @param sz The size of the item to allocate.
 * @return A pointer to an item, or NULL on error.
 */
void * memcreek_alloc(memcreek_s * creek, size_t sz);

/**
 * Initialize a river.
 *
 * @param riv The river to initialize.
 * @param creekSize The size of the creeks.
 * @return riv if successfull, NULL on error.
 */
memriver_s * memriver_init(memriver_s * riv, size_t creekSize);
/**
 * Malloc and initialize a river.
 *
 * @param creekSize The size of the creeks.
 * @return An initialized creek, or NULL on error.
 */
memriver_s * memriver_make(size_t creekSize);
/**
 * Allocate an item from the river. The requested size may be larger than the 
 * size of the creeks in this river, in which case 1 new bath will be allocated
 * to fit the requested item exactly.
 * 
 * @param riv The river to allocate from.
 * @param size The size of the item to allocate.
 * @return An item, or NULL on error.
 */
void * memriver_alloc(memriver_s * riv, size_t size);
/**
 * Reset a river: clear all creeks to 1 and reset the remaining one.
 *
 * @param riv The river to reset.
 * @return riv, or NULL on error.
 */
memriver_s * memriver_reset(memriver_s * riv);
/**
 * De-initialize a river: release all items and invalidate the storage.
 *
 * @param riv The river to clear.
 * @return riv, or NULL on error.
 */
memriver_s * memriver_clear(memriver_s * riv);
/**
 * Clear and free a river that was made with memriver_make.
 *
 * @param riv The river to free, must have been obtained via memriver_make.
 */
void memriver_free(memriver_s * riv);

#endif /* MEMPOOLS_H */
