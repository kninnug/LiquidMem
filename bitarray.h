/**
 * Bit-arrays using unsigned ints. Implemented entirely in macros.
 * @author  Marco Gunnink <marco@kninnug.nl>
 * @date    2016-03-17
 * @version 1.0.0
 * @file    bitarray.h
 *
 * Inspired by http://c-faq.com/misc/bitsets.html & xtrapbits.h
 *
 * As far as it isn't copyrighted by anyone else, these bits are in the public
 * domain.
 */

#ifndef BITARRAY_H
#define BITARRAY_H

#include <stddef.h> /* CHAR_BIT */
#include <string.h> /* memset */

/** @private Number of bits in an unsigned int. */
#define BITARRAY_INTBITS (sizeof(unsigned int)*CHAR_BIT)

/**
 * Mask for a bit: the relative bit-position within the int.
 *
 * @param bit The bit.
 * @return Its mask.
 */
#define bitArray_mask(bit) (1 << ((bit) % BITARRAY_INTBITS))

/**
 * Slot for a bit: the index in the array.
 *
 * @param bit The bit.
 * @return Its slot.
 */
#define bitArray_slot(bit) ((bit) / BITARRAY_INTBITS)

/**
 * Returns the size of the array needed to accomodate the bits.
 *
 * @param sz The number of bits.
 * @return The size of the int array.
 */
#define bitArray_size(sz) (((sz) + BITARRAY_INTBITS - 1) / BITARRAY_INTBITS)

/**
 * Returns the mask of bit if it is set, 0 otherwise.
 *
 * @param ar The array.
 * @param bit The bit.
 * @return 0 if the bit is not set, >0 if it is.
 */
#define bitArray_test(ar, bit)                                                 \
		((ar)[bitArray_slot(bit)] & bitArray_mask(bit))

/**
 * Sets a bit to 1.
 *
 * @param ar The array.
 * @param bit The bit to set.
 * @return The new value of the bit's slot.
 */
#define bitArray_set(ar, bit)                                                  \
		((ar)[bitArray_slot(bit)] |= bitArray_mask(bit))

/** Alias for bitArray_set. */
#define bitArray_on(ar, bit) bitArray_set(ar, bit)
/** Alias for bitArray_set. */
#define bitArray_one(ar, bit) bitArray_set(ar, bit)

/**
 * Clears a bit to 0.
 *
 * @param ar The array.
 * @param bit The bit to clear.
 * @return The new value of the bit's slot.
 */
#define bitArray_clear(ar, bit)                                                \
		((ar)[bitArray_slot(bit)] &= ~bitArray_mask(bit))

/** Alias for bitArray_clear. */
#define bitArray_off(ar, bit) bitArray_clear(ar, bit)
/** Alias for bitArray_clear. Not to be confused with bitArray_zeroe. */
#define bitArray_zero(ar, bit) bitArray_clear(ar, bit)

/**
 * Flips a bit: set to 0 if it's 1, or vice versa.
 * 
 * @param ar The array.
 * @param bit The bit to flip.
 * @return The new value of the bit's slot.
 */
#define bitArray_flip(ar, bit)                                                 \
		((ar)[bitArray_slot(bit)] ^= bitArray_mask(bit))

/** Alias for bitArray_flip. */
#define bitArray_toggle(ar, bit) bitArray_flip(ar, bit)

/**
 * Compute the union of two bit arrays.
 * 
 * @param sz The size of both arrays, in bits.
 * @param ar1 Array 1, also the output.
 * @param ar2 Array 2.
 * @return void.
 */
#define bitArray_union(sz, ar1, ar2)                                           \
		do{                                                                    \
			for(size_t bitArray__i = 0; bitArray__i < bitArray_size(sz); bitArray__i++){ \
				(ar1)[bitArray__i] |= (ar2)[bitArray__i];                      \
			}                                                                  \
		}while(0)

/**
 * Compute the intersection of two bit arrays.
 * 
 * @param sz The size of both arrays, in bits.
 * @param ar1 Array 1, also the output.
 * @param ar2 Array 2.
 * @return void.
 */
#define bitArray_intersect(sz, ar1, ar2)                                       \
		do{                                                                    \
			for(size_t bitArray__i = 0; bitArray__i < bitArray_size(sz); bitArray__i++){ \
				(ar1)[bitArray__i] &= (ar2)[bitArray__i];                      \
			}                                                                  \
		}while(0)

/**
 * Declare a bit-array with given name and number of bits. Contents are not
 * initialized. Safe for use in struct- or union-definitions.
 * 
 * @param name The name.
 * @param sz The size, in bits.
 * @return declaration.
 */
#define bitArray_declare(name, sz)                                             \
		unsigned int name[bitArray_size(sz)]
		
/**
 * Declare and initialize a bit-array with given name and number of bits. Not
 * safe for use in struct- or union-definitions.
 *
 * @param name The name.
 * @param sz The size, in bits.
 * @return declaration & initialization.
 */
#define bitArray_init(name, sz)                                                \
		bitArray_declare(name, sz) = {0}

/**
 * Set all bits in the bit-array to 0. Not to be confused with bitArray_zero.
 *
 * @param ar The bit-array.
 * @param sz The size of the bit-array, in bits.
 * @return void.
 */
#define bitArray_zeroe(ar, sz)                                                  \
		memset(ar, 0, bitArray_size(sz))

#endif /* BITARRAY_H */
