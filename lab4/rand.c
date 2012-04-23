#include "rand.h"

void setSeed(Random r, int seed){
    r.w = seed + 1;
    r.z = seed * seed + seed + 2;
}

int nextRand(Random r){
    r.z = 36969 * (r.z & 65535) + (r.z >> 16);
    r.w = 18000 * (r.w & 65535) + (r.w >> 16);
    return (r.z << 16) + r.w;
}
