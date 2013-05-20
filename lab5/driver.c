#include <pthread.h>
#include <math.h>
#include <sys/times.h>
#include <sys/time.h>
#include <ctype.h>
#include <stdbool.h>
#include <libspe2.h>
#include <string.h>
#include <unistd.h>

#include "driver.h"
#include "list.h"
#include "rand.h"

#define D(x) 
#define MSG(x)  x
#define UNUSED(x) (void)(x)

typedef struct{
	spe_context_ptr_t	ctx;
	pthread_t		pthread;
    unsigned short	spu_num;
	void*			arg;
} arg_t A16;

typedef struct{
	int index;
	bool listed;
	list_t* pred_list;
	list_t* succ_list;
	pthread_mutex_t mutex;
    pthread_cond_t cond;
} Vertex;

extern spe_program_handle_t dataflow;

bool print_input;
int nvertex;
int	nthread;
int	nsym;
unsigned int alloc_size;
unsigned int bitset_subsets;
arg_t   data[MAX_SPUS];
char*		progname;


unsigned int *in, *out, *use, *def;


void bitset_megaop(unsigned int vertex_index, unsigned int thread_index);
void spu_bitset_megaop(unsigned int vertex_index, unsigned int thread_index);
#ifdef USE_CELL
void (*bitset_megaop_ptr)(unsigned int, unsigned int) = &spu_bitset_megaop;
#else
void (*bitset_megaop_ptr)(unsigned int, unsigned int) = &bitset_megaop;
#endif

static double sec(void){
	struct timeval tv;
	gettimeofday(&tv, NULL);
	return (double) tv.tv_sec + (double)tv.tv_usec / 1000000;
}

inline
unsigned int pad_length(unsigned int input){
/*    if(input<=1)
        return 1;
    if(input<=2)
        return 2;
    if(input<=4)
        return 4;
    if(input<=8)
        return 8;
    if(input<=16)
        return 16;
*/
    unsigned int mod = input % 16;
    if(mod == 0){
        return input;
    } else {
        return input + 16 - mod;
    }
}

Vertex* new_vertex(int i){
	Vertex* v = malloc(sizeof(Vertex));
	v->index = i;
	v->listed = false;
	v->pred_list = create_node(NULL); //First element = NULL
	v->succ_list = create_node(NULL); //First element = NULL

    pthread_mutex_init(&v->mutex, NULL);
    pthread_cond_init(&v->cond, NULL);
	return v;
}

void acquire_locks(Vertex* u, list_t* succ_list, list_t* pred_list){
    unsigned int acquired_locks = 0;
    unsigned int needed_locks = 1; // We know of Vertex* u.
    unsigned int saved_locks = 0;
    list_t* tmp_list;
    Vertex* v;

    tmp_list = succ_list;
	while(tmp_list->next != tmp_list){
		tmp_list = tmp_list->next;
        ++needed_locks;
	}
    tmp_list = pred_list;
	while(tmp_list->next != tmp_list){
		tmp_list = tmp_list->next;
        ++needed_locks;
	}
    pthread_mutex_t mutexes[needed_locks];
    pthread_cond_t conds[needed_locks];
    mutexes[0] = u->mutex;
    conds[0] = u->cond;
    ++saved_locks;
    
    tmp_list = succ_list;
	while(tmp_list->next != tmp_list){
		tmp_list = tmp_list->next;
        v = tmp_list->data;
        if(v != NULL){
            mutexes[saved_locks] = v->mutex;
            conds[saved_locks] = v->cond;
            ++saved_locks;
        }
	}
    tmp_list = pred_list;
	while(tmp_list->next != tmp_list){
		tmp_list = tmp_list->next;
        v = tmp_list->data;
        if(v != NULL){
            conds[saved_locks] = v->cond;
            mutexes[saved_locks] = v->mutex;
            ++saved_locks;
        }
	}

    // Acquire all or no locks.
    while(1){
        //printf("while(1)\n");
        acquired_locks = 0;
        for(unsigned int i = 0; i < needed_locks; ++i){
            if(pthread_mutex_trylock(&mutexes[i])){
                ++acquired_locks;
                if(acquired_locks == needed_locks){
                    //printf("Acquired all locks.\n");
                    return;
                }
            } else {
                //printf("Failed to acquire lock.\n");
                for(unsigned int j = 0; j < acquired_locks; ++j){
                    //printf("Freeing a lock.\n");
                    pthread_mutex_unlock(&mutexes[j]);
                    pthread_cond_signal(&conds[j]);
                }
                acquired_locks = 0;
                //pthread_cond_wait(&conds[i], &mutexes[i]);
                break; // Break out of for-loop.
            }
        }
    }
}


