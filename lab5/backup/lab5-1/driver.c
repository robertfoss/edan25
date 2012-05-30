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
#include <ctype.h>
#include <unistd.h>

#include "driver.h"
#include "error.h"
#include "vertex.h"

//#define THOROUGH_FREE_LOCKS

#ifdef DEBUG1
#define TRACE1 if(1)
#else
#define TRACE1 if(0)
#endif

#ifdef DEBUG2
#define TRACE2 if(1)
#else
#define TRACE2 if(0)
#endif

#ifdef DEBUG_SPU_MANAGER
#define TRACE_SPU_MANAGER if(1)
#else
#define TRACE_SPU_MANAGER if(0)
#endif

//#define DEBUG_ACQUIRE_NEXT
#ifdef DEBUG_ACQUIRE_NEXT
#define TRACE_ACQUIRE_NEXT if(1)
#else
#define TRACE_ACQUIRE_NEXT if(0)
#endif

typedef struct slist_t{
    struct slist_t* prev;
    struct slist_t* next;
    int v_index;
} slist_t;

extern spe_program_handle_t dataflow;
char*		progname;

pthread_mutex_t wl_mutex;
pthread_cond_t wl_cond;

unsigned int acquired_locks_ctr;
pthread_mutex_t acquired_locks_mutex;

pthread_mutex_t* mutexes = NULL;
vertex_t *vertices = NULL;
slist_t *worklist = NULL;
slist_t *worklist_last = NULL;

unsigned int spu_work_ctr[6][2];// DELETEME:
unsigned int spu_work_next[6][100];// DELETEME:
unsigned int spu_work_returned[6][100];// DELETEME:

void print_spu_work(){
    for(unsigned int i=0;i < 6; ++i){
        printf("spu_%d_work_next{",i);
        for(unsigned int j = 0; j < spu_work_ctr[i][0]; ++j){
            printf("%d ",  spu_work_next[i][j]);
        }
        printf("}\nspu_%d_work_return{",i);
        for(unsigned int j = 0; j < spu_work_ctr[i][1]; ++j){
            printf("%d ",  spu_work_next[i][j]);
        }
        printf("}\n");
    }
}

typedef struct {
	spe_context_ptr_t	ctx;
	pthread_t		pthread;
	pthread_t		manager_pthread;
    unsigned short	spu_num;
	void*			arg;
} arg_t A16;


slist_t* slist_create_node(int v_index){
	slist_t* tmp = malloc(sizeof(slist_t));
	tmp->next = tmp->prev = NULL;
	tmp->v_index = v_index;
	return tmp;
}


bool slist_contains(int v_index){
    slist_t* tmp = worklist;
    while(tmp != NULL){
        if(tmp->v_index == v_index){
            return true;
        }
        tmp = tmp->next;
    }
    return false;
}


void slist_insert_last(slist_t* last, void* wholock){
    wholock = wholock;
    //printf("Thread with ctx: %p LOCK\n", wholock);
    pthread_mutex_lock(&wl_mutex);
    //printf("Thread with ctx: %p ACQUIRES\n", wholock);
    if(worklist_last == NULL){
        worklist_last = worklist = last;
    } else if (!slist_contains(last->v_index)){
        last->prev = worklist_last;
        worklist_last->next = last;
        worklist_last = last;
    }
    pthread_mutex_unlock(&wl_mutex);
    //printf("Thread with ctx: %p UNLOCK\n", wholock);

}


int slist_pop_first(void* wholock){
    wholock = wholock;
    //printf("Thread with ctx: %p LOCK\n", wholock);
    pthread_mutex_lock(&wl_mutex);

    //printf("Thread with ctx: %p ACQUIRES\n", wholock);
    if(worklist == NULL){
        pthread_mutex_unlock(&wl_mutex);
        //printf("Thread with ctx: %p UNLOCK\n", wholock);
        return -1; // Nothing to see here
    }
    int ret = worklist->v_index;
    if(worklist == worklist_last){ //Only 1 element
        free(worklist);
        worklist = worklist_last = NULL;
        pthread_mutex_unlock(&wl_mutex);
        //printf("Thread with ctx: %p UNLOCK\n", wholock);
        return ret;
    }
    slist_t* tmp = worklist;
    worklist = worklist->next;
    worklist->prev = NULL;
    free(tmp);
    pthread_mutex_unlock(&wl_mutex);
    //printf("Thread with ctx: %p UNLOCK\n", wholock);
    return ret;
}

