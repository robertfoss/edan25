#include <assert.h>
#include <stdbool.h>

#include "bitset.h"


void test_bitset_or(){
    printf("Test bitset_or(): \t");

    printf("2 bitsets of differing length. \t\t");
    BitSet_struct* bs1 = bitset_create();
    bitset_set_bit(bs1, 1, true);
    bitset_set_bit(bs1, 5, true);
    bitset_set_bit(bs1, 20, true);

    BitSet_struct* bs2 = bitset_create();
    bitset_set_bit(bs2, 1, true);
    bitset_set_bit(bs2, 3, true);
    bitset_set_bit(bs2, 61, true);

    bitset_or(bs1, bs2);

    assert(bitset_get_bit(bs1,0) == false);
    assert(bitset_get_bit(bs1,1) == true);
    assert(bitset_get_bit(bs1,2) == false);
    assert(bitset_get_bit(bs1,3) == true);
    assert(bitset_get_bit(bs1,4) == false);
    assert(bitset_get_bit(bs1,5) == true);
    assert(bitset_get_bit(bs1,6) == false);
    assert(bitset_get_bit(bs1,60) == false);
    assert(bitset_get_bit(bs1,61) == true);
    assert(bitset_get_bit(bs1,62) == false);
    printf("PASSED\n");

    printf("\t\t\tsparse bitset with regular bitset. \t");
    bs1 = bitset_create();
    bitset_set_bit(bs1, 55, true);

    bs2 = bitset_create();
    bitset_set_bit(bs2, 1, true);
    bitset_set_bit(bs2, 3, true);
    bitset_set_bit(bs2, 61, true);

    bitset_or(bs1, bs2);
    assert(bitset_get_bit(bs1,0) == false);
    assert(bitset_get_bit(bs1,1) == true);
    assert(bitset_get_bit(bs1,2) == false);
    assert(bitset_get_bit(bs1,3) == true);
    assert(bitset_get_bit(bs1,4) == false);
    assert(bitset_get_bit(bs1,54) == false);
    assert(bitset_get_bit(bs1,55) == true);
    assert(bitset_get_bit(bs1,56) == false);
    assert(bitset_get_bit(bs1,60) == false);
    assert(bitset_get_bit(bs1,61) == true);
    assert(bitset_get_bit(bs1,62) == false);
    printf("PASSED\n");


    printf("\t\t\tsparse bitset with very long arg. \t");
    bs1 = bitset_create();
    bitset_set_bit(bs1, 55, true);

    bs2 = bitset_create();
    bitset_set_bit(bs2, 1, true);
    bitset_set_bit(bs2, 3, true);
    bitset_set_bit(bs2, 61, true);
    bitset_set_bit(bs2, 127, true);
    bitset_set_bit(bs2, 127+SUBSET_BITS, true);
    bitset_set_bit(bs2, 127+2*SUBSET_BITS, true);
    bitset_set_bit(bs2, 127+4*SUBSET_BITS, true);

    bitset_or(bs1, bs2);
    assert(bitset_get_bit(bs1,0) == false);
    assert(bitset_get_bit(bs1,1) == true);
    assert(bitset_get_bit(bs1,2) == false);
    assert(bitset_get_bit(bs1,3) == true);
    assert(bitset_get_bit(bs1,4) == false);
    assert(bitset_get_bit(bs1,54) == false);
    assert(bitset_get_bit(bs1,55) == true);
    assert(bitset_get_bit(bs1,56) == false);
    assert(bitset_get_bit(bs1,60) == false);
    assert(bitset_get_bit(bs1,61) == true);
    assert(bitset_get_bit(bs1,62) == false);

    assert(bitset_get_bit(bs1, 127+SUBSET_BITS) == true);
    assert(bitset_get_bit(bs1, 127+2*SUBSET_BITS) == true);
    assert(bitset_get_bit(bs1, 127+3*SUBSET_BITS) == false);
    assert(bitset_get_bit(bs1, 127+4*SUBSET_BITS) == true);
    printf("PASSED\n");
}

void test_bitset_and_not(){
    printf("Test bitset_not_and(): \t");

    printf("2 bitsets of differing length. \t\t");
    BitSet_struct* bs1 = bitset_create();
    bitset_set_bit(bs1, 3, true);
    bitset_set_bit(bs1, 5, true);
    bitset_set_bit(bs1, 36, true);

    BitSet_struct* bs2 = bitset_create();
    bitset_set_bit(bs2, 1, true);
    bitset_set_bit(bs2, 3, true);
    bitset_set_bit(bs2, 6, true);
    bitset_set_bit(bs2, 33, true);
    bitset_set_bit(bs2, 65, true);

    bitset_and_not(bs1, bs2);

    assert(bitset_get_bit(bs1,0) == false);
    assert(bitset_get_bit(bs1,1) == false);
    assert(bitset_get_bit(bs1,3) == false);
    assert(bitset_get_bit(bs1,5) == true);
    assert(bitset_get_bit(bs1,6) == false);
    assert(bitset_get_bit(bs1,33) == false);
    assert(bitset_get_bit(bs1,36) == true);
    assert(bitset_get_bit(bs1,65) == false);
    printf("PASSED\n");

    printf("\t\t\tsparse bitsets with regular bitset. \t");
    bs1 = bitset_create();
    bitset_set_bit(bs1, 36, true);
    bitset_set_bit(bs1, 40, true);

    bs2 = bitset_create();
    bitset_set_bit(bs2, 1, true);
    bitset_set_bit(bs2, 36, true);
    bitset_set_bit(bs2, 61, true);
    
    bitset_and_not(bs1, bs2);

    assert(bitset_get_bit(bs1,0) == false);
    assert(bitset_get_bit(bs1,31) == false);
    assert(bitset_get_bit(bs1,36) == false);
    assert(bitset_get_bit(bs1,40) == true);
    assert(bitset_get_bit(bs1,61) == false);
    printf("PASSED\n");
}