void unlock_locks(Vertex* u, list_t* succ_list, list_t* pred_list){
    Vertex* v;
    list_t* tmp_list = succ_list;

	while(tmp_list->next != tmp_list){
		tmp_list = tmp_list->next;
        v = tmp_list->data;
        if(v != NULL){
            pthread_mutex_unlock(&v->mutex);
            pthread_cond_signal(&v->cond);
        }
	}
    //printf("Unlocked succ_list.\n");
    tmp_list = pred_list;
	while(tmp_list->next != tmp_list){
		tmp_list = tmp_list->next;
        v = tmp_list->data;
        if(v != NULL){
            pthread_mutex_unlock(&v->mutex);
            pthread_cond_signal(&v->cond);
        }
	}
    //printf("Unlocked pred_list.\n");
    pthread_mutex_unlock(&u->mutex);
    pthread_cond_signal(&u->cond);
    //printf("Unlocked vertex.\n");
}

unsigned int* bitset_copy(unsigned int* bs){
    unsigned int* new_bs = calloc(bitset_subsets, sizeof(unsigned int));
    //for(unsigned int i = 0; i < (sizeof(unsigned int) * (nsym / (sizeof(unsigned int) * 8) + 1)); ++i){
    for(unsigned int i = 0; i < bitset_subsets; ++i) {
        new_bs[i] = bs[i];
    }
    return new_bs;
}


bool bitset_equals(unsigned int* bs1, unsigned int* bs2){
    //for(unsigned int i = 0; i < (sizeof(unsigned int) * (nsym / (sizeof(unsigned int) * 8) + 1)); ++i){
    for(unsigned int i = 0; i < bitset_subsets; ++i) {
        if(bs1[i] != bs2[i]){
            return false;
        }
    }
	return true;
}

void bitset_or(unsigned int* bs1, unsigned int* bs2){
    //for(unsigned int i = 0; i < (sizeof(unsigned int) * (nsym / (sizeof(unsigned int) * 8) + 1)); ++i){
    for(unsigned int i = 0; i < bitset_subsets; ++i) {
        bs1[i] |= bs2[i];
    }
}


void bitset_and_not(unsigned int* bs1, unsigned int* bs2){
    //for(unsigned int i = 0; i < (sizeof(unsigned int) * (nsym / (sizeof(unsigned int) * 8) + 1)); ++i){
    for(unsigned int i = 0; i < bitset_subsets; ++i) {
        unsigned int tmp = bs1[i] & bs2[i];
        tmp = ~tmp;
        bs1[i] = tmp & bs1[i];
    }
}


void bitset_set_bit(unsigned int* arr, unsigned int bit){
    unsigned int bit_offset = (bit / (sizeof(unsigned int) * 8));
    unsigned int bit_local_index = (unsigned int) (bit % (sizeof(unsigned int) * 8));
    //printf("bit: %d\tbit_offset: %d \tbit_local_index: %d\n",bit,bit_offset,bit_local_index);
    arr[bit_offset] |= (1 << bit_local_index);
}


bool bitset_get_bit(unsigned int* arr, unsigned int bit){
    unsigned int bit_offset = (bit / (sizeof(unsigned int) * 8));
    unsigned int bit_local_index = (unsigned int) (bit % (sizeof(unsigned int) * 8));
    return (arr[bit_offset]) & (1 << bit_local_index);
}


