LiquidMem: mildly more efficient memory management
==================================================

Quick start
-----------

Copy `liquidmem.c`, `liquidmem.h` and `bitarray.h` into your project directory.
Compile your program with

	cc -std=c99 -O2 yourProgram.c liquidmem.c

C99 is required, the optimization level is of course all up to you, but `-O2`
works well for me.

	#include "liquidmem.h"
	
	int main(){
		size_t poolSize = 1024;
		size_t itemSize = sizeof(int);
		
		// make pool: 1024 items of sizeof(int) bytes per bath
		mempool_s * pool = mempool_make(poolSize, itemSize);
		// allocate item
		int * ptr = mempool_alloc(pool);
		// use item...
		*ptr = 42;
		// release item (optional)
		mempool_release(pool, ptr);
		// clean up pool
		mempool_free(pool);
		
		// make river: each creek is 1024 bytes
		memriver_s * river = memriver_make(poolSize);
		// allocate item
		char * str = memriver_alloc(river, 42);
		// use item..
		strcpy(str, "Hello world!");
		// clean up river (and free items)
		memriver_free(river);
		
		return 0;
	}

The library exposes 4 data structures: `mempool`, `membath`, `memriver` and 
`memcreek`. While the names are somewhat creative, the main distinction is only
twofold. For most purposes you only need to concern yourself with the two 
managing structures: pools and rivers. Pools consist of 1 or more baths and 
rivers consist of 1 or more creeks, but using those sub-types directly shouldn't
be necessary.

The difference between pools and rivers is that a pool allocates items of fixed
size that has to be supplied during the creation of the pool. However, pools 
also allow items to be released back to the pool so the space can be re-used. A 
river allows for the allocation of items of variable size, but they cannot be 
released: the space remains in use as long as the river is allocated.

For both pools and rivers the size of the sub-structures (baths and creeks,
respectively) is supplied at creation time. But more items can be allocated: the
pool/river will automatically create more baths/creeks to accomodate the items,
provided memory is available. Additionally, rivers also support allocating 
single items that are bigger than ordinary creeks (provided enough memory is 
available of course): they will allocate a single creek for that item.

Due to the overhead of allocating releasable items, pools are not as 
performance-efficient as rivers. So consider using rivers even when you are
only allocating items of a fixed size, if internal memory leakage is not a
concern. Nevertheless, if the size of the pool is chosen well, i.e. close to
the actual number of items required, it is still faster than ordinary malloc/
free.

Note: `mempool_release`/`membath_release` may (or more technically: do) invoke
undefined behaviour. In order to determine to which bath a pointer to release
belongs, the pointer is compared against the start and end point of the
bath's storage. Using `<` and `>` on two pointers to different objects is 
undefined. If this is a problem, you may define `USE_INTPTR` when compiling
`liquidmem.c`. This will convert the pointers to `intptr_t`s and compare those,
which should be safe. According to the spec, this type is optional, so make
sure your implementation supports it.
