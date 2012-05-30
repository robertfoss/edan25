#ifndef vertex_c
#define vertex_c

#include <pthread.h>
#include "rand.h"
#include <math.h>
#include <sys/times.h>
#include <sys/time.h>
#include <ctype.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "list.h"
#include "vertex.h"
#include "driver.h"

int	nsym;
unsigned int bitset_size;


unsigned int* bitset_copy(unsigned int* bs){
    unsigned int* new_bs;
    void* tmp;
    posix_memalign(&tmp, (size_t) ALIGN_CONSTANT, (size_t) bitset_size);
    new_bs = tmp;
    for(unsigned int i = 0; i < (sizeof(unsigned int) * (nsym / (sizeof(unsigned int) * 8) + 1)); ++i){
        new_bs[i] = bs[i];
    }
    return new_bs;
}

bool bitset_equals(unsigned int* bs1, unsigned int* bs2){
    for(unsigned int i = 0; i < (sizeof(unsigned int) * (nsym / (sizeof(unsigned int) * 8) + 1)); ++i){
        if(bs1[i] != bs2[i]){
            return false;
        }
    }
    return true;
}

void bitset_or(unsigned int* bs1, unsigned int* bs2){
    for(unsigned int i = 0; i < (sizeof(unsigned int) * (nsym / (sizeof(unsigned int) * 8) + 1)); ++i){
        bs1[i] |= bs2[i];
    }
}

void bitset_and_not(unsigned int* bs1, unsigned int* bs2){
    for(unsigned int i = 0; i < (sizeof(unsigned int) * (nsym / (sizeof(unsigned int) * 8) + 1)); ++i){
        unsigned int tmp = bs1[i] & bs2[i];
        tmp = ~tmp;
        bs1[i] = tmp & bs1[i];
    }
}


void print_vertex(vertex_t* v){
	int i;

	printf("use[%d] = { ", v->index);
	for (i = 0; i < nsym; ++i){
        unsigned int bit_offset = (i / (sizeof(unsigned int) * 8));
        unsigned int bit_local_index = (unsigned int) (i % (sizeof(unsigned int) * 8));
		if ((v->use[bit_offset] & (1 << bit_local_index))){//bitset_get_bit(v->use, i)){
			printf("%d ", i);
		}
	}
	printf("}\n");
	printf("def[%d] = { ", v->index);

	for (i = 0; i < nsym; ++i){
        unsigned int bit_offset = (i / (sizeof(unsigned int) * 8));
        unsigned int bit_local_index = (unsigned int) (i % (sizeof(unsigned int) * 8));
		if ((v->def[bit_offset] & (1 << bit_local_index))){
//		if (bitset_get_bit(v->def, i)){
			printf("%d ", i);
		}
	}
	printf("}\n\n");
	printf("in[%d] = { ", v->index);

	for (i = 0; i < nsym; ++i){
        unsigned int bit_offset = (i / (sizeof(unsigned int) * 8));
        unsigned int bit_local_index = (unsigned int) (i % (sizeof(unsigned int) * 8));
		if ((v->in[bit_offset] & (1 << bit_local_index))){
//		if (bitset_get_bit(v->in, i)){
			printf("%d ", i);
		}
	}
	printf("}\n");
	printf("out[%d] = { ", v->index);

	for (i = 0; i < nsym; ++i){
        unsigned int bit_offset = (i / (sizeof(unsigned int) * 8));
        unsigned int bit_local_index = (unsigned int) (i % (sizeof(unsigned int) * 8));
		if ((v->out[bit_offset] & (1 << bit_local_index))){
//		if (bitset_get_bit(v->out, i)){
			printf("%d ", i);
		}
	}
	printf("}\n\n");
}

void connect(vertex_t* pred, vertex_t* succ){
    unsigned int* tmp;

    pred->succ_count++;
    posix_memalign( (void *)&tmp, (size_t) ALIGN_CONSTANT, (size_t) pad_length(sizeof(unsigned int)*(pred->succ_count)));
    if(pred->succ_list != NULL){
        memcpy(tmp, pred->succ_list, (pred->succ_count-1)*sizeof(unsigned int));
        free(pred->succ_list);
    }
    pred->succ_list = tmp;
    pred->succ_list[pred->succ_count-1] = succ->index;

    succ->pred_count++;
    posix_memalign( (void *)&tmp, (size_t) ALIGN_CONSTANT, (size_t) pad_length(sizeof(unsigned int)*(succ->pred_count)));
    if(succ->pred_list != NULL){
        memcpy(tmp, succ->pred_list, (succ->pred_count-1)*sizeof(unsigned int));
        free(succ->pred_list);
    }
    succ->pred_list = tmp;
    succ->pred_list[succ->pred_count-1] = pred->index;

}