void bitset_megaop(unsigned int vertex_index, unsigned int thread_index){
    unsigned int tmp = vertex_index*bitset_subsets;
    UNUSED(thread_index);
    for(unsigned int i = 0; i < bitset_subsets; ++i) {
        in[tmp + i]  = out[tmp + i];
		in[tmp + i]  = in[tmp + i] & (~(in[tmp + i] & def[tmp + i]));
		in[tmp + i] |= use[tmp + i];
    }
}

void computeIn(Vertex* u, list_t* worklist, unsigned int thread_index){
	Vertex* v;
    acquire_locks(u, u->succ_list, u->pred_list);

	list_t* tmp_list = u->succ_list;
	do {
        tmp_list = tmp_list->next;
		v = tmp_list->data;
        if(v != NULL){
		    bitset_or(&(out[u->index*bitset_subsets]), &(in[v->index*bitset_subsets]));
        }
	} while(tmp_list->next != tmp_list);

	unsigned int* old = bitset_copy(&(in[u->index*bitset_subsets]));
	bitset_megaop_ptr(u->index, thread_index);


	if(!bitset_equals( &(in[u->index*bitset_subsets]), old)){
		tmp_list = u->pred_list;
		do{
            tmp_list = tmp_list->next;
			v = tmp_list->data;
			if(v != NULL){
                if(!v->listed){
				    add_last(worklist, create_node(v));
				    v->listed = true;
                }
			}
		}while(tmp_list->next != tmp_list);
	}
    free(old);
    unlock_locks(u, u->succ_list, u->pred_list);
}

void print_vertex(Vertex* v){
	int i;

	printf("use[%d] = { ", v->index);
	for (i = 0; i < nsym; ++i){
	if (bitset_get_bit( &(use[v->index*bitset_subsets]), i) ) {//bitset_get_bit(v->use, i)){
			printf("%d ", i);
		}
	}
	printf("}\n");
	printf("def[%d] = { ", v->index);

	for (i = 0; i < nsym; ++i){
	if ( bitset_get_bit( &(def[v->index*bitset_subsets]), i) ) {
			printf("%d ", i);
		}
	}
	printf("}\n\n");
	printf("in[%d] = { ", v->index);

	for (i = 0; i < nsym; ++i){
	if ( bitset_get_bit( &(in[v->index*bitset_subsets]), i) ) {
			printf("%d ", i);
		}
	}
	printf("}\n");
	printf("out[%d] = { ", v->index);

	for (i = 0; i < nsym; ++i){
		if ( bitset_get_bit( &(out[v->index*bitset_subsets]), i) ) {
			printf("%d ", i);
		}
	}
	printf("}\n\n");
}

void connect(Vertex* pred, Vertex* succ){
	add_last(pred->succ_list, create_node(succ));
	add_last(succ->pred_list, create_node(pred));
}

void generateCFG(list_t* vertex_list, int maxsucc, Random* r){
    //printf("in generateCFG\n");
	int i = 2;
	int j;
	int k;
	int s;
    Vertex* tmp_v;
    Vertex* tmp_s;
	list_t* tmp_list = vertex_list->next;
    list_t* tmp_list_s;

    tmp_v = tmp_list->next->data;
	connect(tmp_list->data, tmp_v);

    tmp_v = tmp_list->next->next->data;
	connect(tmp_list->data, tmp_v);
	tmp_list = tmp_list->next;

	while(tmp_list->next != tmp_list){
		tmp_list = tmp_list->next;
        tmp_v = tmp_list->data; //vertex[i]
		if(print_input){
			printf("[%d] succ = {", i);
		}

        s = nextRand(r) % maxsucc +1;
		for (j = 0; j < s; ++j) {
			k = abs(nextRand(r)) % nvertex;
			if(print_input){
				printf(" %d", k);
			}
            tmp_list_s = vertex_list->next;
            tmp_s = tmp_list_s->data;
            while(tmp_s->index != k){
                tmp_list_s = tmp_list_s->next;
                tmp_s = tmp_list_s->data;
            }

			connect(tmp_v, tmp_s);
		}
		if(print_input){
			printf("}\n");
		}
		++i;
	}
}


