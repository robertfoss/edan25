
#include <stdlib.h>
#include <stdio.h>

#include "bitset.h"

inline static
unsigned int nand_bits(unsigned int uint1, unsigned int uint2){
    return ~(uint1 & uint2);
}

inline static
unsigned int or_bits(unsigned int uint1, unsigned int uint2){
    return (uint1 | uint2);
}

BitSetSubset_struct* bitsetsubset_create(unsigned int offset){
    BitSetSubset_struct* bss = malloc(sizeof(BitSetSubset_struct));
    bss->offset = offset;
    bss->bit = 0;
    return bss;
}

BitSet_struct* bitset_create(){
    BitSet_struct* bs = malloc(sizeof(BitSet_struct));
    bs->list = NULL;
    return bs;
}

void bitset_or(BitSet_struct* result, BitSet_struct* arg){
	list_t* result_l = result->list;
    list_t* arg_l = arg->list;

    if(arg_l == NULL)
        return;

    if(result_l == NULL){
        result = bitset_copy(arg);
        return;
    }

    BitSetSubset_struct* result_bss = result_l->data;
    BitSetSubset_struct* arg_bss = arg_l->data;
    unsigned int result_offset = result_bss->offset;
    unsigned int arg_offset = arg_bss->offset;

    do{
        if(arg_offset == result_offset){
            result_bss->bit = or_bits(result_bss->bit, arg_bss->bit);
            result_offset = result_bss->offset;

        }else if(arg_offset > result_offset){
            BitSetSubset_struct* new_bss = bitsetsubset_create(arg_offset);
            new_bss->bit = arg_bss->bit;
            insert_after(result_l, create_node(new_bss));
            result_l = result_l->next; //result_l == new_bss node
            result_l = result_l->next; //result_l == next in original result_l
            result_bss = result_l->data;
            result_offset = result_bss->offset;

        }else{ //arg_offset < result_offset
            arg_l = arg_l->next;
            arg_bss = arg_l->data;
            arg_offset = arg_bss->offset;
        }

    }while(arg_l->next != arg_l && result_l->next != result_l);

    while(arg_l->next != arg_l){ //reached end of result_l but not arg_l
        arg_l = arg_l->next;
        arg_bss = arg_l->data;
        arg_offset = arg_bss->offset;

        BitSetSubset_struct* new_bss = bitsetsubset_create(arg_offset);
        new_bss->bit = arg_bss->bit;

        insert_after(result_l, create_node(new_bss));
        result_l = result_l->next; //result_l == new_bss node (new last in result_l)
    }
}

static
BitSetSubset_struct* find_bss(BitSet_struct* bs, unsigned int offset){
	list_t* bs_l = bs->list;
    short done = 1;
    while(done > 0){
        if (bs_l->next == bs_l)
            --done;
        if ( ((BitSetSubset_struct*) bs_l->data)->offset == offset)
            return (BitSetSubset_struct*) bs_l->data;
        bs_l = bs_l->next;
    }
    return NULL;
}

