//#include <pthread.h>
#include "bitset.h"
#include "rand.h"
#include <math.h>
#include <sys/times.h>
#include <sys/time.h>
#include <ctype.h>

bool print_input;
int nvertex;
int	nsym;

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
	BitSet_struct* in;
	BitSet_struct* out;
	BitSet_struct* use;
	BitSet_struct* def;
	//pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
} Vertex;

Vertex* new_vertex(int i){
	Vertex* v = malloc(sizeof(Vertex));
	v->index = i;
	v->listed = false;
	v->pred_list = create_node(NULL); //First element = NULL
	v->succ_list = create_node(NULL); //First element = NULL
	v->in = bitset_create();
	v->out = bitset_create();
	v->use = bitset_create();
	v->def = bitset_create();
	return v;
}

void computeIn(Vertex* u, list_t* worklist){
	BitSet_struct* old;
	Vertex* v;

	list_t* tmp_list = u->succ_list;
	do{
        tmp_list = tmp_list->next;
		v = tmp_list->data;
        if(v != NULL){
		    bitset_or(u->out, v->in);
        }
	}while(tmp_list->next != tmp_list);
	old = bitset_copy(u->in);
	u->in = bitset_create();
	bitset_or(u->in, u->out);
	bitset_and_not(u->in, u->def);
	bitset_or(u->in, u->use);

	if(!bitset_equals(u->in, old)){
		tmp_list = u->pred_list;
		do{
            tmp_list = tmp_list->next;
			v = tmp_list->data;
			if(v != NULL && !(v->listed)){
				add_last(worklist, create_node(v));
				v->listed = true;
			}
		}while(tmp_list->next != tmp_list);
	}
}

void print_vertex(Vertex* v){
	int i;

	printf("use[%d] = { ", v->index);
	for (i = 0; i < nsym; ++i){
		if (bitset_get_bit(v->use, i)){
			printf("%d ", i);
		}
	}
	printf("}\n");
	printf("def[%d] = { ", v->index);

	for (i = 0; i < nsym; ++i){
		if (bitset_get_bit(v->def, i)){
			printf("%d ", i);
		}
	}
	printf("}\n\n");
	printf("in[%d] = { ", v->index);

	for (i = 0; i < nsym; ++i){
		if (bitset_get_bit(v->in, i)){
			printf("%d ", i);
		}
	}
	printf("}\n");
	printf("out[%d] = { ", v->index);

	for (i = 0; i < nsym; ++i){
		if (bitset_get_bit(v->out, i)){
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


			if (j % 4 != 0) {
				if(!bitset_get_bit(v->def, sym)){
					if(print_input){
						printf(" u %d", sym);
					}
					bitset_set_bit(v->use, sym, true);
				}
			}else{
				if(!bitset_get_bit(v->use, sym)){
					if(print_input){
						printf(" d %d", sym);
					}
					bitset_set_bit(v->def, sym, true);
				}
			}
		}
		if(print_input){
			printf("}\n");
		}
	}while(tmp_list->next != tmp_list);
}

void liveness(list_t* vertex_list){
    //printf("in liveness\n");
	Vertex* u;
	Vertex* v;
	list_t* worklist = create_node(NULL);
	double begin;
	double end;

	begin = sec();

	list_t* tmp_list = vertex_list;//->next;
	while(tmp_list->next != tmp_list){
        tmp_list = tmp_list->next;
		v = tmp_list->data;
		v->listed = true;
		add_last(worklist, create_node(v));
	}

	while(worklist->next != worklist){ // while (!worklist.isEmpty())
        u = remove_node(worklist->next);
        //biprintf("u->index = %d\n", u->index);
		u->listed = false;
		computeIn(u, worklist);
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

    tmp_list = vertex;
	for (i = 0; i < nvertex; ++i){
        insert_after(tmp_list, create_node(new_vertex(i)));
		tmp_list = tmp_list->next;
	}

	generateCFG(vertex, maxsucc, r);
	generateUseDef(vertex, nsym, nactive, r);
	liveness(vertex);

	if(print_output){
        tmp_list = vertex->next;

		for (i = 0; i < nvertex; ++i){
			print_vertex(tmp_list->data);
			tmp_list = tmp_list->next;
		}
	}

	return 0;
}