void slist_print_worklist(){
    slist_t* tmp = worklist;
    printf(" { ");
    while(tmp != NULL){
        printf("%d ", tmp->v_index);
        tmp = tmp->next;
    }
    printf("}\n");
}

/*
void test_list(){
    for(int i = 0; i < 10; ++i){
        slist_insert_last(slist_create_node(i));
    }
    printf("10 elems inserted\n");
    slist_print_worklist();
    for(int i = 0; i < 4; ++i){
        slist_pop_first();
    }
    printf("4 elems removed\n");
    slist_print_worklist();
    for(int i = 0; i < 7; ++i){
        slist_pop_first();
    }
    printf("7(11) elems removed\n");
    slist_print_worklist();
}*/

static
double sec(void){
	struct timeval tv;
	gettimeofday(&tv, NULL);
	return (double) tv.tv_sec + (double)tv.tv_usec / 1000000;
}


void test_abi(arg_t* data){
    mail_t mail;
    mail_t mail_rec;

    mail.uint = 0;
    mail.str.vertex_done = 1;
    mail_rec.uint = 0;
	spe_in_mbox_write(data->ctx, &mail.uint, 1, 1);
    spe_out_intr_mbox_read(data->ctx, &mail_rec.uint, 1, SPE_MBOX_ALL_BLOCKING);

    printf("PPU vertex_done: %u\tuint: %u\t:\n", mail_rec.str.vertex_done, mail_rec.uint);

    mail.uint = 0;
    mail.str.requeue_vertex = 1;
    mail_rec.uint = 0;
	spe_in_mbox_write(data->ctx, &mail.uint, 1, 1);
	spe_out_intr_mbox_read(data->ctx, &mail_rec.uint, 1, SPE_MBOX_ALL_BLOCKING);
    printf("PPU requeue_vertex: %u\n", mail_rec.str.requeue_vertex);

    mail.uint = 0;
    mail.str.get_next_vertex = 1;
    mail_rec.uint = 0;
	spe_in_mbox_write(data->ctx, &mail.uint, 1, 1);
    spe_out_intr_mbox_read(data->ctx, &mail_rec.uint, 1, SPE_MBOX_ALL_BLOCKING);
    printf("PPU get_next_vertex: %u\n", mail_rec.str.get_next_vertex);

    mail.uint = 0;
    mail.str.complete_execution = 1;
    mail_rec.uint = 0;
	spe_in_mbox_write(data->ctx, &mail.uint, 1, 1);
    spe_out_intr_mbox_read(data->ctx, &mail_rec.uint, 1, SPE_MBOX_ALL_BLOCKING);
    printf("PPU complete_execution: %u\n", mail.str.complete_execution);

    mail.uint = 0;
    mail.str.start_execution = 1;
    mail_rec.uint = 0;
	spe_in_mbox_write(data->ctx, &mail.uint, 1, 1);
    spe_out_intr_mbox_read(data->ctx, &mail_rec.uint, 1, SPE_MBOX_ALL_BLOCKING);
    printf("PPU start_execution: %u\n", mail.str.start_execution);

    mail.uint = 0;
    mail.str.vertex_number = 134217727;
    mail_rec.uint = 0;
	spe_in_mbox_write(data->ctx, &mail.uint, 1, 1);
    spe_out_intr_mbox_read(data->ctx, &mail_rec.uint, 1, SPE_MBOX_ALL_BLOCKING);

    printf("PPU vertex_number: %u\n", mail.str.vertex_number);
}

inline
unsigned int pad_length(unsigned int input){
    if(input<=1)
        return 1;
    if(input<=2)
        return 2;
    if(input<=4)
        return 4;
    if(input<=8)
        return 8;
    if(input<=16)
        return 16;

    unsigned int mod = input % 16;
    if(mod == 0){
        return input;
    } else {
        return input + 16 - mod;
    }
}


void* work(void *arg)
{
	arg_t* data = arg;
	unsigned int entry = SPE_DEFAULT_ENTRY;

	if (spe_context_run(data->ctx, &entry, 0, data->arg, NULL, NULL) < 0) {
		perror("Failed running context");
		exit (1);
	}

	//printf("PPU pthread sees SPU has terminated.\n");

	pthread_exit(NULL);  
}


