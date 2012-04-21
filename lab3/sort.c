#include <assert.h>
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

//#define THREAD_STATS

static unsigned int nbr_threads;
#ifdef THREAD_STATS
static unsigned int tc = 0;
static int* t;
#endif

inline
unsigned int find_recursion_depth(unsigned int x)
{
	return (unsigned int) (ceil(log((double) x) / log(2.0)));
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
#ifdef THREAD_STATS
	int c;
	unsigned int tn;
#endif
};
typedef struct recur_struct recur_struct;


/* inner recursion of merge sort */
void recur(void* rs_in)
{
	recur_struct rs = *((recur_struct*) rs_in);
	
	int l = rs.len >> 1; // div by 2
	if (rs.len <= 1){
#ifdef THREAD_STATS
		t[rs.tn] = rs.c;
#endif
		return;
	}
 	
 	pthread_t thread;
	int status = -111;
		
	//recur(tmp + l, buf + l, len - l, ++recursion_depth, max_thread_split_depth, cmp);
	recur_struct rs_new;
	rs_new.buf 						= rs.tmp + l;
	rs_new.tmp 						= rs.buf + l;
	rs_new.len 						= rs.len - l;
	rs_new.recursion_depth 		    = rs.recursion_depth + 1;
	rs_new.max_thread_split_depth 	= rs.max_thread_split_depth;
	rs_new.cmp						= rs.cmp;
	
	if (rs.recursion_depth <= rs.max_thread_split_depth){
#ifdef THREAD_STATS
		rs_new.c = 0;
		rs_new.tn = tc;
		tc++;
#endif
		status = pthread_create(&thread, NULL, recur, &rs_new);
		if (status != 0){
			printf("Horrible error occured, thread couldn't be created!\nAborting..\n");
			exit(1);
		}
	} else {
#ifdef THREAD_STATS
		rs_new.tn = rs.tn;
		rs_new.c = rs.c + 1;
#endif
		recur((void*) &rs_new);	
	}

	//recur(tmp, buf, l, ++recursion_depth, max_thread_split_depth);
	recur_struct rs_new2;
	rs_new2.tmp = rs.buf;
	rs_new2.buf = rs.tmp;
	rs_new2.len = l;
	rs_new2.recursion_depth = rs.recursion_depth + 1;
	rs_new2.max_thread_split_depth = rs.max_thread_split_depth;
	rs_new2.cmp = rs.cmp;
#ifdef THREAD_STATS
	rs_new2.c = rs.c + 1;
	rs_new2.tn = rs.tn;
#endif
	recur((void*) &rs_new2);
	
	if (status != -111){
		pthread_join(thread, NULL); // Wait here
	}

	merge(rs.tmp, l, rs.tmp + l, rs.len - l, rs.buf, rs.cmp);
	//printf("c = %d\n", rs.c);
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

 	rs.max_thread_split_depth = find_recursion_depth( nbr_threads);
 	rs.cmp = cmp;
#ifdef THREAD_STATS
	rs.c = 0;
	rs.tn = tc;
	tc++;
#endif 	
	recur((void*) &rs);

	free(tmp);
}

static double sec(void)
{
	struct timeval tv;
	gettimeofday(&tv, NULL);
	return (double) tv.tv_sec +  (double)tv.tv_usec / 1000000;
}


static int cmp(const void* ap, const void* bp)
{	
	/* you need to modify this function to compare doubles. */
	return ((*(double*) ap) < (*(double*) bp)); 
}

int main(int ac, char** av)
{
	int			n = 1000000;
	int			i;
	double*		a;
	double*		b;
	double*		c;
	double		start, end;
	double 		start2,end2;
	double 		start3,end3;
	nbr_threads = 4;

	if (ac > 1)
		sscanf(av[1], "%d", &n);
	if (ac > 2)
		sscanf(av[2], "%d", &nbr_threads);

	unsigned int rec_depth = find_recursion_depth(nbr_threads);
	unsigned int actual_threads = (unsigned int) pow(2.0, rec_depth);

	printf("\nnbr_threads: %d\tactual_threads: %d\trec_depth: %d\n", nbr_threads, actual_threads, rec_depth);

	srand(getpid());

	a = malloc(n * sizeof a[0]);
	b = malloc(n * sizeof b[0]);
	c = malloc(n * sizeof b[0]);
	for (i = 0; i < n; i++){
		c[i] = b[i] = a[i] = rand();
	}
#ifdef THREAD_STATS
	t = malloc(actual_threads * sizeof t[0]);
#endif

	/*puts("before:");
	for (i = 0; i < n; i++) printf("%1.0f\t%1.0f\n", a[i], b[i]);
	putchar('\n');*/

	printf("qsort...");
	fflush(stdout);
	start = sec();
	qsort(a, n, sizeof a[0], cmp);
	end = sec();
	printf(" done!\n");
	/*puts("after qsort:");
	for (i = 0; i < n; i++) printf("%1.0f ", a[i]);
	putchar('\n');*/


	
	printf("mergesort...");
	unsigned int tmp_nbr_threads = nbr_threads;
	nbr_threads = 1;
	fflush(stdout);
	start2 = sec();
	merge_sort(b, n, sizeof(b[0]), cmp);
 	end2 = sec();
	nbr_threads = tmp_nbr_threads;
 	printf(" done!\n");


	printf("parallel mergesort...");
	fflush(stdout);
	start3 = sec();
	merge_sort(c, n, sizeof(c[0]), cmp);
 	end3 = sec();
 	printf(" done!\n");

	/*puts("after p. mergesort:");
	for (i = 0; i < n; i++) printf("%1.0f ", c[i]);
	putchar('\n');*/
	
	
	printf("\nSorting %d elements using %d threads.\n", n, actual_threads);

	printf("qsort: \t\tTook %1.5f seconds..\n", (double) end-start);
 	printf("mergesort: \tTook %1.5f seconds..\n", (double) end2-start2);
 	printf("p. mergesort: \tTook %1.5f seconds..\n", (double) end3-start3);
 	double speedup_q_vs_m = (double)(end-start)/(end2-start2);
 	double speedup_q_vs_pm = (double)(end-start)/(end3-start3);
 	double speedup_m_vs_pm = (double)(end2-start2)/(end3-start3);
 	printf("\nqsort vs. mergesort speedup: \t\t%1.2fx\n", speedup_q_vs_m);
 	printf("qsort vs. p. mergesort speedup: \t%1.2fx\n", speedup_q_vs_pm);
 	printf("mergesort vs. p. mergesort speedup: \t%1.2fx\n", speedup_m_vs_pm);

 	unsigned int thread_eff = (unsigned int)(  (100*speedup_m_vs_pm) / (double) (actual_threads));
 	printf("mergesort vs. p. mergesort threading efficiency: %d%%\n", thread_eff);



	for (i = 0; i < n; i++){
		assert(a[i] == c[i]);		
	}
	free(a);
	free(b);
	free(c);

#ifdef THREAD_STATS
	for (i = 0; i < actual_threads; i++){
		printf("t[%d] = %d\n", i, t[i]);
	}
	free(t);
#endif
	return 0;
}

