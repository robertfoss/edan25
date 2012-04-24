//#include <pthread.h>
#include "bitset.h"
#include "rand.h"
#include "list.h"
#include <math.h>
#include <sys/times.h>
#include <sys/time.h>
#include <stdbool.h>
#include <stdlib.h>

bool print_input;
int nvertex;

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
	BitSet_struct in;
	BitSet_struct out;
	BitSet_struct use;
	BitSet_struct def;
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
	BitSet_struct old;
	Vertex* v;

	list_t* tmp_list = u->succ_list->next;
	while(tmp_list->next != tmp_list){ //End of list
		v = tmp_list->data;
		bitset_or(out, v->in);
	}

	old = u->in;
	BitSet_struct tmp_bs = bitset_create();
	u->in = tmp_bs;
	bitset_or(in, out);
	bitset_and_not(in, def);
	bitset_or(in, use);

	if(bitset_equals(in, out)){
		tmp_list = u->pred_list->next;
		while(tmp_list->next != tmp_list){ //End of list
			v = tmp_list->data;
			if(!(v->listed)){
				add_last(worklist, create_node(v));
				v->listed = true;
			}
		}
	}
}

void print_vertex(Vertex* v){
//use, def max nsym

	int i;

	printf("use[%d] = { ", v->index);
	for (i = 0; i < nsym; ++i){ //i < use.size()
		if (bitset_get_bit(v->use, i)){ //use.get(i)
			printf("%d ", i);
		}
	}
	printf("}\n");
	printf("def[%d] = { ", v->index);

	for (i = 0; i < nsym; ++i){ //i < def.size()
		if (bitset_get_bit(v->def, i)){ //def.get(i)
			printf("%d ", i);
		}
	}
	printf("}\n");
	printf("in[%d] = { ", v->index);

	for (i = 0; i < nsym; ++i){ //i < in.size()
		if (bitset_get_bit(v->in, i)){ //in.get(i)
			printf("%d ", i);
		}
	}
	printf("}\n");
	printf("out[%d] = { ", v->index);

	for (i = 0; i < nsym; ++i){ //i < out.size()
		if (bitset_get_bit(v->out, i)){ //out.get(i)
			printf("%d ", i);
		}
	}
	printf("}\n");
}

void connect(Vertex* pred, Vertex* succ){
	add_last(pred->succ_list, succ);
	add_last(succ->pred_list, pred);
}

void generateCFG(list_t* vertex_list, int maxsucc, Random r){
	int i = 2;
	int j;
	int k;
	int s; // number of successors of a vertex.
	list_t* tmp_list = vertex_list->next;

	connect(tmp_list->data, tmp_list->next->data);
	tmp_list = tmp_list->next;
	connect(tmp_list->data, tmp_list->next->data);
	tmp_list = tmp_list->next;

	while(tmp_list->next != tmp_list){
		if(print_input){
			printf("[%d] succ = {", i);
		}
		s = (nextRand(r) % maxsucc) + 1;
		for (j = 0; j < s; ++j) {
			k = abs(nextRand(r)) % nvertex; //vertex.length
			if(print_input){
				printf(" %d", k);
			}
			connect(tmp_list->data, tmp_list->next->data);
		}
		if(print_input){
			printf("}\n");
		}
		tmp_list = tmp_list->next;
		++i;
	}
}

void generateUseDef(list_t* vertex_list, int nsym, int nactive, Random r){
	int i = 0;
	int j;
	int sym;
	list_t* tmp_list = vertex_list->next;
	Vertex* v;

	while(tmp_list->next != tmp_list){
		v = tmp_list->data; //vertex_list[i]

		if(print_input){
			printf("[%d] usedef = {", i);
		}

		for (j = 0; j < nactive; ++j) {
			sym = abs(nextRand(r)) % nsym;

			if (j % 4 != 0) {
				if(bitset_get_bit(v->def, sym)){ //!vertex[i].def.get(sym) 
					if(print_input){
						printf(" u %d", sym);
					}
					bitset_set_bit(v->use, sym, 1); //vertex[i].use.set(sym);
				}
			}else{
				if(bitset_get_bit(v->use, sym)){ //!vertex[i].use.get(sym)
					if(print_input){
						printf(" d %d", sym);
					}
					bitset_set_bit(v->def, sym, 1); //vertex[i].def.set(sym);
				}
			}
		}
		if(print_input){
			printf("}\n");
		}
		++i;
	}
}

void liveness(list_t* vertex_list){
	Vertex* u;
	Vertex* v;
	list_t* worklist;
	double begin;
	double end;

	begin = sec();

	list_t* tmp_list = vertex_list->next;

	while(tmp_list->next != tmp_list){
		v = tmp_list->data;
		add_last(worklist, create_node(v));
		v->listed = true;
	}

	while(worklist->next != worklist){
		tmp_list = worklist->next; //worklist.remove(); 
		u = worklist->data; 
		remove_node(worklist);
		worklist = tmp_list

		u->listed = false;
		computeIn(u, worklist);
	}

	end = sec();
}

int main(){

	int	i;
	int	nsym;
	//int	nvertex; //global
	int	maxsucc;
	int	nactive;
	//int	nthread;
	bool print_output;
	//bool print_input; //global
	//bool print_debug;
	list_t* vertex; //Vertex vertex[];
	Random r;

	setSeed(r, 1);
	vertex = create_node(NULL); //First element = NULL

	char* tmp_string;

	sscanf(av[1], "%d", &nsym); //nsym = Integer.parseInt(args[0]);
	sscanf(av[2], "%d", &nvertex); //nvertex = Integer.parseInt(args[1]);
	sscanf(av[3], "%d", &maxsucc); //maxsucc = Integer.parseInt(args[2]);
	sscanf(av[4], "%d", &nactive); //nactive = Integer.parseInt(args[3]);
	//sscanf(av[5], "%d", &nthread); //nthread = Integer.parseInt(args[4]);

	sscanf(av[6], "%d", &tmp_string);
	if(tolower(tmp_string[0]) == 't'){
		print_output = true; //print_output = Boolean.valueOf(args[5]).booleanValue();
	}else{
		print_output = false;
	}

	sscanf(av[7], "%d", &tmp_string);
	if(tolower(tmp_string[0]) == 't'){
		print_input = true;	//print_input = Boolean.valueOf(args[6]).booleanValue();
	}else{
		print_input = false;
	}

/*
	sscanf(av[8], "%d", &tmp_string);
	if(tolower(tmp_string[0]) == 't'){
		print_debug = true; //print_debug = Boolean.valueOf(args[7]).booleanValue();
	}else{
		print_debug = false;
	}
*/

	for (i = 0; i < nvertex; ++i){
		vertex->next = create_node(new_vertex(i));
	}

	generateCFG(vertex, maxsucc, r);
	generateUseDef(vertex, nsym, nactive, r);
	liveness(vertex);

	if(print_output){
		list_t* tmp_list = vertex->next;
		for (i = 0; i < nvertex; ++i){
			print(tmp_list->data);
			tmp_list = tmp_list->next;
		}
	}

	return 0;
}