int acquire_next_dummy(void * wholock){
    int ret = slist_pop_first(wholock);
    return ret;
}


bool acquire_locks(int v_index){
    unsigned int needed_locks = 1;
    unsigned int saved_locks = 0;
    bool failed = false;

    needed_locks += vertices[v_index].pred_count + vertices[v_index].succ_count;
    int local_mutexes[needed_locks];

    if( pthread_mutex_trylock(&mutexes[v_index]) ){
        local_mutexes[0] = v_index;
        ++saved_locks;
    } else {
        failed = true;
    }
    for(unsigned int i = 0; (i < vertices[v_index].succ_count) && !failed; ++i){
        if(pthread_mutex_trylock(&mutexes[vertices[v_index].succ_list[i]])){
            local_mutexes[saved_locks++] = vertices[v_index].succ_list[i];
        } else {
            failed = true;

        }
    }

    for(unsigned int i = 0; (i < vertices[v_index].pred_count) && !failed; ++i){
        if(pthread_mutex_trylock(&mutexes[vertices[v_index].pred_list[i]])){
            local_mutexes[saved_locks++] = vertices[v_index].pred_list[i];
        } else {
            failed = true;
        }
    }

    if(failed){
        for(unsigned int j = 0; (j < saved_locks); ++j){
            pthread_mutex_unlock(&mutexes[local_mutexes[j]]);
        }
        return false;
    } else {
        /*printf("acquire locks { ");
        for(unsigned int i = 0; i < saved_locks; ++i){
            printf("%d ", local_mutexes[i]);
        }
        printf("}\n");*/
        pthread_mutex_lock(&acquired_locks_mutex);
        acquired_locks_ctr += saved_locks;
        //printf("acquire_locks(%d): acquired_locks_ctr=%d\t saved_locks=%d needed_locks=%d \n", v_index, acquired_locks_ctr,saved_locks,needed_locks);
        pthread_mutex_unlock(&acquired_locks_mutex);
        return true;
    }
}


/*
 * -1 is returned if no next vertex can be returned (ie we're done = empty worklist & no locks).
 */
int acquire_next(int wholocked){
    int ret;
    //printf("PPU aquire_next\n");
    
    while(acquired_locks_ctr > 0 || worklist != NULL){
        ret = slist_pop_first(NULL);
        TRACE_ACQUIRE_NEXT printf("PPU-%d acquired_locks_ctr=%u   ret=%d   worklist=",wholocked, ret, acquired_locks_ctr);
        TRACE_ACQUIRE_NEXT slist_print_worklist();
        bool aq;
        if (ret != -1 && (aq = acquire_locks(ret))){ //Non empty list && lockable
            TRACE_ACQUIRE_NEXT printf("\t %s\n", aq ? "ACQUIRED" : "DENIED");
            //printf("acquire_next(): return ret!\n");
            vertices[ret].listed = false;
            return ret;
        } else if (ret >= 0) {
            slist_insert_last(slist_create_node(ret), NULL);
            //printf("acquire_next(): slist_insert_last!\n");
        } else {
            //return -1;
            //printf("acquire_next(): whoops!\n");
        }
    }
    return -1;
}


void free_locks(int v_index){
#ifdef THOROUGH_FREE_LOCKS
    if(pthread_mutex_trylock(&mutexes[v_index])){
        printf("ERROR! An unlocked (vertex)lock was free'd\n");
        exit(0);
    }
#endif
    //printf("Free locks: { ");
    for(unsigned int i = 0;i < vertices[v_index].succ_count; ++i){
        //printf("%d ", vertices[v_index].succ_list[i]);
#ifdef THOROUGH_FREE_LOCKS
        if (pthread_mutex_trylock(&mutexes[vertices[v_index].succ_list[i]])){
            printf("ERROR! An unlocked (succ_list)lock was free'd\n");
            exit(0);
        }
#endif
        pthread_mutex_unlock(&mutexes[vertices[v_index].succ_list[i]]);
    }

    for(unsigned int i = 0;i < vertices[v_index].pred_count; ++i){
        //printf("%d ", vertices[v_index].pred_list[i]);
#ifdef THOROUGH_FREE_LOCKS
        if (pthread_mutex_trylock(&mutexes[vertices[v_index].pred_list[i]])){
            printf("ERROR! An unlocked (pred_list)lock was free'd\n");
            exit(0);
        }
#endif
        pthread_mutex_unlock(&mutexes[vertices[v_index].pred_list[i]]);
    }
    pthread_mutex_unlock(&mutexes[v_index]);
    //printf("%d }\n", v_index);
    pthread_mutex_lock(&acquired_locks_mutex);
    acquired_locks_ctr -= 1 + vertices[v_index].pred_count + vertices[v_index].succ_count;
    //printf("free_locks(%d): acquired_locks_ctr=%d\n", v_index, acquired_locks_ctr);
    pthread_mutex_unlock(&acquired_locks_mutex);
    //printf("Unlocked vertex.\n");
}

