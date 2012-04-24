#include "list.h"

list_t* create_node(void* data){
	list_t* tmp = malloc(sizeof(list_t));
	tmp->data = data;
	tmp->next = tmp;
	tmp->prev = tmp;
	return tmp;
}

void* remove_node(list_t* list){
    void* data_tmp = list->data;
	list_t* tmp = list->prev;
	tmp->next = list->next;
	list->next->prev = tmp;
    free(list);
    return data_tmp;
}

void insert_after(list_t* target, list_t* new_node){
	if (target == target->next){ // Incase of target being the last node
		target->next = new_node;
		new_node->next = new_node;
	} else {
		list_t* old_next = target->next;
		target->next = new_node;
		new_node->next = old_next;
		old_next->prev = new_node;
	}
}

void add_last(list_t* list, list_t* new_node){
	list_t* tmp = list;
	while(tmp->next != tmp){
		tmp = tmp->next;
	}
	insert_after(tmp, new_node);
}