void generateUseDef(list_t* vertex_list, int nsym, int nactive, Random* r){
	int j;
	int sym;
	list_t* tmp_list = vertex_list;
	Vertex* v;

	do{
        tmp_list = tmp_list->next;
		v = tmp_list->data;

		if(print_input){
			printf("[%d] usedef = {", v->index);
		}
		for (j = 0; j < nactive; ++j) {
			sym = abs(nextRand(r)) % nsym;
			if (j % 4 != 0) {
printf("bitset_get_bit(v->def, %d) = %d\n", sym, bitset_get_bit( &(def[v->index*bitset_subsets]), sym));
				if(!bitset_get_bit( &(def[v->index*bitset_subsets]), sym)){//!bitset_get_bit(v->def, sym)){
					if(print_input){
						printf(" u %d", sym);
					}
					bitset_set_bit( &(use[v->index*bitset_subsets]), sym);//bitset_set_bit(v->use, sym, true);
				}
			}else{
printf("bitset_get_bit(v->use, %d) = %d\n", sym, bitset_get_bit(&(use[v->index*bitset_subsets]), sym));
				if(!bitset_get_bit( &(use[v->index*bitset_subsets]), sym)){//!bitset_get_bit(v->use, sym)){
					if(print_input){
						printf(" d %d", sym);
					}
					bitset_set_bit( &(def[v->index*bitset_subsets]), sym);//bitset_set_bit(v->def, sym, true);
				}
			}
		}
		if(print_input){
			printf("}\n");
		}
	}while(tmp_list->next != tmp_list);
}

typedef struct thread_struct {
    unsigned int index;
    list_t* worklist;
} thread_struct;

void* thread_func(void* ts){
    list_t* worklist = ((thread_struct*) ts)->worklist;
    unsigned int index = ((thread_struct*) ts)->index;
	printf("Thread[%u] in thread_func()\n", index);
	Vertex* u;
    unsigned int work_counter = 0;
	while(worklist->next != worklist){ // while (!worklist.isEmpty())
        u = remove_node(worklist->next);

D(printf("PPU[%u] index: %u  bitset_subsets: %u  offset: %u\n", index, u->index, bitset_subsets, u->index*bitset_subsets);
printf("PPU[%u]\t&use: %p\n\t&def: %p\n\t&out: %p\n\t&in:  %p\n", index, (void*)&(use[u->index]), (void*)&(def[u->index]), (void*)&(out[u->index]), (void*)&(in[u->index]));
printf("Iteration #%d has\tuse(%p)={", work_counter, (void*)&(use[u->index*bitset_subsets]));
	for (int i = 0; i < nsym; ++i){
	if ( bitset_get_bit( &(use[u->index*bitset_subsets]), i) ) {
			printf("%d ", i);
		}
	}
printf("}\n");
printf("Iteration #%d has\tdef(%p)={", work_counter, (void*)&(def[u->index*bitset_subsets]));
	for (int i = 0; i < nsym; ++i){
	if ( bitset_get_bit( &(def[u->index*bitset_subsets]), i) ) {
			printf("%d ", i);
		}
	}
printf("}\n");
printf("Iteration #%d has\tout(%p)={", work_counter, (void*)&(out[u->index*bitset_subsets]));
	for (int i = 0; i < nsym; ++i){
	if ( bitset_get_bit( &(out[u->index*bitset_subsets]), i) ) {
			printf("%d ", i);
		}
	}
printf("}\n");
printf("Iteration #%d has\tin (%p)={", work_counter, (void*)&(in[u->index*bitset_subsets]));
	for (int i = 0; i < nsym; ++i){
	if ( bitset_get_bit( &(in[u->index*bitset_subsets]), i) ) {
			printf("%d ", i);
		}
	}
printf("}\n");)
		u->listed = false;
		computeIn(u, worklist, index);

D(printf("Iteration #%d returned\tin={", work_counter);
	for (int i = 0; i < nsym; ++i){
	if ( bitset_get_bit( &(in[u->index*bitset_subsets]), i) ) {
			printf("%d ", i);
		}
	}
printf("}\n");)

        work_counter++;
	}
    printf("Thread[%u] worked %u iterations.\n", index, work_counter);
    return NULL;
}

