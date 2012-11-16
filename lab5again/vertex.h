#ifndef vertex_h
#define vertex_h

#ifndef A16
#define A16 		__attribute__ ((aligned (16)))
#endif

#include "list.h"

typedef struct{
	int index;
	bool listed;
    unsigned int pred_count;
    unsigned int succ_count;
	unsigned int* pred_list;
	unsigned int* succ_list;
	unsigned int* in;
	unsigned int* out;
	unsigned int* use;
	unsigned int* def;
    unsigned int* in_backup;
    unsigned int* out_backup;
} vertex_t A16;

vertex_t* create_vertices(int nsym, int nvertex, int maxsucc, int nactive, bool print_input);

void print_vertex(vertex_t* v);

#endif

