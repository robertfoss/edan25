typedef struct{
    int w;
    int z;
} Random;

Random* new_random();
void setSeed(Random* r, int seed);
int nextRand(Random* r);