void* spu_init(void *arg)
{
	arg_t* data = arg;
	unsigned int entry = SPE_DEFAULT_ENTRY;
    printf("PPU thread #%d, Starting SPU[%d] context.\n", data->spu_num, data->spu_num);
	if (spe_context_run(data->ctx, &entry, 0, data->arg, NULL, NULL) < 0) {
		perror("Failed running context");
		exit (1);
	}
	printf("PPU thread #%d sees that SPU[%u] has terminated.\n", data->spu_num, data->spu_num);


	pthread_exit(NULL);  
}

void spu_quit(){
	printf("spu_quit()\n");
	ppu_send_mail_t send;
	send.vertex_index = UINT_MAX;
	for(int i = 0; i < nthread; ++i){
		spe_in_mbox_write(data[i].ctx, &send.vertex_index, 1, 1);
	}
}

void spu_bitset_megaop(unsigned int vertex_index, unsigned int thread_index){
	ppu_send_mail_t send;
	spu_send_mail_t recv;

	recv.op_completed = UINT_MAX;

	// Select which SPU to use.
	spe_context_ptr_t context = data[thread_index].ctx;
	
	MSG(printf("spu_bitset_megaop() PPU[%u]->SPU[%u] sending msg #%u\n", thread_index, thread_index, vertex_index);)
	send.vertex_index = vertex_index;
    spe_in_mbox_write(context, &send.vertex_index, 1, 1);

	// Block until bitset has been completed
    while (recv.op_completed != vertex_index) {
		MSG(printf("spu_bitset_megaop() PPU[%u]<-SPU[%u] waiting for reply #%u..\n", thread_index, thread_index, vertex_index);)
		spe_out_intr_mbox_read(context, &recv.op_completed, 1, SPE_MBOX_ALL_BLOCKING);
		MSG(printf("spu_bitset_megaop() PPU[%u]<-SPU[%u] received message #%u \n", thread_index, thread_index, recv.op_completed);)

	}
}

void liveness(list_t* vertex_list, int nthread){
    printf("liveness()\n");
	Vertex* v;

    int status[nthread];
 	pthread_t thread[nthread];
	list_t* worklist[nthread];
    int worksplit[nthread];

	double begin;
	double end;

	begin = sec();

    for(int i = 0; i < nthread; ++i){
        worksplit[i] = 0;
        worklist[i] = create_node(NULL);
    }


	printf("Splitting worklist..\n");
	list_t* tmp_list = vertex_list;//->next;
    int counter = 0;
	while(tmp_list->next != tmp_list){
        tmp_list = tmp_list->next;
		v = tmp_list->data;
		v->listed = true;
        int index = (counter++) % nthread;
        worksplit[index]++;
		add_last(worklist[index], create_node(v));
	}

	printf("Allocing thread_structs\n");
    for(int i = 0; i < nthread; ++i){
        thread_struct* ts = malloc(sizeof(thread_struct));
        ts->index = i;
        //printf("i = %d\n", i);
        ts->worklist = worklist[i];
        status[i] = pthread_create(&thread[i], NULL, thread_func, ts);
    }

    for(int i = 0; i < nthread; ++i){
        pthread_join(thread[i], NULL);
    }
	end = sec();
//	spu_quit();
    printf("c runtime = %f s\n", (end - begin));

}

