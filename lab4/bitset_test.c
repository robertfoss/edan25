
#include "bitset.h"

int main(){
    BitSet_struct* bss = bitset_create();
    bitset_set_bit(bss, 10, true);
    bitset_print(bss);
}
