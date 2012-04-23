typedef struct{
    int w;
    int z;
} Random;

void setSeed(Random r, int seed);
int nextRand(Random r);