int main(int ac, char** av){

	int	i;
	int	maxsucc;
	int	nactive;
	bool print_output; 
	list_t* vertex;
	Random* r = new_random();
    list_t* tmp_list;

	setSeed(r, 1);
	vertex = create_node(NULL); //First element = NULL

    if (ac == 8){
	    sscanf(av[1], "%d", &nsym);
	    sscanf(av[2], "%d", &nvertex);
	    sscanf(av[3], "%d", &maxsucc);
	    sscanf(av[4], "%d", &nactive);
	    sscanf(av[5], "%d", &nthread);
        if(nthread < 1 || nthread > 6){
            nthread = 6;
        }
	    char* tmp_string = "";
        tmp_string = av[6];
	    if(tolower(tmp_string[0]) == 't'){
		    print_output = true;
	    }else{
		    print_output = false;
	    }

        tmp_string = av[7];
	    if(tolower(tmp_string[0]) == 't'){
		    print_input = true;
	    }else{
		    print_input = false;
	    }
    } else {
	    printf("\nWrong # of args (nsym nvertex maxsucc nactive nthreads print_output print_input).\nAssuming sane defaults.\n");
        nsym = 100;
        nvertex = 8;
        maxsucc = 4;
        nactive = 10;
        nthread = 1;
        print_output = true;
	    print_input = false;
    }
	progname = av[0];
    int bits_per_uint = sizeof(unsigned int) * 8;
    int uints_per_bitset = (unsigned int) ( ( (float)(nsym) / (float)(bits_per_uint) ) + 1);
    alloc_size = pad_length(sizeof(unsigned int) * pad_length(uints_per_bitset));
	bitset_subsets = pad_length(uints_per_bitset);

	printf("nsym   = %zu\n", nsym);
	printf("nvertex   = %zu\n", nvertex);
	printf("maxsucc   = %zu\n", maxsucc);
	printf("nactive   = %zu\n", nactive);
	printf("bits_per_uint   = %zu\n", bits_per_uint);
	printf("uints_per_bitset   = %zu\n", uints_per_bitset);
	printf("alloc_size   = %zu\n", alloc_size);
	printf("bitset_subsets   = %zu\n", bitset_subsets);

	//
	// Generate CFG
    tmp_list = vertex;
	for (i = 0; i < nvertex; ++i){
        insert_after(tmp_list, create_node(new_vertex(i)));
		tmp_list = tmp_list->next;
	}

    posix_memalign((void*) &in, (size_t) ALIGN_CONSTANT, (size_t) alloc_size * nvertex);
    posix_memalign((void*) &out, (size_t) ALIGN_CONSTANT, (size_t) alloc_size * nvertex);
    posix_memalign((void*) &use, (size_t) ALIGN_CONSTANT, (size_t) alloc_size * nvertex);
    posix_memalign((void*) &def, (size_t) ALIGN_CONSTANT, (size_t) alloc_size * nvertex);
    if(in == NULL || out == NULL || use == NULL || def == NULL){
        printf("posix_memalign returned null\n");
		exit(1);
	}
    memset(in,  0, alloc_size * nvertex);
    memset(out, 0, alloc_size * nvertex);
    memset(use, 0, alloc_size * nvertex);
    memset(def, 0, alloc_size * nvertex);

	printf("Generating CFG..\n");
	generateCFG(vertex, maxsucc, r);
	printf("Generating UseDef\n");
	generateUseDef(vertex, nsym, nactive, r);
	printf("Creating SPU context\n");

	//
	// Create SPU context and SPU-managing threads
#ifdef USE_CELL
    param_t param[nthread] A16;

	for (int i = 0; i < nthread; ++i) {
		printf("Setting SPU params\n");
		data[i].spu_num = param[i].proc = i;
		param[i].nvertex = nvertex;
        param[i].bs_in_addr  = in;
        param[i].bs_out_addr = out;
        param[i].bs_use_addr = use;
        param[i].bs_def_addr = def;
        param[i].bitset_size = pad_length( alloc_size );
		param[i].bitset_subsets = bitset_subsets;

		if ((data[i].ctx = spe_context_create (0, NULL)) == NULL) {
			perror ("Failed creating context");
			exit(1);
		}

		if (spe_program_load (data[i].ctx, &dataflow))  {
			perror ("Failed loading program");
			exit(1);
		}

		data[i].arg = &param[i];

		if (pthread_create(&data[i].pthread, NULL, spu_init, &data[i]) ) {
			perror ("Failed creating thread");
			exit(1);
		}
	}
#endif
	//
	// Start liveness analysis
	liveness(vertex, nthread);

	//
	// Print results
	if(print_output){
        tmp_list = vertex->next;

		for (i = 0; i < nvertex; ++i){
			print_vertex(tmp_list->data);
			tmp_list = tmp_list->next;
		}
	}

	return 0;
}

