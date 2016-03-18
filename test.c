#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <assert.h>
#include <string.h>

#include "liquidmem.h"

/* Make a mempool and alloc n items. */
static mempool_s * benchMempoolAlloc(size_t n, int * data[], unsigned int div){
	size_t poolz = n / div;
	size_t itemSz = sizeof(int);
	size_t i;
	
	mempool_s * pool = mempool_make(poolz, itemSz);
	for(i = 0; i < n; i++){
		data[i] = mempool_alloc(pool);
		*data[i] = i;
	}
	
	return pool;
}

/* Release all items from a mempool, one-by-one. */
static void benchMempoolRelease(mempool_s * pool, size_t n, int * data[]){
	size_t i;
	
	for(i = 0; i < n; i++){
		assert(mempool_release(pool, data[i]));
		data[i] = NULL;
	}
}

/* Release n/2 random elements from a mempool and re-alloc them. */
static void benchMempoolReuse(mempool_s * pool, size_t n, int * data[]){
	size_t m = n / 2;
	size_t i, r;
	size_t released[m];
	memset(released, 0, sizeof released);
	
	for(i = 0; i < m; i++){
		r = rand() % n;
		if(data[r]){
			released[i] = r;
			mempool_release(pool, data[r]);
			data[r] = NULL;
		}
	}
	
	for(i = 0; i < m; i++){
		r = released[i];
		if(!data[r]){
			data[r] = mempool_alloc(pool);
			*data[r] = r;
		}
	}
}

/* Make a memriver and alloc n items. */
static memriver_s * benchMemriverAlloc(size_t n, int * data[], unsigned int div){
	size_t poolz = n / div;
	size_t itemSz = sizeof(int);
	size_t i;
	
	memriver_s * riv = memriver_make(itemSz * poolz);
	for(i = 0; i < n; i++){
		data[i] = memriver_alloc(riv, itemSz);
		*data[i] = i;
	}
	
	return riv;
}

/* Malloc n items. */
static void benchMalloc(size_t n, int * data[]){
	size_t itemSz = sizeof(int);
	size_t i;
	
	for(i = 0; i < n; i++){
		data[i] = malloc(itemSz);
		*data[i] = i;
	}
}

/* Free n items. */
static void benchFree(size_t n, int * data[]){
	size_t i;
	
	for(i = 0; i < n; i++){
		free(data[i]);
		data[i] = NULL;
	}
}

/* Free n/2 random items an re-malloc them. */
static void benchRemalloc(size_t n, int * data[]){
	size_t itemSz = sizeof(int);
	size_t m = n / 2;
	size_t i, r;
	size_t released[m];
	memset(released, 0, sizeof released);
	
	for(i = 0; i < m; i++){
		r = rand() % n;
		if(data[r]){
			released[i] = r;
			free(data[r]);
			data[r] = NULL;
		}
	}
	
	for(i = 0; i < m; i++){
		r = released[i];
		if(!data[r]){
			data[r] = malloc(itemSz);
			*data[r] = r;
		}
	}
}

/* Check if the allocated data is consistent: data[i] == i for i = 0 to n. */
static int checkData(size_t n, int * data[]){
	for(size_t i = 0; i < n; i++){
		if(data[i] && *data[i] != i){
			printf("@ %i: %i\n", i, *data[i]);
			return 0;
		}
	}
	
	return 1;
}

int main(int argc, char ** argv){
	unsigned int mult = 2, div = 4;
	int doRelease = 1, doReuse = 1;
	
	/* Multiplier: do mult * 1024 rounds of mult * 1024 items. */
	if(argc > 1){
		mult = strtoul(argv[1], NULL, 10);
	}
	/* Divider: make pools/rivers of n / div items. */
	if(argc > 2){
		div = strtoul(argv[2], NULL, 10);
	}
	/* Don't bench mempool_release. */
	if(argc > 3){
		doRelease = argv[3][0] != 'n';
	}
	/* Don't bench pool release & re-alloc. */
	if(argc > 4){
		doReuse = argv[4][0] != 'n';
	}
	
	size_t m = 1024 * mult;
	size_t n = 1024 * mult;
	int * data[n];
	size_t i;
	clock_t start, end;
	double mempoolTime, memriverTime, mallocTime;
	
	srand(time(NULL));
	
	/* Malloc/free */
	start = clock();
	for(i = 0; i < m; i++){
		benchMalloc(n, data);
		benchRemalloc(n, data);
		assert(checkData(n, data));
		benchFree(n, data);
	}
	end = clock();
	mallocTime = (double)(end - start) / (double)CLOCKS_PER_SEC;
	printf("malloc  : (%lux%lu): %9f sec, ratio: %9f\n", m, n, 
			mallocTime, mallocTime / mallocTime);
	
	/* Pools */
	start = clock();
	for(i = 0; i < m; i++){
		mempool_s * pool = benchMempoolAlloc(n, data, div);
		if(doReuse){
			benchMempoolReuse(pool, n, data);
		}
		assert(checkData(n, data));
		if(doRelease){
			benchMempoolRelease(pool, n, data);
		}
		mempool_free(pool);
	}
	end = clock();
	mempoolTime = (double)(end - start) / (double)CLOCKS_PER_SEC;
	printf("mempool : (%lux%lu): %9f sec, ratio: %9f\n", m, n, 
			mempoolTime, mallocTime / mempoolTime);
	
	/* Rivers */
	start = clock();
	for(i = 0; i < m; i++){
		memriver_s * river = benchMemriverAlloc(n, data, div);
		assert(checkData(n, data));
		memriver_free(river);
	}
	end = clock();
	memriverTime = (double)(end - start) / (double)CLOCKS_PER_SEC;
	printf("memriver: (%lux%lu): %9f sec, ratio: %9f\n", m, n, 
			memriverTime, mallocTime / memriverTime);
	
	return 0;
}
