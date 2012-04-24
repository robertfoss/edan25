#include "bitset.h"


BitSet_struct* bitset_create(){
    BitSet_struct* bs = malloc(sizeof(BitSet_struct));
    bs->list = NULL;
    return 
}


void bitset_or(BitSet_struct* result, BitSet_struct* arg){
    // TODO:
}

void bitset_and_not(BitSet_struct* result, BitSet_struct* arg){
	list_t* result_l = result->list;
    list_t* arg_l = arg->list;

    if(arg_l == NULL)
        return;

    unsigned int arg_offset = arg_l->data->offset;
    unsigned int result_offset = result_l->data->offset;
    while(arg_l->next != arg_l){
        while(result_offset < arg_offset && result_l->next != result_l){
            result_l = result_l->next;
            result_offset = result_l->data->offset;
        }

        if(arg_offset == result_offset){
            result_l->data->bit = nand_bits(result_l->data->bit, arg_l->data->bit);
        } else {
            BitSetSubset_struct* bss = bitsetsubset_create(arg_offset);
            bss->bit = ~ ((unsigned int) 0);
           
            insert_after(result_l, create_node(bss));
            result_l->data->bit 
        }
        
        arg_l = arg_l->next;
        arg_offset = arg_l->data->offset;
    }
}


bool bitset_equals(BitSet_struct* arg1, BitSet_struct* arg2){
	list_t* arg1_l = arg1->list;
    list_t* arg2_l = arg2->list;
    
    if(arg1_l == NULL && arg2_l == NULL)
        return true;
    else if(arg1_l == NULL || arg2_l == NULL)
        return false;

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
    return NULL; // TODO:
}


bool bitset_set_bit(BitSet_struct* bs, unsigned int bit_index, bool bit_val){
    list_t* bs_l = bs->list;
    unsigned int bit_offset = bit_index % SUBSET_BITS;
    unsigned int bit_local_index = (unsigned int) (bit_index / SUBSET_BITS);
    

    unsigned int bs_offset = bs_l->data->offset;
    while(bs_offset < bit_offset && bs_l->next != bs_l){
        bs_l = bs_l->next;
        bs_offset = bs_l->data->offset;
    }
    
    bool old_bit_val;
    if(bit_offset == bs_offset){
        old_bit_val = (bool) bs_l->data->bit & (1 << x);
        bs_l->data->bit &= ~( ((unsigned int) bit_val) << bit_local_index);
    } else {
        BitSetSubset_struct* bss = bitsetsubset_create(arg_offset);
        bss->bit = 0 & ~( ((unsigned int) bit_val) << bit_local_index);
        insert_after(result_l, create_node(bss));
        old_bit_val = false;
    }

    return old_bit_val;
}


bool bitset_get_bit(BitSet_struct* bs, unsigned int bit_index){
    list_t* bs_l = bs->list;
    unsigned int bit_offset = bit_index % SUBSET_BITS;
    unsigned int bit_local_index = (unsigned int) (bit_index / SUBSET_BITS);
    

    unsigned int bs_offset = bs_l->data->offset;
    while(bs_offset < bit_offset && bs_l->next != bs_l){
        bs_l = bs_l->next;
        bs_offset = bs_l->data->offset;
    }
    
    bool old_bit_val;
    if(bit_offset == bs_offset){
        old_bit_val = (bool) bs_l->data->bit & (1 << x);
    } else {
        old_bit_val = false;
    }

    return old_bit_val;
}

void bitset_print(BitSet_struct* bs){
    list_t* bs_l = bs->list;
    if (bs_l == NULL){
        printf("Empty bitset (all zeros).\n");
        return;
    }

    unsigned int last_print_offset = 0;
    unsigned int bs_print_offset = bs->list->data->offset;
    while(bs_l->next != bs_l){
        
        while(last_print_offset < bs_print_offset){
            printf("%d-%d\t|", last_print_offset, last_print_offset + 63);
            for(int j = 0; j < 64; ++j)
                printf("0");
            printf("|\n");
            last_print_offset += 64;
        }

        printf("%d-%d\t|", last_print_offset, last_print_offset + 63);
        for(int i = 0; i < SUBSET_BITS; ++i){
            if((bool) bs_l->data->bit & (1 << x)){
                print("1")
            } else {
                print("0");
            }
        }
        bs_l = bs_l->next;
    }
}

BitSetSubset_struct* bitsetsubset_create(unsigned int offset){
    BitSetSubset_struct* bss = malloc(sizeof(BitSetSubset_struct));
    bss->offset = offset;
    bss->bit = 0;
    return bss;
}

inline
unsigned int nand_bits(unsigned int* uint1, unsigned int* uint2){
    return ~(uint1 & uint2);
}

inline
unsigned int or_bits(unsigned int* uint1, unsigned int* uint2){
    return (uint1 | uint2);
}

