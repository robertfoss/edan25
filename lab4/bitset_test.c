#include <assert.h>
#include <stdbool.h>

#include "bitset.h"


void test_bitset_or(){
    printf("Test or: ");

    BitSet_struct* bs1 = bitset_create();
    bitset_set_bit(bs1, 1, true);
    bitset_set_bit(bs1, 5, true);
    bitset_set_bit(bs1, 20, true);

    BitSet_struct* bs2 = bitset_create();
    bitset_set_bit(bs2, 1, true);
    bitset_set_bit(bs2, 3, true);
    bitset_set_bit(bs2, 61, true);

    bitset_or(bs1, bs2);

    printf("Or 2 bitsets of differing length. \t");
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

    bs1 = bitset_create();
    bitset_set_bit(bs1, 55, true);

    bs2 = bitset_create();
    bitset_set_bit(bs2, 1, true);
    bitset_set_bit(bs2, 3, true);
    bitset_set_bit(bs2, 61, true);

    bitset_or(bs1, bs2);
    printf("\t\tOr sparse bitset with reg. bitset \t");
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
}

void test_bitset_and_not(){
    printf("\ntest_bitset_and_not:\t\tresult\n");

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

    bitset_print(bs1);
    printf("and_not\t\t\t\targ\n");
    bitset_print(bs2);
    printf("=\n");
    bitset_and_not(bs1, bs2);
    bitset_print(bs1);

    printf("*********************************\n");

    bs1 = bitset_create();
    bitset_set_bit(bs1, 40, true);
    bitset_set_bit(bs1, 36, true);

    bs2 = bitset_create();
    bitset_set_bit(bs2, 1, true);
    bitset_set_bit(bs2, 36, true);
    bitset_set_bit(bs2, 61, true);
    
    bitset_print(bs1);
    printf("and_not\t\t\t\targ\n");
    bitset_print(bs2);
    printf("=\n");
    bitset_and_not(bs1, bs2);
    bitset_print(bs1);
}

void test_bitset_equals(){
    printf("\ntest_bitset_equals:\n");

    BitSet_struct* bs1 = bitset_create();
    bitset_set_bit(bs1, 1, true);
    bitset_set_bit(bs1, 3, true);
    bitset_set_bit(bs1, 20, true);

    BitSet_struct* bs2 = bitset_create();
    bitset_set_bit(bs2, 1, true);
    bitset_set_bit(bs2, 3, true);
    bitset_set_bit(bs2, 6, true);

    BitSet_struct* bs3 = bitset_create();
    bitset_set_bit(bs3, 1, true);
    bitset_set_bit(bs3, 3, true);
    bitset_set_bit(bs3, 20, true);

    assert(bitset_equals(bs1, bs2) == false);
    assert(bitset_equals(bs1, bs3) == true);
}

void test_bitset_set_bit(){
    printf("\ntest_bitset_set_bit:\n");

    int bits[] = {1, 3, 63};
    int len = sizeof(bits)/sizeof(int);

    BitSet_struct* bs = bitset_create();

    int i;
    printf("Setting bits to true: ");
    for(i = 0; i < len; ++i){
        bitset_set_bit(bs, bits[i], true);
        printf("%d ", bits[i]);
    }
    printf("\n");

    bitset_print(bs);

    printf("Setting bit %d to false\n", bits[0]);
    bitset_set_bit(bs, bits[0], false);
    bitset_print(bs);
}

void test_bitset_get_bit(){
    printf("\ntest_bitset_get_bit:\n");

    BitSet_struct* bs = bitset_create();
    bitset_set_bit(bs, 1, true);
    bitset_set_bit(bs, 3, true);

    assert(bitset_get_bit(bs, 1) == true);
    assert(bitset_get_bit(bs, 5) == false);
}

void test_bitset_copy(){
    printf("\ntest_bitset_copy:\n");

    BitSet_struct* bs = bitset_create();
    bitset_set_bit(bs, 1, true);
    bitset_set_bit(bs, 3, true);

    BitSet_struct* bs_cp = bitset_copy(bs);

    assert(bitset_equals(bs, bs_cp) == true);
    assert(&bs != &bs_cp);
}

int main(){
    test_bitset_or();
    //test_bitset_and_not();
    //test_bitset_equals();
    //test_bitset_set_bit();
    //test_bitset_get_bit();
    //test_bitset_copy();
}




