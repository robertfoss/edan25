#include "rand.h"
#include <stdio.h>

void test_w_z_seed(){
    Random* r = new_random();
    setSeed(r, 1);
    printf("r.w = %d\nr.z = %d\n", r->w, r->z);
}

void test_random_rand(){
    Random* r = new_random();
    setSeed(r, 1);
    int i;
    for(i = 0; i < 5; ++i){
        printf("nextRand = %d\n", nextRand(r));
    }
}

int main(){
    test_w_z_seed();
    test_random_rand();
}
