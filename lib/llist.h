typedef struct LinkedList {
	struct LinkedList *_prev;
	struct LinkedList *_next;
	void *data;
} LinkedList;
/*
 * llist set a pointer at given position in linked list.
 * be aware that if data is pointed at at given position 
 * and allocated, it won't be freed causing memory leek.
 */
void llist_set_at(LinkedList *llist, int pos, void *data);
void llist_delete_at(LinkedList *llist, int pos);
void *llist_get_at(LinkedList *llist, int pos);
LinkedList *llist_init();
void llist_destroy(LinkedList *llist);
int llist_size(LinkedList *llist);
void llist_foreach(LinkedList *llist, void (*callback)(int pos, void *data, void *context), void *context);
void *llist_find(LinkedList *llist, int (*callback)(int pos, void *data, void *context), void *context);
void llist_copy(LinkedList *target, LinkedList *source, void *(*duplicate)(void*));

