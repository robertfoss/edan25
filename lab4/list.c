#include "list.h"

list_t* create_node(void* data){
	list_t* tmp = malloc(sizeof(list_t));
	tmp->next = tmp->prev = tmp;
	tmp->data = data;
	return tmp;
}

void* remove_node(list_t* list){
    void* data_tmp = list->data;

    if(list->next != list){
	    list->prev->next = list->next;
	    list->next->prev = list->prev;
	    list->next = list->prev = list;
    } else {
        list->prev->next = list->prev;
    }

    free(list);
    return data_tmp;
}

void insert_after(list_t* target, list_t* new_node){
    if (target == target->next){ // Incase of target being the last node.
        target->next = new_node;
        new_node->next = new_node;
        new_node->prev = target;
    } else {
        list_t* old_next = target->next;
        target->next = new_node;
        new_node->prev = target;
        new_node->next = old_next;
        old_next->prev = new_node;
    }
}

void insert_before(list_t* target, list_t* new_node){
    if (target == target->prev){ // Incase of target being the first node.
        printf("insert before, target is first node\n");
        new_node->prev = new_node;
        target->prev = new_node;
        new_node->next = target;
    } else {
        printf("insert before, else\n");
        new_node->prev = target->prev;
        target->prev = new_node;
        new_node->next = target;
    }
}

void add_last(list_t* list, list_t* new_node){
	list_t* tmp = list;
	while(tmp->next != tmp){
		tmp = tmp->next;
	}
	insert_after(tmp, new_node);
}
