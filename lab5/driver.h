#ifndef driver_h
#define driver_h

#include "list.h"

#ifndef A16
#define A16 		__attribute__ ((aligned (16)))
#endif

#define ALIGN_EXP 7
#define ALIGN_CONSTANT 128
#define MAX_SPUS (8)

typedef struct{
	int		        proc;
	int		        nvertex;
	unsigned int*	bs_in_addr;
	unsigned int*	bs_out_addr;
	unsigned int*	bs_use_addr;
	unsigned int*	bs_def_addr;
    unsigned int    bitset_size;
    unsigned int    bitset_subsets;
} param_t A16;

typedef struct ppu_send_mail_t {
    unsigned int vertex_index;
} ppu_send_mail_t;

typedef struct spu_send_mail_t {
    unsigned int op_completed; //vertex index
} spu_send_mail_t;

unsigned int pad_length(unsigned int input);
void spu_quit();

#endif
