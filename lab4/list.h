struct list_t;
typdef struct{
	list_t* next;
	list_t* prev;
	void* data;
} list_t;

/** Create an initialize a new list node */
list_t* create_node(void* data);

/** Remove and de-link list node from list and returns contained data */
void* remove_node(list_t* list);

/** Inject new_node into a list after target */
void insert_after(list_t* target, list_t* new_node);

/** Insert an element last in the list */
void add_last(list_t* list, list_t* new_node);
