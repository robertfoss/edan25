#include <assert.h>
#include <string.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <inttypes.h>
#include <sys/time.h>
#include <pthread.h>
#include <libspe2.h>
#include "driver.h"
#include "error.h"

extern spe_program_handle_t dataflow;

typedef struct {
	spe_context_ptr_t	ctx;
	pthread_t		pthread;
	void*			arg;
	char			unused[2];
} arg_t A16;

char*		progname;

double sec(void)
{
	struct timeval	tv;

	gettimeofday(&tv, NULL);

	return tv.tv_sec + 1e-6 * tv.tv_usec;
}

void* work(void *arg)
{
	arg_t*  data = arg;
	unsigned int            entry = SPE_DEFAULT_ENTRY;

	if (spe_context_run(data->ctx, &entry, 0, data->arg, NULL, NULL) < 0) {
		perror("Failed running context");
		exit (1);    
	}

	printf("PPU pthread sees SPU has terminated.\n");

	pthread_exit(NULL);  
}

int main(int argc, char** argv) 
{
	double		begin;
	double		end;
	int		errnum;
	size_t		nthread = P;
	size_t		i;
	size_t		nvertex;
	unsigned int	x;		// sent to each SPU
	int		code;		// status;
	unsigned int	reply;		// from SPU
	arg_t		data[nthread];
	param_t		param[nthread] A16;

	argc 		= argc; 	// to silence gcc...
	progname	= argv[0];
	nvertex		= atoi(argv[2]);

	printf("nthread   = %zu\n", nthread);
	printf("nvertex   = %zu\n", nvertex);
	printf("ctx   = %zu\n", sizeof(param_t));
	printf("arg   = %zu\n", sizeof(arg_t));

	begin = sec();

	for (i = 0; i < nthread; ++i) {
		param[i].proc = i;
		param[i].nvertex = nvertex;

		if ((data[i].ctx = spe_context_create (0, NULL)) == NULL) {
			perror ("Failed creating context");
			exit(1);
		}

		if (spe_program_load (data[i].ctx, &dataflow))  {
			perror ("Failed loading program");
			exit(1);
		}

		data[i].arg = &param[i];
		printf("i=%d param=%p\n", i, data[i].arg);

		if (pthread_create (&data[i].pthread, NULL, work, &data[i])) {
			perror ("Failed creating thread");
			exit(1);
		}
	}

	// send some data to each SPU and wait for a reply.

	x = 42;

	for (i = 0; i < nthread; ++i) {
		code = spe_in_mbox_write(data[i].ctx, &x, 1, 1);
		code = spe_out_mbox_read(data[i].ctx, &reply, 1);
		assert(reply == x * 10);
	}

	end = sec();

	printf("%1.3lf s\n", end-begin);

	for (i = 0; i < nthread; ++i) {
		printf("joining with PPU pthread %zu...\n", i);
		errnum = pthread_join(data[i].pthread, NULL);
		if (errnum != 0)
			syserror(errnum, "pthread_join failed");

		if (spe_context_destroy (data[i].ctx) != 0) {
			perror("Failed destroying context");
			exit(1);
		}
	}

	return 0;
}
