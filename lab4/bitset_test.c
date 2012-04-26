#include <assert.h>
#include "bitset.h"


void test_bitset_or(){
    printf("\ntest_bitset_or:\n");

    BitSet_struct* bss1 = bitset_create();
    bitset_set_bit(bss1, 1, true);
    bitset_set_bit(bss1, 5, true);
    bitset_set_bit(bss1, 20, true);

    BitSet_struct* bss2 = bitset_create();
    bitset_set_bit(bss2, 1, true);
    bitset_set_bit(bss2, 3, true);
    bitset_set_bit(bss2, 61, true);

    bitset_print(bss1);
    printf("or\n");
    bitset_print(bss2);
    printf("=\n");
    
    bitset_or(bss1, bss2);
    bitset_print(bss1);

/////////////////

    bss1 = bitset_create();
    bitset_set_bit(bss1, 1, true);
    bitset_set_bit(bss1, 3, true);
    bitset_set_bit(bss1, 20, true);

    bss2 = bitset_create();

    printf("*********************************\n");
    bitset_print(bss1);
    printf("or\n");
    bitset_print(bss2);
    printf("=\n");
    
    bitset_or(bss2, bss1);
    bitset_print(bss1);

/////////////////

    bss1 = bitset_create();
    bitset_set_bit(bss1, 55, true);

    bss2 = bitset_create();
    bitset_set_bit(bss2, 1, true);
    bitset_set_bit(bss2, 3, true);
    bitset_set_bit(bss2, 61, true);

    printf("*********************************\n");
    bitset_print(bss1);
    printf("or\n");
    bitset_print(bss2);
    printf("=\n");
    
    bitset_or(bss1, bss2);
    bitset_print(bss1);
}

void test_bitset_and_not(){
    printf("\ntest_bitset_and_not:\t\tresult\n");

    BitSet_struct* bss1 = bitset_create();
    bitset_set_bit(bss1, 33, true);
    bitset_set_bit(bss1, 36, true);

    BitSet_struct* bss2 = bitset_create();
    bitset_set_bit(bss2, 1, true);
    bitset_set_bit(bss2, 3, true);
    bitset_set_bit(bss2, 6, true);
    bitset_set_bit(bss2, 33, true);
    bitset_set_bit(bss2, 65, true);
    bitset_print(bss1);
    printf("and_not\t\t\t\targ\n");
    bitset_print(bss2);
    printf("=\n");
    
    bitset_and_not(bss1, bss2);
    bitset_print(bss1);
}

void test_bitset_equals(){
    printf("\ntest_bitset_equals:\n");

    BitSet_struct* bss1 = bitset_create();
    bitset_set_bit(bss1, 1, true);
    bitset_set_bit(bss1, 3, true);
    bitset_set_bit(bss1, 20, true);

    BitSet_struct* bss2 = bitset_create();
    bitset_set_bit(bss2, 1, true);
    bitset_set_bit(bss2, 3, true);
    bitset_set_bit(bss2, 6, true);

    BitSet_struct* bss3 = bitset_create();
    bitset_set_bit(bss3, 1, true);
    bitset_set_bit(bss3, 3, true);
    bitset_set_bit(bss3, 20, true);

    assert(bitset_equals(bss1, bss2) == false);
    assert(bitset_equals(bss1, bss3) == true);
}

void test_bitset_set_bit(){
    printf("\ntest_bitset_set_bit:\n");

    int bits[] = {1, 3, 63};
    int len = sizeof(bits)/sizeof(int);

    BitSet_struct* bss = bitset_create();

    int i;
    printf("Setting bits to true: ");
    for(i = 0; i < len; ++i){
        bitset_set_bit(bss, bits[i], true);
        printf("%d ", bits[i]);
    }
    printf("\n");

    bitset_print(bss);

    printf("Setting bit %d to false\n", bits[0]);
    bitset_set_bit(bss, bits[0], false);
    bitset_print(bss);
}

void test_bitset_get_bit(){
    printf("\ntest_bitset_get_bit:\n");

    BitSet_struct* bss = bitset_create();
    bitset_set_bit(bss, 1, true);
    bitset_set_bit(bss, 3, true);

    assert(bitset_get_bit(bss, 1) == true);
    assert(bitset_get_bit(bss, 5) == false);
}

void test_bitset_copy(){
    printf("\ntest_bitset_copy:\n");

    BitSet_struct* bss = bitset_create();
    bitset_set_bit(bss, 1, true);
    bitset_set_bit(bss, 3, true);

    BitSet_struct* bss_cp = bitset_copy(bss);

    assert(bitset_equals(bss, bss_cp) == true);
    assert(&bss != &bss_cp);
}

int main(){
    test_bitset_or();
    //test_bitset_and_not();
    //test_bitset_equals();
    //test_bitset_set_bit();
    //test_bitset_get_bit();
    //test_bitset_copy();
}




