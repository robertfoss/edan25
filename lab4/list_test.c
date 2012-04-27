#include <assert.h>

#include "list.h"


void print_list(list_t* input){
    list_t* tmp = input;
    while(tmp->next != tmp){
        printf("%u\tnext: %u\tprev: %u\t\n", (unsigned int) (tmp->data), (unsigned int) (tmp->next->data), (unsigned int) (tmp->prev->data));
        tmp = tmp->next;
    }
    printf("%u\tnext: %u\tprev: %u\t\n", (unsigned int) (tmp->data), (unsigned int) (tmp->next->data), (unsigned int) (tmp->prev->data));
}


void test_insert_after(){
    printf("Test insert_after(): \t");

    printf("insert_after() chain. \t\t\t");
    int nbr = 0;
    list_t* fst = create_node((void*)nbr);
    list_t* lst = fst;
    list_t* tmp = fst;

    int nbr1 = 1;
    tmp = create_node((void*)nbr1);
    insert_after(lst, tmp);
    lst = tmp;

    int nbr2 = 2;
    tmp = create_node((void*)nbr2);
    insert_after(lst, tmp);
    lst = tmp;

    int nbr3 = 3;
    tmp = create_node((void*)nbr3);
    insert_after(lst, tmp);
    lst = tmp;

    int nbr4 = 4;
    tmp = create_node((void*)nbr4);
    insert_after(lst, tmp);
    lst = tmp;

    unsigned int counter = 0;

    tmp = fst;
    while(tmp->next != tmp){
        assert((unsigned int) (tmp->data) == counter++);
        tmp = tmp->next;
    }
    assert((unsigned int) (tmp->data) == counter++);
    printf("PASSED\n");
}

void test_remove_node(){
    printf("Test remove_node(): \t");

    printf("remove a single node. \t\t\t");
    int nbr = 0;
    list_t* fst = create_node((void*)nbr);
    list_t* lst = fst;
    list_t* tmp = fst;

    int nbr1 = 1;
    tmp = create_node((void*)nbr1);
    insert_after(lst, tmp);
    lst = tmp;

    int nbr2 = 2;
    tmp = create_node((void*)nbr2);
    insert_after(lst, tmp);
    lst = tmp;

    int nbr3 = 3;
    tmp = create_node((void*)nbr3);
    insert_after(lst, tmp);
    lst = tmp;

    int nbr4 = 4;
    tmp = create_node((void*)nbr4);
    insert_after(lst, tmp);
    lst = tmp;

    remove_node(fst->next->next);

    tmp = fst;
    assert((unsigned int) (tmp->data) == 0);
    tmp = tmp->next;
    assert((unsigned int) (tmp->data) == 1);
    tmp = tmp->next;
    assert((unsigned int) (tmp->data) == 3);
    tmp = tmp->next;
    assert((unsigned int) (tmp->data) == 4);
    tmp = tmp->next;
    printf("PASSED\n");

    printf("\t\t\tremove last node. \t\t\t");
    nbr = 0;
    tmp = lst = fst = create_node((void*)nbr);

    nbr1 = 1;
    tmp = create_node((void*)nbr1);
    insert_after(lst, tmp);
    lst = tmp;

    nbr2 = 2;
    tmp = create_node((void*)nbr2);
    insert_after(lst, tmp);
    lst = tmp;

    nbr3 = 3;
    tmp = create_node((void*)nbr3);
    insert_after(lst, tmp);
    lst = tmp;

    nbr4 = 4;
    tmp = create_node((void*)nbr4);
    insert_after(lst, tmp);
    lst = tmp;

    remove_node(lst);

    tmp = fst;
    assert((unsigned int) (tmp->data) == 0);
    tmp = tmp->next;
    assert((unsigned int) (tmp->data) == 1);
    tmp = tmp->next;
    assert((unsigned int) (tmp->data) == 2);
    tmp = tmp->next;
    assert((unsigned int) (tmp->data) == 3);
    tmp = tmp->next;
    assert((unsigned int) (tmp->data) == 3); // Make sure end->next = end.
    printf("PASSED\n");


    printf("\t\t\tremove 2 first nodes. \t\t\t");
    nbr = 0;
    tmp = lst = fst = create_node((void*)nbr);

    nbr1 = 1;
    tmp = create_node((void*)nbr1);
    insert_after(lst, tmp);
    lst = tmp;

    nbr2 = 2;
    tmp = create_node((void*)nbr2);
    insert_after(lst, tmp);
    lst = tmp;

    nbr3 = 3;
    tmp = create_node((void*)nbr3);
    insert_after(lst, tmp);
    lst = tmp;

    nbr4 = 4;
    tmp = create_node((void*)nbr4);
    insert_after(lst, tmp);
    lst = tmp;
    
    tmp = fst->next->next;
    remove_node(fst->next);
    remove_node(fst);

    tmp = lst;
    assert((unsigned int) (tmp->data) == 4);
    tmp = tmp->prev;
    assert((unsigned int) (tmp->data) == 3);
    tmp = tmp->prev;
    assert((unsigned int) (tmp->data) == 2);
    tmp = tmp->prev;
    assert((unsigned int) (tmp->data) == 2); // Make sure first->prev = first.
    printf("PASSED\n");

}



