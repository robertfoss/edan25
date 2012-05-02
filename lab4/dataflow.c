#include <pthread.h>
//#include "bitset.h"
#include "rand.h"
#include <math.h>
#include <sys/times.h>
#include <sys/time.h>
#include <ctype.h>
#include <stdbool.h>
#include "list.h"

bool print_input;
int nvertex;
int	nsym;
unsigned int alloc_size;


static double sec(void){
	struct timeval tv;
	gettimeofday(&tv, NULL);
	return (double) tv.tv_sec + (double)tv.tv_usec / 1000000;
}

typedef struct{
	int index;
	bool listed;
	list_t* pred_list;
	list_t* succ_list;
	unsigned int* in;
	unsigned int* out;
	unsigned int* use;
	unsigned int* def;
	pthread_mutex_t mutex;
    pthread_cond_t cond;
} Vertex;

Vertex* new_vertex(int i){
	Vertex* v = malloc(sizeof(Vertex));
	v->index = i;
	v->listed = false;
	v->pred_list = create_node(NULL); //First element = NULL
	v->succ_list = create_node(NULL); //First element = NULL

	v->in = calloc( alloc_size, sizeof(unsigned int));
	v->out = calloc( alloc_size, sizeof(unsigned int));
	v->use = calloc( alloc_size, sizeof(unsigned int));
	v->def = calloc( alloc_size, sizeof(unsigned int));
    if(v->in == NULL || v->out == NULL || v->use == NULL || v->def == NULL)
        printf("calloc returned null\n");
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
            mutexes[saved_locks] = v->mutex;
            conds[saved_locks] = v->cond;
            ++saved_locks;
        }
	}

    // Acquire all or no locks.
    while(1){
        //printf("while(1)\n");
        acquired_locks = 0;
        for(int i = 0; i < needed_locks; ++i){
            if(pthread_mutex_trylock(&mutexes[i])){
                ++acquired_locks;
                if(acquired_locks == needed_locks){
                    //printf("Acquired all locks.\n");
                    return;
                }
            } else {
                //printf("Failed to acquire lock.\n");
                for(int j = 0; j < acquired_locks; ++j){
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
    unsigned int* new_bs = calloc(  alloc_size, sizeof(unsigned int));
    for(int i = 0; i < (sizeof(unsigned int) * (nsym / (sizeof(unsigned int) * 8) + 1)); ++i){
        new_bs[i] = bs[i];
    }
    return new_bs;
}

bool bitset_equals(unsigned int* bs1, unsigned int* bs2){
    for(int i = 0; i < (sizeof(unsigned int) * (nsym / (sizeof(unsigned int) * 8) + 1)); ++i){
        if(bs1[i] != bs2[i]){
            return false;
        }
    }
    return true;
}

void bitset_or(unsigned int* bs1, unsigned int* bs2){
    for(int i = 0; i < (sizeof(unsigned int) * (nsym / (sizeof(unsigned int) * 8) + 1)); ++i){
        bs1[i] |= bs2[i];
    }
}

void bitset_and_not(unsigned int* bs1, unsigned int* bs2){
    for(int i = 0; i < (sizeof(unsigned int) * (nsym / (sizeof(unsigned int) * 8) + 1)); ++i){
        unsigned int tmp = bs1[i] & bs2[i];
        tmp = ~tmp;
        bs1[i] = tmp & bs1[i];
    }
}

void computeIn(Vertex* u, list_t* worklist){
	//BitSet_struct* old;
	Vertex* v;
    acquire_locks(u, u->succ_list, u->pred_list);

	list_t* tmp_list = u->succ_list;
	do{
        tmp_list = tmp_list->next;
		v = tmp_list->data;
        if(v != NULL){
		    bitset_or(u->out, v->in);
        }
	}while(tmp_list->next != tmp_list);

	unsigned int* old = bitset_copy(u->in);
    /*printf("old:\n");
    bitset_print(old);*/
	u->in = calloc( alloc_size, sizeof(unsigned int));//bitset_create();
	bitset_or(u->in, u->out);
    /*printf("after or #1:\n");
    bitset_print(u->in);*/
	bitset_and_not(u->in, u->def);
    /*printf("after and not:\n");
    bitset_print(u->in);*/
	bitset_or(u->in, u->use);
    /*printf("after or #2:\n");
    bitset_print(u->in);*/

	if(!bitset_equals(u->in, old)){
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

void generateUseDef(list_t* vertex_list, int nsym, int nactive, Random* r){
    //printf("in generateUseDef\n");
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

            if(v == NULL)
                printf("v == NULL\n");
            if(v->use == NULL)
                printf("v->use == NULL\n");
            if(v->def == NULL)
                printf("v->def == NULL\n");

			if (j % 4 != 0) {
				if(!bitset_get_bit(v->def, sym)){//!bitset_get_bit(v->def, sym)){
					if(print_input){
						printf(" u %d", sym);
					}
					bitset_set_bit(v->use, sym);//bitset_set_bit(v->use, sym, true);
				}
			}else{
				if(!bitset_get_bit(v->use, sym)){//!bitset_get_bit(v->use, sym)){
					if(print_input){
						printf(" d %d", sym);
					}
					bitset_set_bit(v->def, sym);//bitset_set_bit(v->def, sym, true);
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
	Vertex* u;
    unsigned int work_counter = 0;
	while(worklist->next != worklist){ // while (!worklist.isEmpty())
        u = remove_node(worklist->next);
        //printf("u->index = %d\n", u->index);
		u->listed = false;
		computeIn(u, worklist);
        work_counter++;
	}
    printf("Thread[%u] worked %u iterations.\n", index, work_counter);
    return NULL;
}

void liveness(list_t* vertex_list, int nthread){
    //printf("in liveness\n");
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
/*    for(int i = 0; i < nthread; ++i){
        printf("worksplit[%d] = %d\n", i, worksplit[i]);
    }
*/
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
    printf("c runtime = %f s\n", (end - begin));

}

int main(int ac, char** av){

    if(ac != 8){
        printf("Wrong # of args (nsym nvertex maxsucc nactive nthreads print_output print_input)\n");
        exit(1);
    }

	int	i;
	int	maxsucc;
	int	nactive;
	int	nthread;
	bool print_output; 
	list_t* vertex;
	Random* r = new_random();
    list_t* tmp_list;

	setSeed(r, 1);
	vertex = create_node(NULL); //First element = NULL

	char* tmp_string = "";

	sscanf(av[1], "%d", &nsym);
	sscanf(av[2], "%d", &nvertex);
	sscanf(av[3], "%d", &maxsucc);
	sscanf(av[4], "%d", &nactive);
	sscanf(av[5], "%d", &nthread);

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
    alloc_size = 50*(nsym / (sizeof(unsigned int) * 8)) + 1;

    tmp_list = vertex;
	for (i = 0; i < nvertex; ++i){
        insert_after(tmp_list, create_node(new_vertex(i)));
		tmp_list = tmp_list->next;
	}

	generateCFG(vertex, maxsucc, r);
	generateUseDef(vertex, nsym, nactive, r);
	liveness(vertex, nthread);

	if(print_output){
        tmp_list = vertex->next;

		for (i = 0; i < nvertex; ++i){
			print_vertex(tmp_list->data);
			tmp_list = tmp_list->next;
		}
	}

	return 0;
}
