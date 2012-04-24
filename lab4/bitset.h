#ifndef BITSET_H
#define BITSET_H

#include <stdbool.h>
#include "list.h"

#define SUBSET_BITS = (sizeof(unsigned int))


typedef struct BitSetSubset_struct {
	unsigned int offset;
	unsigned int bit;
} BitSetSubset_struct;


typedef struct BitSet_struct {
	list_t* list;
} BitSet_struct;


/** Create and initialize an empty BitSet_struct. The BitSet_struct represents a 
    near infinite set of zero bits.
    Actual max length is sizeof(unsigned int)*(sizeof(size_t)^2 -1). */
BitSet_struct* bitset_create();

/** Logical or of each bit of result and arg, (result:1 || arg:1) is saved into result:1 */
void bitset_or(BitSet_struct* result, BitSet_struct* arg);

/** Logical and not of each bit of result and arg, !(result:1 && arg:1) is saved into result:1 */
void bitset_and_not(BitSet_struct* result, BitSet_struct* arg);

/** Returns true if all bits contained by arg1 and arg2 are equal */ 
bool bitset_equals(BitSet_struct* arg1, BitSet_struct* arg2);

/** Sets the bit at bit_index, while returning the previous value at bit_index */
bool bitset_set_bit(BitSet_struct* bs, unsigned int bit_index, bool bit_val);

/** Returns the value of bit at bit_index */
bool bitset_get_bit(BitSet_struct* bs, unsigned int bit_index);

/** Formats and outputs bs to stout */
void bitset_print(BitSet_struct* bs);

/** Creates a copy of a bitset */
BitSet_struct* bitset_copy(BitSet_struct* arg);
#endif

