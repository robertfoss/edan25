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


void test1(){
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
    printf("test1 - Passed -- basic list inserting and ordering.\n");
}

void test2(){
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

    printf("test2 - Passed -- removing elements from list and list linkage (forward using next).\n");
}

void test3(){
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

    remove_node(lst->prev->prev);

    tmp = fst;
    assert((unsigned int) (tmp->data) == 0);
    tmp = tmp->next;
    assert((unsigned int) (tmp->data) == 1);
    tmp = tmp->next;
    assert((unsigned int) (tmp->data) == 3);
    tmp = tmp->next;
    assert((unsigned int) (tmp->data) == 4);
    tmp = tmp->next;

    printf("test3 - Passed -- removing elements from list and list linkage (backward using prev).\n");
}


void test4(){
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
    
    remove_node(lst->prev);

    assert((unsigned int) (fst->next->next->data) == 3);
    assert((unsigned int) (fst->next->prev->data) == 0);

    assert((unsigned int) (lst->next->data) == 3);
    assert((unsigned int) (lst->prev->data) == 1);

    printf("test4 - Passed -- removing an element from the list.\n");
}


void test5(){
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
    
    tmp = tmp->prev;

    remove_node(lst);

    assert((unsigned int) (tmp->prev->data) == 1);
    assert((unsigned int) (tmp->next->data) == 2);

    printf("test5 - Passed -- removing the last element from the list.\n");
}

int main(){

    test1(); // Test basic list inserting and ordering.
    test2(); // Test removing elements from list (forward using next).
    test3(); // Test removing elements from list (backward using prev).
    test4(); // Test removing an element from the list.
    test5(); // Test removing the last element from the list.
}
