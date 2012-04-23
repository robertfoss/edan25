#include "bitset.h"


BitSet_struct* bitset_create(){
    BitSet_struct* bs = malloc(sizeof(BitSet_struct));
    bs->list = NULL;
    return 
}


void bitset_or(BitSet_struct* result, BitSet_struct* arg){

}

void bitset_and_not(BitSet_struct* result, BitSet_struct* arg){

}


bool bitset_equals(BitSet_struct* arg1, BitSet_struct* arg2){
	list_t* arg1_l = arg1->list;
    list_t* arg2_l = arg2->list;
    
    if(arg1_l == NULL && arg2_l == NULL)
        return true;
    else if(arg1_l == NULL || arg2_l == NULL)
        reutrn false;

    BitSetSubset_struct* bss1 = arg1_l->data;
    BitSetSubset_struct* bss2 = arg2_l->data;
    
    while(arg1_l != NULL){
        bss1 = arg1_l->data;
        bss2 = arg2_l->data;
        
        if(bss1->offset != bss2->offset || bss1->bit != bss2->bit)
            return false;
        
        arg1_l = arg1_l->next;
        arg2_l = arg2_l->next;
    }
    return true;
}


BitSet_struct* bitset_copy(BitSet_struct* arg){

}


bool bitset_set_bit(BitSet_struct* bs, unsigned int bit_index){
    return true;
}


bool bitset_get_bit(BitSet_struct* bs, unsigned int bit_index){
    return true;
}

BitSetSubset_struct* bitsetsubset_create(unsigned int offset){
    BitSetSubset_struct* bss = malloc(sizeof(BitSetSubset_struct));
    bss->offset = offset;
    bss->bit = 0;
    return bss;
}

unsigned int nand_bits(unsigned int* uint1, unsigned int* uint2){
    return ~(uint1 & uint2);
}

unsigned int or_bits(unsigned int* uint1, unsigned int* uint2){
    return (uint1 | uint2);
}

