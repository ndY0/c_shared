#include "llist.h"
#include <stdio.h>
#include <stdlib.h>

typedef struct Test {
	int data;
} Test;

void *Test_duplicate(void *source) {
	Test *dupl = malloc(sizeof(Test));
	Test *casted_source = (Test*)source;
	if(casted_source != NULL) {
		dupl->data = casted_source->data;
		return dupl;
	} else {
		return NULL;
	}
}

int main(int argc, char **argv) {
	
	LinkedList *list = llist_init();
	int i = 0;
	
	for(i = 0; i < 100; i++) {
		Test *test = malloc(sizeof(Test));
		test->data = i;
		llist_set_at(list, i, test);
	}
	
	llist_delete_at(list, 50);
	
	for(i = 0; i < 100; i++) {
		Test *test_retrieved = llist_get_at(list, i);
		if(&test_retrieved->data != NULL) {
			printf("%d\n", test_retrieved->data);
		}
	}
	printf("size : %d\n", llist_size(list));
	
	LinkedList *duplicate = llist_init();
	llist_copy(duplicate, list, &Test_duplicate);
	
	for(i = 0; i < 100; i++) {
		Test *test_retrieved = llist_get_at(duplicate, i);
		if(&test_retrieved->data != NULL) {
			printf("%d\n", test_retrieved->data);
		}
	}
	
	llist_destroy(list);
	
	return 0;
}