void generateCFG(vertex_t* vertex_list, int nvertex, int maxsucc, Random* r, bool print_input){
    //printf("in generateCFG\n");
	int j;
	int k;
	int s;

	connect(&vertex_list[0], &vertex_list[1]);
	connect(&vertex_list[0], &vertex_list[2]);


    for(int i = 2; i < nvertex; ++i){
		if(print_input){
			printf("[%d] succ = {", i);
		}
        s = nextRand(r) % maxsucc +1;
		for (j = 0; j < s; ++j) {
			k = abs(nextRand(r)) % nvertex;
			if(print_input){
				printf(" %d", k);
			}
			connect(&vertex_list[i], &vertex_list[k]);
		}
		if(print_input){
			printf("}\n");
		}
	}
}

void bitset_set_bit(unsigned int* arr, unsigned int bit){
    unsigned int bit_offset = (bit / (sizeof(unsigned int) * 8));
    unsigned int bit_local_index = (unsigned int) (bit % (sizeof(unsigned int) * 8));
    //printf("\nSET\tbit_offset = %d\tbit_local_index = %d\n", bit_offset, bit_local_index);
    arr[bit_offset] |= (1 << bit_local_index);
}

bool bitset_get_bit(unsigned int* arr, unsigned int bit){
    unsigned int bit_offset = (bit / (sizeof(unsigned int) * 8));
    unsigned int bit_local_index = (unsigned int) (bit % (sizeof(unsigned int) * 8));
    //printf("\nbit_offset = %d\tbit_local_index = %d\n", bit_offset, bit_local_index);
    return (arr[bit_offset]) & (1 << bit_local_index);
}

void generateUseDef(vertex_t* vertex_list, int nvertex, int nsym, int nactive, Random* r, bool print_input){
    //printf("in generateUseDef\n");
	int j;
	int sym;

    for(int i = 0; i < nvertex; ++i){

		if(print_input){
			printf("[%d] usedef = {", vertex_list[i].index);
		}
		for (j = 0; j < nactive; ++j) {
			sym = abs(nextRand(r)) % nsym;

			if (j % 4 != 0) {
				if(!bitset_get_bit(vertex_list[i].def, sym)){//!bitset_get_bit(v->def, sym)){

					if(print_input){
						printf(" u %d", sym);
					}
					bitset_set_bit(vertex_list[i].use, sym);//bitset_set_bit(v->use, sym, true);
				}
			}else{
				if(!bitset_get_bit(vertex_list[i].use, sym)){//!bitset_get_bit(v->use, sym)){
					if(print_input){
						printf(" d %d", sym);
					}
					bitset_set_bit(vertex_list[i].def, sym);//bitset_set_bit(v->def, sym, true);
				}
			}
		}
		if(print_input){
			printf("}\n");
		}
	}
}


vertex_t* create_vertices(int nsym2, int nvertex, int maxsucc, int nactive, bool print_input){
	vertex_t* vertices;
    nsym = nsym2;
    bitset_size = 50*(nsym / (sizeof(unsigned int) * 8)) + 1;

	Random* r = new_random();
	setSeed(r, 1);


    void* tmp;
    posix_memalign(&tmp, (size_t) ALIGN_CONSTANT, (size_t) sizeof(vertex_t) * nvertex);
    vertices = tmp;

    for(int i = 0; i < nvertex; ++i){
	    vertices[i].index = i;
	    vertices[i].listed = false;
        //posix_memalign(&tmp, (size_t) ALIGN_CONSTANT, (size_t) maxsucc);
        vertices[i].pred_list = NULL;
        vertices[i].pred_count = 0;
        //posix_memalign(&tmp, (size_t) ALIGN_CONSTANT, (size_t) maxsucc);
        vertices[i].succ_list = NULL;
        vertices[i].succ_count = 0;
	    //vertices[i].pred_list = create_node(NULL); //First element = NULL
	    //vertices[i].succ_list = create_node(NULL); //First element = NULL
        posix_memalign(&tmp, (size_t) ALIGN_CONSTANT, (size_t) bitset_size);
        vertices[i].in = tmp;
        posix_memalign(&tmp, (size_t) ALIGN_CONSTANT, (size_t) bitset_size);
        vertices[i].out = tmp;
        posix_memalign(&tmp, (size_t) ALIGN_CONSTANT, (size_t) bitset_size);
        vertices[i].use = tmp;
        posix_memalign(&tmp, (size_t) ALIGN_CONSTANT, (size_t) bitset_size);
        vertices[i].def = tmp;

        // Instead of using a homegrown calloc_align..
        for(int j = 0; j < (int)(nsym / (sizeof(unsigned int) * 8)+1 ); ++j){
            vertices[i].in[j] = 0;
            vertices[i].out[j] = 0;
            vertices[i].use[j] = 0;
            vertices[i].def[j] = 0;
        }
    }

	generateCFG(vertices, nvertex, maxsucc, r, print_input);
	generateUseDef(vertices, nvertex, nsym, nactive, r, print_input);

	return vertices;
}

#endif

