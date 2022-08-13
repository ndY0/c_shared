#include "llist.h"
#include <stdlib.h>
#include <stdio.h>

void llist_set_at(LinkedList *llist, int pos, void *data) {
	
	if(!llist) {
		perror("provided list is null pointer");
		exit(-1);
	}
	int curr_pos = 0;
	LinkedList *curr = llist;
	while(curr_pos < pos) {
		if(curr->_next == NULL) {
			LinkedList *next = llist_init();
			next->_prev = curr;
			curr->_next = next;
			curr = next;
		} else {
			curr = curr->_next;
		}
		curr_pos++;
	}
	curr->data = data;
}

void llist_delete_at(LinkedList *llist, int pos) {

	if(!llist) {
		perror("provided list is null pointer");
		exit(-1);
	}
	int curr_pos = 0;
	LinkedList *curr = llist;
	while(curr_pos < pos) {
		LinkedList *next = curr->_next;
		if(next != NULL) {
			curr = next;
			curr_pos++;
		} else {
			break;
		}
	}
	if(curr_pos == pos) {
		curr->data = NULL;	
	}
}

void *llist_get_at(LinkedList *llist, int pos) {
	
	if(!llist) {
		perror("provided list is null pointer");
		exit(-1);
	}
	int curr_pos = 0;
	LinkedList *curr = llist;
	while(curr_pos < pos) {
		LinkedList *next = curr->_next;
		if(next != NULL) {
			curr = next;
		} else {
			return NULL;
		}
		curr_pos++;
	}
	return curr->data;
}

LinkedList *llist_init() {
	LinkedList * llist = malloc(sizeof(LinkedList));
	llist->_prev = NULL;
	llist->_next = NULL;
	llist->data = NULL;
	
	return llist;
}

void llist_destroy(LinkedList *llist) {
	LinkedList *curr = llist;
	while(curr) {
		LinkedList *next = curr->_next;
		free(curr);
		curr = next;
	}
}

int llist_size(LinkedList *llist) {
	LinkedList *curr = llist;
	if(curr == NULL) {
		return 0;
	}
	int count = 1;
	while(curr->_next != NULL) {
		count++;
		curr = curr->_next;
	}
	return count;
}

void llist_foreach(LinkedList *llist, void (*callback)(int pos, void *data, void *context), void *context) {
	LinkedList *curr = llist;
	int pos = 0;
	while(curr != NULL) {
		callback(pos, curr->data, context);
		curr = curr->_next;
		pos++;
	}
}

void *llist_find(LinkedList *llist, int (*callback)(int pos, void *data, void *context), void *context) {
	LinkedList *curr = llist;
	int pos = -1;
	int found = 0;
	while(curr != NULL && !found) {
		found = callback(pos, curr->data, context);
		curr = curr->_next;
		pos++;
	}
	if(found) {
		return llist_get_at(llist, pos);
	} else {
		return NULL;
	}
}

void llist_copy(LinkedList *target, LinkedList *source, void *(*duplicate)(void*)) {	
	void llist_copy_foreach(int pos, void *data, void *context) {
		llist_set_at(target, pos, duplicate(data));
	}
	llist_foreach(source, &llist_copy_foreach, NULL);
}

void llist_serialize(char *target, int (*get_node_size)(void *), char *serialize_node(void *), LinkedList *source) {
		void llist_serialize_foreach(int pos, void *data) {
			
		}
}