void test_bitset_equals(){
    printf("Test bitset_equals(): \t");
    printf("3 different bitsets circularly. \t");
    BitSet_struct* bs1 = bitset_create();
    bitset_set_bit(bs1, 1, true);
    bitset_set_bit(bs1, 3, true);
    bitset_set_bit(bs1, 20, true);
    bitset_set_bit(bs1, 67, true);

    BitSet_struct* bs2 = bitset_create();
    bitset_set_bit(bs2, 1, true);
    bitset_set_bit(bs2, 3, true);
    bitset_set_bit(bs2, 6, true);
    bitset_set_bit(bs2, 68, true);

    BitSet_struct* bs3 = bitset_create();
    bitset_set_bit(bs3, 1, true);
    bitset_set_bit(bs3, 3, true);
    bitset_set_bit(bs3, 20, true);
    bitset_set_bit(bs3, 68, true);

    assert(bitset_equals(bs1, bs2) == false);
    assert(bitset_equals(bs1, bs3) == true);
    assert(bitset_equals(bs2, bs3) == false);
    printf("PASSED\n");
}

void test_bitset_set_bit(){
    printf("Test bitset_set_bit(): \t");
    
    int bits_true[] = {1, 3, 5, 127, 128}; // bits_true[index+1] remains unset.
    int len_true = sizeof(bits_true)/sizeof(int);
    int bits_false[] = {3, 127};
    int len_false = sizeof(bits_false)/sizeof(int);

    BitSet_struct* bs = bitset_create();
    
    printf("setting %d bits to true. \t\t", len_true);
    for(int i = 0; i < len_true; ++i){
        bitset_set_bit(bs, bits_true[i], true);
        bitset_set_bit(bs, bits_true[i+1], false);
    }
    for(int i = 0; i < len_true; ++i){
        assert(bitset_get_bit(bs, bits_true[i]) == true);
    }  
    printf("PASSED\n");


    printf("\t\t\tsetting %d bits to false. \t\t", len_false);
    for(int i = 0; i < len_true; ++i){
        bitset_set_bit(bs, bits_true[i], false);
    }
    for(int i = 0; i < len_true; ++i){
        assert(bitset_get_bit(bs, bits_false[i]) == false);
    }
    printf("PASSED\n");

}

void test_bitset_get_bit(){
    printf("Test bitset_get_bit(): \t");
    printf("simple get test. \t\t\t");
    BitSet_struct* bs = bitset_create();
    bitset_set_bit(bs, 1, true);
    bitset_set_bit(bs, 3, true);

    assert(bitset_get_bit(bs, 1) == true);
    assert(bitset_get_bit(bs, 2) == false);
    assert(bitset_get_bit(bs, 3) == true);
    assert(bitset_get_bit(bs, 5) == false);
    printf("PASSED\n");



    printf("\t\t\tsparse get test. \t\t\t");
    bs = bitset_create();
    bitset_set_bit(bs, 1, true);
    bitset_set_bit(bs, 3, true);
    bitset_set_bit(bs, 31, true);
    bitset_set_bit(bs, 63, true);
    bitset_set_bit(bs, 128, true);

    assert(bitset_get_bit(bs, 1) == true);
    assert(bitset_get_bit(bs, 2) == false);
    assert(bitset_get_bit(bs, 3) == true);
    assert(bitset_get_bit(bs, 5) == false);
    assert(bitset_get_bit(bs, 30) == false);
    assert(bitset_get_bit(bs, 5) == false);
    assert(bitset_get_bit(bs, 32) == false);
    assert(bitset_get_bit(bs, 62) == false);
    assert(bitset_get_bit(bs, 63) == true);
    assert(bitset_get_bit(bs, 64) == false);
    assert(bitset_get_bit(bs, 127) == false);
    assert(bitset_get_bit(bs, 128) == true);
    assert(bitset_get_bit(bs, 129) == false);
    printf("PASSED\n");



}

void test_bitset_copy(){
    printf("Test bitset_copy(): \t");
    printf("simple copy test. \t\t\t");
    BitSet_struct* bs = bitset_create();
    bitset_set_bit(bs, 1, true);
    bitset_set_bit(bs, 3, true);

    BitSet_struct* bs_cp = bitset_copy(bs);

    assert(bitset_equals(bs, bs_cp) == true);
    assert(&bs != &bs_cp);
    printf("PASSED\n");

    
    printf("\t\t\tsparse copy test. \t\t\t");
    bs = bitset_create();
    bitset_set_bit(bs, 1, true);
    bitset_set_bit(bs, 3, true);
    bitset_set_bit(bs, 31, true);
    bitset_set_bit(bs, 32, true);
    bitset_set_bit(bs, 128, true);


    bs_cp = bitset_copy(bs);


    assert(bitset_equals(bs, bs_cp) == true);
    assert(&bs != &bs_cp);
    printf("PASSED\n");
}

int main(){
    test_bitset_get_bit();
    test_bitset_set_bit();
    test_bitset_equals();
    test_bitset_copy();
    test_bitset_or();
    test_bitset_and_not();
}