void print_mail_t(mail_t mail){
    //printf("\tmail_t, uint: %u\n", mail.uint);
    printf("\tvertex_done: %u\n", mail.str.vertex_done);
    printf("\trequeue_vertex: %u\n", mail.str.requeue_vertex);
    printf("\tget_next_vertex: %u\n", mail.str.get_next_vertex);
    printf("\tcomplete_execution: %u\n", mail.str.complete_execution);
    printf("\tstart_execution: %u\n", mail.str.start_execution);
    printf("\tvertex_number: %u\n", mail.str.vertex_number);
}

void* spu_manager(void *arg){
    arg_t* data = arg;
    int tmp_vertex;

    mail_t mail;
    mail_t mail_rec;

    // Send first message containing both vertex_number and start_execution
    mail.uint = 0;
    mail.str.start_execution = 1;


    tmp_vertex = acquire_next(data->spu_num);
    spu_work_next[data->spu_num][spu_work_ctr[data->spu_num][0]] = tmp_vertex;
    spu_work_ctr[data->spu_num][0] += 1;

    if(tmp_vertex < 0){
        mail.str.complete_execution = 1;
    } else {
        mail.str.get_next_vertex = 1;
        mail.str.vertex_number = tmp_vertex;
    }

    TRACE_SPU_MANAGER printf("PPU-%d first mbox write.\n", data->spu_num);
    spe_in_mbox_write(data->ctx, &mail.uint, 1, 1);
    TRACE_SPU_MANAGER printf("PPU-%d after first mbox write.\n", data->spu_num);


    while(1){


        TRACE_SPU_MANAGER printf("PPU-%d fst loop read.\n", data->spu_num);
        spe_out_intr_mbox_read(data->ctx, &mail_rec.uint, 1, SPE_MBOX_ALL_BLOCKING);

        TRACE_SPU_MANAGER printf("PPU-%d after fst loop read.\n", data->spu_num);

        if(mail_rec.str.vertex_done == 1){
            TRACE_SPU_MANAGER printf("PPU-%d mail_rec.str.vertex_done == 1\n", data->spu_num);
            spu_work_returned[data->spu_num][spu_work_ctr[data->spu_num][1]] = mail_rec.str.vertex_number;
            spu_work_ctr[data->spu_num][1] +=1;
            free_locks(mail_rec.str.vertex_number);
            ///print_spu_work();
        }else if(mail_rec.str.requeue_vertex == 1){
            TRACE_SPU_MANAGER printf("PPU-%d mail_rec.str.requeue_vertex-%d == 1\n", data->spu_num, mail_rec.str.vertex_number);
            vertices[mail_rec.str.vertex_number].listed = true;
            slist_insert_last(slist_create_node(mail_rec.str.vertex_number),(void*)&data->ctx);
            continue;
        }
        if(mail_rec.str.get_next_vertex == 1){
            TRACE_SPU_MANAGER printf("PPU-%d mail_rec.str.get_next_vertex == 1\n", data->spu_num);
            tmp_vertex = acquire_next(data->spu_num);
            //slist_print_worklist();
        }
        if(tmp_vertex < 0){//|| mail_rec.str.complete_execution){
            TRACE_SPU_MANAGER printf("PPU-%d tmp_vertex < 0\n", data->spu_num);
            mail.uint = 0;
            break;
        } else {
            mail.str.get_next_vertex = 1;
            mail.str.vertex_number = tmp_vertex;
            spu_work_next[data->spu_num][spu_work_ctr[data->spu_num][0]] = tmp_vertex;
            spu_work_ctr[data->spu_num][0] +=1;
            //print_spu_work();
        }
        TRACE_SPU_MANAGER printf("PPU-%d end of loop, writing mbox.\n", data->spu_num);

        spe_in_mbox_write(data->ctx, &mail.uint, 1, 1);

        TRACE_SPU_MANAGER printf("PPU-%d end of loop, after writing mbox.\n", data->spu_num);

        mail.uint = 0;
    }
    mail.str.complete_execution = 1;

    printf("PPU-%d loop complete! tell spu to complete execution.\n", data->spu_num);

    spe_in_mbox_write(data->ctx, &mail.uint, 1, 1);
    return NULL;
}


