#ifndef driver_h
#define driver_h

#include <stddef.h>

#include "vertex.h"

#define N	(2048)
#define S	(64)
#define P	(6)

#ifndef A16
#define A16 		__attribute__ ((aligned (16)))
#endif

#define ALIGN_CONSTANT 128
#define ALIGN_EXP 7

typedef int		type_t;

unsigned int pad_length(unsigned int input);

typedef struct {
	int		        proc;
	int		        nvertex;
    int             nsym;
    vertex_t*       vertices;
    unsigned int    bitset_size;
	char	        unused[12];
} param_t A16;

typedef union mail_t {
    struct {
        unsigned int vertex_done:1;
        unsigned int requeue_vertex:1;
        unsigned int get_next_vertex:1;
        unsigned int complete_execution:1;
        unsigned int start_execution:1;
        unsigned int vertex_number:27;
    } str;

    unsigned int uint;
} mail_t;



#endif

