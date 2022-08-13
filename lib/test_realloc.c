#include <stdlib.h>
#include <string.h>
#include <stdio.h>

int main(int argc, char **argv) {

	char *test = "je suis un string";
	printf("size: %ld", strlen(test) * sizeof(char));
	return 0;
}
