h#include <assert.h>
#include <limits.h>
#include <pthread.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <sys/times.h>
#include <sys/time.h>
#include <unistd.h>
#include <math.h>

#define NTHREADS 5


inline
unsigned int find_recursion_depth(unsigned int x)
{
	return (unsigned int) (floor (log( (double) x ) / log( 2.0 ))  );
}

inline
void merge(double *left, int l_len, double *right, int r_len, double *out, int (*cmp)(const void *, const void *))
{
	int i, j, k;
	for (i = j = k = 0; i < l_len && j < r_len; ){
		out[k++] = cmp((void*) &left[i], (void*) &right[j]) ? right[j++] : left[i++];
	}
 
	while (i < l_len) out[k++] = left[i++];
	while (j < r_len) out[k++] = right[j++];
}

struct recur_struct{
	double *buf;
	double *tmp; 
	int len;
	unsigned int recursion_depth;
	unsigned int max_thread_split_depth;
	int (*cmp)(const void *, const void *);
};
typedef struct recur_struct recur_struct;

/* inner recursion of merge sort */
void recur(void* rs_in)
{
	recur_struct rs = *((recur_struct*) rs_in);
	
	int l = rs.len / 2;
	if (rs.len <= 1) return;
 	
 	pthread_t thread;
	int status = -111;
		
	//recur(tmp + l, buf + l, len - l, ++recursion_depth, max_thread_split_depth, cmp);
	recur_struct* rs_new = (recur_struct*) malloc(sizeof(recur_struct));
	rs_new->buf 						= rs.tmp + l;
	rs_new->tmp 						= rs.buf + l;
	rs_new->len 						= rs.len - l;
	rs_new->recursion_depth 			= rs.recursion_depth + 1;
	rs_new->max_thread_split_depth 	= rs.max_thread_split_depth;
	rs_new->cmp						= rs.cmp;
	
	if (rs.recursion_depth <= rs.max_thread_split_depth){
		status = pthread_create( &thread, NULL, recur, rs_new);
		if (status != 0){
			printf("Horrible error occured, thread couldn't be created!\nAborting..\n");
			exit(1);
		}
	} else {
		recur((void*) rs_new);	
	}

	//recur(tmp, buf, l, ++recursion_depth, max_thread_split_depth);
	recur_struct* rs_new2 = (recur_struct*) malloc(sizeof(recur_struct));
	rs_new2->tmp = rs.buf;
	rs_new2->buf = rs.tmp;
	rs_new2->len = l;
	rs_new2->recursion_depth = rs.recursion_depth + 1;
	rs_new2->max_thread_split_depth = rs.max_thread_split_depth;
	rs_new2->cmp = rs.cmp;
	recur((void*) rs_new2);
	
	
	if (status != -111){
		pthread_join(thread, NULL); // Wait here
	}
 
	merge(rs.tmp, l, rs.tmp + l, rs.len - l, rs.buf, rs.cmp);
	free(rs_new);
	free(rs_new2);
}
 
/* preparation work before recursion */
void merge_sort(double *a, size_t len, size_t elem_size, int (*cmp)(const void *, const void *))
{
	/* call alloc, copy and free only once */
	double *tmp = malloc(elem_size * len);
	memcpy(tmp, a, elem_size * len);
 
 	recur_struct rs;
 	rs.buf = a;
 	rs.tmp = tmp;
 	rs.len = len;
 	rs.recursion_depth = 1;
 	rs.max_thread_split_depth = find_recursion_depth( NTHREADS);
 	rs.cmp = cmp;
 	
	recur( (void*) &rs );
 
	free(tmp);
}

static double sec(void)
{
	return (double) time(NULL);
}

void par_sort(
	void*		base,	// Array to sort.
	size_t		n,	// Number of elements in base.
	size_t		s,	// Size of each element.
	int		(*cmp)(const void*, const void*)) // Behaves like strcmp
{
}

static int cmp(const void* ap, const void* bp)
{	
	/* you need to modify this function to compare doubles. */
	return ((*(double*) ap) < (*(double*) bp)); 
}

int main(int ac, char** av)
{
	int			n = 20000000;
	int			i;
	double*		a;
	double*		b;
	double		start, end;
	double 		start2,end2;

	if (ac > 1)
		sscanf(av[1], "%d", &n);

	srand(getpid());

	a = malloc(n * sizeof a[0]);
	b = malloc(n * sizeof b[0]);
	for (i = 0; i < n; i++){
		a[i] = rand();
		b[i] = a[i];
	}

	/*puts("before:");
	for (i = 0; i < n; i++) printf("%1.0f\t%1.0f\n", a[i], b[i]);
	putchar('\n');*/

	printf("qsort:ing...");
	fflush(stdout);
	start = sec();
	qsort(a, n, sizeof a[0], cmp);
	end = sec();
	printf(" done!\n");
	/*puts("after qsort:");
	for (i = 0; i < n; i++) printf("%1.0f ", a[i]);
	putchar('\n');*/

	srand(getpid());
	
	/*for (i = 0; i < n; i++)
		a[i] = rand();*/
 
	/*puts("before sort:");
	for (i = 0; i < n; i++) printf("%1.0f ", a[i]);
	putchar('\n');*/
	
	printf("Parallel mergesorting...");
	fflush(stdout);
	start2 = sec();
	merge_sort(b, n, sizeof(b[0]), cmp);
 	end2 = sec();
 	printf(" done!\n");

	/*puts("after parsort:");
	for (i = 0; i < n; i++) printf("%1.0f ", b[i]);
	putchar('\n');*/

	for (i = 0; i < n; i++){
		//printf("i = %d\n", i);
		assert(a[i] == b[i]);		
	}
 	
	/*puts("after sort:");
	for (i = 0; i < n; i++) printf("%1.0f ", a[i]);
	putchar('\n');*/
	
	printf("\nqsort: \t\t\tTook %1.2f seconds..\n", (double) end-start);
 	printf("parallel mergsort: \tTook %1.2f seconds..\n", (double) end2-start2);

	return 0;
}

