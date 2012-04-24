
#include "bitset.h"

int main(){
    BitSet_struct* bss = bitset_create();
    bitset_set_bit(bss, 1, true);
    bitset_set_bit(bss, 3, true);
    bitset_set_bit(bss, 63, true);
/*    bitset_set_bit(bss, 33, true);
    bitset_set_bit(bss, 64, true);
    bitset_set_bit(bss, 65, true);*/
    bitset_set_bit(bss, 128, true);
    bitset_print(bss);
}