void bitset_and_not(BitSet_struct* result, BitSet_struct* arg){
	list_t* result_l = result->list;
    list_t* arg_l = arg->list;

    if(arg_l == NULL)
        return;


    BitSetSubset_struct* result_bss = ((BitSetSubset_struct*) result_l->data);
    BitSetSubset_struct* arg_bss = ((BitSetSubset_struct*) arg_l->data);

    unsigned int result_offset = 0;
    unsigned int found_result_offset = result_bss->offset;
    unsigned int max_result_offset = result_bss->offset;

    unsigned int arg_offset = 0;
    unsigned int found_arg_offset = arg_bss->offset;
    unsigned int max_arg_offset = arg_bss->offset;
    
    
    short done = 1;
    while(done > 0){
        if (arg_l->next == arg_l)
            --done;
        max_arg_offset = ((BitSetSubset_struct*) arg_l->data)->offset;
        arg_l = arg_l->next;
    }
    arg_l = arg_l->next;

    done = 1;
    while(done > 0){
        if (result_l->next == result_l)
            --done;
        max_result_offset = ((BitSetSubset_struct*) result_l->data)->offset;
        result_l = result_l->next;
    }
    result_l = result_l->next;

    BitSetSubset_struct* tmp_result_bss;
    BitSetSubset_struct* tmp_arg_bss;
    unsigned int max_offset = (max_arg_offset > max_result_offset) ? max_arg_offset : max_result_offset;
    for(unsigned int i = 0; i < max_offset; i+=SUBSET_BITS){
        tmp_result_bss = find_bss(result, i);
        tmp_arg_bss = find_bss(arg, i);
        if (tmp_result_bss == NULL && tmp_arg_bss == NULL){
            BitSetSubset_struct* bss = bitsetsubset_create(i);
            bss->bit =  ~ 0; //TODO: does this even work?
            insert_after(create_node(bss), result_l);
        } else if (tmp_result_bss == NULL){
            BitSetSubset_struct* bss = bitsetsubset_create(i);
            bss->bit =  ~ ((BitSetSubset_struct*) arg_l->data)->bit; //TODO: does this even work?
            insert_after(create_node(bss), result_l);
        } else if (tmp_arg_bss == NULL){
            tmp_result_bss->bit =  ~ ((BitSetSubset_struct*) result_l->data)->bit; //TODO: does this even work?
        } else {
            BitSetSubset_struct* tmp_r = ((BitSetSubset_struct*) result_l->data);
            BitSetSubset_struct* tmp_a = ((BitSetSubset_struct*) arg_l->data);
            tmp_result_bss->bit = nand_bits(tmp_r->bit, tmp_a->bit);
        }
        result_l = result_l->next;
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
    
    do{
        bss1 = arg1_l->data;
        bss2 = arg2_l->data;
        
        if(bss1->offset != bss2->offset || bss1->bit != bss2->bit)
            return false;
        
        arg1_l = arg1_l->next;
        arg2_l = arg2_l->next;
    }while(arg1_l->next != arg1_l && arg2_l->next != arg2_l);

    if(arg1_l->next == arg1_l && arg2_l->next == arg2_l){
        return true;
    }
    return false;
}


BitSet_struct* bitset_copy(BitSet_struct* arg){
    BitSet_struct* new_bss = bitset_create();

    if(arg == NULL){
        return new_bss;
    }

    list_t* arg_l = arg->list;

    do{
//((BitSetSubset_struct*) bs_l->data)->offset
        BitSetSubset_struct* b_sub = bitsetsubset_create(((BitSetSubset_struct*) arg_l->data)->offset);
        b_sub->bit = ((BitSetSubset_struct*) arg_l->data)->bit;

        if(new_bss->list == NULL){
            new_bss->list = create_node(b_sub);
        }else{
            add_last(new_bss->list, create_node(b_sub));
        }

        arg_l = arg_l->next;

    }while(arg_l->next != arg_l);

    return new_bss;
}


bool bitset_set_bit(BitSet_struct* bs, unsigned int bit_index, bool bit_val){
    list_t* bs_l = bs->list;
    unsigned int bit_offset = (unsigned int)((bit_index / SUBSET_BITS)* SUBSET_BITS);
    unsigned int bit_local_index = (unsigned int) (bit_index % SUBSET_BITS);

//    printf("\nbitset_set_bit: bit_index: %d\tbit_local_index: %d\tbit_offset: %d\n", bit_index, bit_local_index, bit_offset );

    if(bs_l == NULL && bit_val){
//        printf("bitset_set_bit: bs_l == NULL && bit_val == true\n");
        BitSetSubset_struct* bss = bitsetsubset_create(bit_offset);
//        printf("bitset_set_bit:  bit-storage was %u\n", bss->bit);
//        printf("bitset_set_bit:  bit_val was %u\n", (unsigned int) bit_val);
        bss->bit = (1 << bit_local_index);
//        printf("bitset_set_bit:  bit_val<< is %u\n", ((unsigned int) bit_val) << bit_local_index);
//        printf("bitset_set_bit:  bit_val<<~ is %u\n", 0 & ~( ((unsigned int) bit_val) << bit_local_index));
//        printf("bitset_set_bit:  bit-storage<<~&0 is %u\n", bss->bit);
        bss->offset = bit_offset;
        bs->list = create_node(bss);
        return false;
    } else if (bs_l == NULL){
//        printf("bitset_set_bit: bs_l == NULL\n");
        return false;
    }

    unsigned int bss_offset = ((BitSetSubset_struct*) bs_l->data)->offset;
    while(bss_offset < bit_offset && bs_l->next != bs_l){
        bs_l = bs_l->next;
        bss_offset = ((BitSetSubset_struct*) bs_l->data)->offset;
    }
    
    bool old_bit_val;
    if(bit_offset == bss_offset){
        BitSetSubset_struct* bss = ((BitSetSubset_struct*) bs_l->data);
 //       printf("bitset_set_bit: bit_offset == bss_offset\n");
        old_bit_val = (bool) bss->bit & (1 << bit_local_index);
//        printf("bitset_set_bit:  bit-storage was %u\n", ((BitSetSubset_struct*) bs_l->data)->bit);
        bss->bit = bit_val ? (bss->bit | (1 << bit_local_index)) : (bss->bit & ~(1 << bit_local_index)) ;
 //       printf("bitset_set_bit:  bit-storage is %u\n", ((BitSetSubset_struct*) bs_l->data)->bit);
    } else if (bit_val == true) {
 //       printf("bitset_set_bit: bit_offset != bss_offset\n");
        BitSetSubset_struct* bss = bitsetsubset_create(bit_offset);
        bss->bit =  ((unsigned int) bit_val) << bit_local_index;
        bss->offset = bit_offset;
        insert_after(bs_l, create_node(bss));
        old_bit_val = false;
    } else {
        return false;
    }

    return old_bit_val;
}


bool bitset_get_bit(BitSet_struct* bs, unsigned int bit_index){
    list_t* bs_l = bs->list;
    unsigned int bit_offset = bit_index / SUBSET_BITS;
    unsigned int bit_local_index = (unsigned int) (bit_index % SUBSET_BITS);

    unsigned int bs_offset = ((BitSetSubset_struct*) bs_l->data)->offset;
    while(bs_offset < bit_offset && bs_l->next != bs_l){
        bs_l = bs_l->next;
        bs_offset = ((BitSetSubset_struct*) bs_l->data)->offset;
    }
    
    bool old_bit_val;
    if(bit_offset == bs_offset){
        old_bit_val = (bool) (((BitSetSubset_struct*) bs_l->data)->bit & (1 << bit_local_index));
    } else {
        old_bit_val = false;
    }

    return old_bit_val;
}

static 
void bitset_print_helper(list_t* bs_l, unsigned int* last_print_offset, unsigned int* bs_print_offset){

    while(*last_print_offset < *bs_print_offset){
        printf("%4u -%4u\t|", *last_print_offset, (unsigned int) (*last_print_offset + SUBSET_BITS - 1));
        for(int j = 0; j < SUBSET_BITS; ++j)
            printf("0");
        printf("|\n");
        *last_print_offset += SUBSET_BITS;
    }

    printf("%4u -%4u\t|", *bs_print_offset, (unsigned int) (*bs_print_offset + SUBSET_BITS - 1));
    for(int i = 0; i < SUBSET_BITS; ++i){
        if(((BitSetSubset_struct*) bs_l->data)->bit & (1 << i)){
            printf("1");
        } else {
            printf("0");
        }
    }
    printf("|\n");
    *last_print_offset += SUBSET_BITS;
}

void bitset_print(BitSet_struct* bs){
    list_t* bs_l = bs->list;
    if (bs_l == NULL){
        printf("Empty bitset (all zeros).\n");
        return;
    }

    unsigned int last_print_offset = 0;
    unsigned int bs_print_offset = ((BitSetSubset_struct*) bs_l->data)->offset;
    short done = 1;
    while(done > 0){
        if (bs_l->next == bs_l)
            --done;
        
        while(last_print_offset < bs_print_offset){
            printf("%4u -%4u\t|", last_print_offset, (unsigned int) (last_print_offset + SUBSET_BITS - 1));
            for(int j = 0; j < SUBSET_BITS; ++j)
                printf("0");
            printf("|\n");
            last_print_offset += SUBSET_BITS;
        }

        printf("%4u -%4u\t|", bs_print_offset, (unsigned int) (bs_print_offset + SUBSET_BITS - 1));
        for(int i = 0; i < SUBSET_BITS; ++i){
            if(((BitSetSubset_struct*) bs_l->data)->bit & (1 << i)){
                printf("1");
            } else {
                printf("0");
            }
        }
        printf("|\n");
        last_print_offset += SUBSET_BITS;

        bs_l = bs_l->next;
        bs_print_offset = ((BitSetSubset_struct*) bs_l->data)->offset;
    }
}


