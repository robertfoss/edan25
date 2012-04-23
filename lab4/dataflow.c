//#include <pthread.h>
#include "bitset.h"
#include "rand.h"
#include "list.h"
#include <math.h>

int output_input;
int nvertex;

static double sec(void){
	struct timeval tv;
	gettimeofday(&tv, NULL);
	return (double) tv.tv_sec + (double)tv.tv_usec / 1000000;
}

typedef struct{
	int index;
	int listed; //0 not listed, !0 listed
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
	return v;
}

void computeIn(Vertex* u, list_t* worklist){
	BitSet_struct old;
	Vertex* v;

	list_t* tmp_list = u->succ_list;
	while(tmp_list->next != tmp_list){ //End of list
		v = tmp_list->data;
		bitset_or(out, v->in);
	}

	old = u->in;
	BitSet_struct tmp_bs;
	u->in = tmp_bs;
	bitset_or(in, out);
	bitset_and_not(in, def);
	bitset_or(in, use);

	if(bitset_equals(in, out)){
		tmp_list = u->pred;
		while(tmp_list->next != tmp_list){ //End of list
			v = tmp_list->data;
			if(!(v->listed)){
				add_last(worklist, create_node(v));
				v->listed = 1;
			}
		}
	}
}

void print_vertex(Vertex* v){

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
	list_t* tmp_list = vertex_list;

	connect(tmp_list->data, tmp_list->next->data);
	tmp_list = tmp_list->next;
	connect(tmp_list->data, tmp_list->next->data);
	tmp_list = tmp_list->next;

	while(tmp_list->next != tmp_list){
		if(output_input){
			printf("[%d] succ = {", i);
		}
		s = (nextRand(r) % maxsucc) + 1;
		for (j = 0; j < s; ++j) {
			k = abs(nextRand(r)) % nvertex; //vertex.length
			if(output_input){
				printf(" %d", k);
			}
			connect(tmp_list->data, tmp_list->next->data);
		}
		if(output_input){
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
	list_t* tmp_list = vertex_list;
	Vertex* v;

	while(tmp_list->next != tmp_list){
		v = tmp_list->data; //vertex_list[i]

		if(output_input){
			printf("[%d] usedef = {", i);
		}

		for (j = 0; j < nactive; ++j) {
			sym = abs(nextRand(r)) % nsym;

			if (j % 4 != 0) {
				if(bitset_get_bit(v->def, sym)){ //!vertex[i].def.get(sym) 
					if(output_input){
						printf(" u %d", sym);
					}
					bitset_set_bit(v->use, sym); //vertex[i].use.set(sym);
				}
			}else{
				if(bitset_get_bit(v->use, sym)){ //!vertex[i].use.get(sym)
					if(output_input){
						printf(" d %d", sym);
					}
					bitset_set_bit(v->def, sym); //vertex[i].def.set(sym);
				}
			}
		}
		if(output_input){
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

	list_t* tmp_list = vertex_list;

	while(tmp_list->next != tmp_list){
		v = tmp_list->data;
		add_last(worklist, create_node(v));
		v->listed = 1;
	}

	while(worklist->next != worklist){
		tmp_list = worklist->next; //worklist.remove(); 
		u = worklist->data; 
		remove_node(worklist);
		worklist = tmp_list

		u->listed = 0;
		computeIn(u, worklist);
	}

	end = sec();
}

int main(){
	return 0;
}

