void test_insert_before(){
    printf("Test insert_before(): \t");
    printf("insert_before() chain. \t\t\t");
    int nbr = 0;
    list_t* fst = create_node((void*)nbr);
    list_t* lst = fst;
    list_t* tmp = fst;

    int nbr1 = 1;
    tmp = create_node((void*)nbr1);
    insert_before(lst, tmp);
    lst = tmp;

    int nbr2 = 2;
    tmp = create_node((void*)nbr2);
    insert_before(lst, tmp);
    lst = tmp;

    int nbr3 = 3;
    tmp = create_node((void*)nbr3);
    insert_before(lst, tmp);
    lst = tmp;

    int nbr4 = 4;
    tmp = create_node((void*)nbr4);
    insert_before(lst, tmp);
    lst = tmp;

    unsigned int counter = 4;

    tmp = lst;

    while(tmp->next != tmp){
        assert((unsigned int) (tmp->data) == counter--);
        tmp = tmp->next;
    }
    assert((unsigned int) (tmp->data) == counter--);
    printf("PASSED\n");
}

void test_insert_mixed(){
    printf("Test mixed inserts: \t");
    printf("mixed inserts. \t\t\t\t");
    int nbr = 0;
    list_t* fst = create_node((void*)nbr);
    list_t* lst = fst;
    list_t* tmp = fst;

    int nbr1 = 1;
    tmp = create_node((void*)nbr1);
    insert_after(lst, tmp);
    lst = tmp;

    int nbr2 = 2;
    tmp = create_node((void*)nbr2);
    insert_before(lst, tmp);
    //lst = tmp;

    int nbr3 = 3;
    tmp = create_node((void*)nbr3);
    insert_before(fst, tmp);
    fst = tmp;


    tmp = fst;
    assert((unsigned int) (tmp->data) == 3);
    tmp =tmp->next;
    assert((unsigned int) (tmp->data) == 0);
    tmp =tmp->next;
    assert((unsigned int) (tmp->data) == 2);
    tmp =tmp->next;
    assert((unsigned int) (tmp->data) == 1);
    printf("PASSED\n");
}