int main(int argc, char** av) 
{   
    double  begin;
	double  end;
	int     errnum;
	int     maxsucc, nactive, nspu, nvertex, nsym;
    bool    print_input, print_output;

    if (argc >= 8){
	    sscanf(av[1], "%d", &nsym);
	    sscanf(av[2], "%d", &nvertex);
	    sscanf(av[3], "%d", &maxsucc);
	    sscanf(av[4], "%d", &nactive);
	    sscanf(av[5], "%d", &nspu);
        if(nspu < 1 || nspu > P){
            nspu = P;
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
        nsym = 100;
        nvertex = 8;
        maxsucc = 4;
        nactive = 10;
        nspu = 5;
        print_output = false;
	    print_input = false;
    }

    arg_t   data[nspu];
    param_t param[nspu] A16;
	argc 		= argc; 	// to silence gcc...
	progname	= av[0];


	printf("nsym   = %zu\n", nsym);
	printf("nvertex   = %zu\n", nvertex);
	printf("maxsucc   = %zu\n", maxsucc);
	printf("nactive   = %zu\n", nactive);
	printf("nspu   = %zu\n", nspu);
	printf("param_t size   = %zu\n", sizeof(param_t));
	printf("arg_t size  = %zu\n", sizeof(arg_t));
    printf("bitset_size = %u\n", 50*(nsym / (sizeof(unsigned int) * 8)) + 1);
    printf("bitset_size (padded) = %u\n", pad_length(50*(nsym / (sizeof(unsigned int) * 8)) + 1));
    printf("sizeof(int): %db\n", sizeof(int));
    printf("sizeof(void*): %db\n", sizeof(void*));
    printf("sizeof(char): %db\n", sizeof(char));
    printf("sizeof(vertex_t): %db\n", sizeof(vertex_t));

    pthread_mutex_init(&wl_mutex, NULL);
    pthread_mutex_init(&acquired_locks_mutex, NULL);
    mutexes = malloc(sizeof(pthread_mutex_t)*nvertex);
    for(int i = 0; i < nvertex; ++i){
        pthread_mutex_init(&mutexes[i], NULL);
    }
    vertices = create_vertices(nsym, nvertex, maxsucc, nactive, print_input);

	begin = sec();

    for(int i = 0; i < nvertex; ++i){
        vertices[i].listed = true;
        slist_insert_last(slist_create_node(i),(void*)&data->ctx);
    }

	for (int i = 0; i < nspu; ++i) {
		data[i].spu_num = param[i].proc = i;
		param[i].nvertex = nvertex;
        param[i].vertices = vertices;
        param[i].nsym = nsym;
        param[i].bitset_size = pad_length( 50*(nsym / (sizeof(unsigned int) * 8)) + 1 );

		if ((data[i].ctx = spe_context_create (0, NULL)) == NULL) {
			perror ("Failed creating context");
			exit(1);
		}

		if (spe_program_load (data[i].ctx, &dataflow))  {
			perror ("Failed loading program");
			exit(1);
		}

		data[i].arg = &param[i];

		if ( pthread_create(&data[i].pthread, NULL, work, &data[i]) ) {
			perror ("Failed creating thread");
			exit(1);
		}

		if ( pthread_create(&data[i].manager_pthread, NULL, spu_manager, &data[i]) ) {
			perror ("Failed creating manager thread");
			exit(1);
		} 
	}



    if(print_output){
        for(int i = 0; i < nvertex; ++i){
            print_vertex(&vertices[i]);
        }
    }

	for (int i = 0; i < nspu; ++i) {
		errnum = pthread_join(data[i].pthread, NULL);
		//printf("joining with PPU pthread %zu...\n", i);
		if (errnum != 0)
			syserror(errnum, "pthread_join failed");

		if (spe_context_destroy (data[i].ctx) != 0) {
			perror("Failed destroying context");
			exit(1);
		}
	}


	end = sec();

	printf("%1.3lf s\n", end-begin);

	return 0;
}

