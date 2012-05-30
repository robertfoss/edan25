#ifndef driver_h
#define driver_h

#include <stddef.h>

#define N	(2048)
#define S	(64)
#define P	(6)

#ifndef A16
#define A16 		__attribute__ ((aligned (16)))
#endif

typedef int		type_t;

typedef struct {
	int		proc;
	int		nvertex;
	char		unused[6];
} param_t A16;

#endif