void test_add_last(){
    printf("Test add_last(): \t");
    printf("starting from a node. \t\t\t");

    int nbr = 0;
    int nbr1 = 1;
    int nbr2 = 2;
    int nbr3 = 3;
    int nbr4 = 4;

    list_t* fst = create_node((void*)nbr);
    list_t* lst = fst;
    list_t* tmp = fst;
    tmp = lst = fst;

    tmp = create_node((void*)nbr1);
    insert_after(fst, tmp);
    lst = tmp;

    tmp = fst;
    assert((unsigned int) (tmp->data) == 0);
    tmp =tmp->next;
    assert((unsigned int) (tmp->data) == 1);
    tmp =tmp->next;
    assert((unsigned int) (tmp->data) == 1); // Make sure end->next = end.
    printf("PASSED\n");


    printf("\t\t\tstarting from first. \t\t\t");
    tmp = lst = fst = create_node((void*)nbr);

    tmp = create_node((void*)nbr1);
    insert_after(lst, tmp);
    lst = tmp;

    tmp = create_node((void*)nbr2);
    insert_after(lst, tmp);
    lst = tmp;

    tmp = create_node((void*)nbr3);
    insert_after(lst, tmp);
    lst = tmp;

    tmp = create_node((void*)nbr4);
    add_last(fst, tmp);
    lst = tmp;

    tmp = fst;
    assert((unsigned int) (tmp->data) == 0);
    tmp =tmp->next;
    assert((unsigned int) (tmp->data) == 1);
    tmp =tmp->next;
    assert((unsigned int) (tmp->data) == 2);
    tmp =tmp->next;
    assert((unsigned int) (tmp->data) == 3);
    tmp =tmp->next;
    assert((unsigned int) (tmp->data) == 4);
    tmp =tmp->next;
    assert((unsigned int) (tmp->data) == 4); // Make sure end->next = end.
    printf("PASSED\n");


    printf("\t\t\tstarting from middle. \t\t\t");
    tmp = lst = fst = create_node((void*)nbr);
    list_t* tmp2 = tmp;


    tmp = create_node((void*)nbr1);
    insert_after(lst, tmp);
    lst = tmp;


    tmp = create_node((void*)nbr2);
    insert_after(lst, tmp);
    tmp2 = lst = tmp;


    tmp = create_node((void*)nbr3);
    insert_after(lst, tmp);
    lst = tmp;


    tmp = create_node((void*)nbr4);
    add_last(tmp2, tmp);
    lst = tmp;


    tmp = fst;
    assert((unsigned int) (tmp->data) == 0);
    tmp =tmp->next;
    assert((unsigned int) (tmp->data) == 1);
    tmp =tmp->next;
    assert((unsigned int) (tmp->data) == 2);
    tmp =tmp->next;
    assert((unsigned int) (tmp->data) == 3);
    tmp =tmp->next;
    assert((unsigned int) (tmp->data) == 4);
    tmp =tmp->next;
    assert((unsigned int) (tmp->data) == 4); // Make sure end->next = end.
    printf("PASSED\n");


    printf("\t\t\tstarting from last. \t\t\t");
    tmp2 = tmp = lst = fst = create_node((void*)nbr);

    tmp = create_node((void*)nbr1);
    insert_after(lst, tmp);
    lst = tmp;


    tmp = create_node((void*)nbr2);
    insert_after(lst, tmp);
    tmp2 = lst = tmp;


    tmp = create_node((void*)nbr3);
    insert_after(lst, tmp);
    lst = tmp;


    tmp = create_node((void*)nbr4);
    add_last(lst, tmp);
    lst = tmp;


    tmp = fst;
    assert((unsigned int) (tmp->data) == 0);
    tmp =tmp->next;
    assert((unsigned int) (tmp->data) == 1);
    tmp =tmp->next;
    assert((unsigned int) (tmp->data) == 2);
    tmp =tmp->next;
    assert((unsigned int) (tmp->data) == 3);
    tmp =tmp->next;
    assert((unsigned int) (tmp->data) == 4);
    tmp =tmp->next;
    assert((unsigned int) (tmp->data) == 4); // Make sure end->next = end.
    printf("PASSED\n");
}

int main(){

    test_insert_after();
    test_insert_before();
    test_remove_node();
    test_insert_mixed();
    test_add_last();
}
