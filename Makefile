CFLAGS=-Wall -g

clean:
	rm -rf *.o
test.o: test.c
	gcc -lpthread -c test.c
test_queue.o: test_queue.c
	gcc -lpthread -c test_queue.c
llist.o:
	gcc -c ./lib/llist.c
event-emitter.o:
	gcc -Wno-incompatible-pointer-types -Wno-pointer-to-int-cast -c ./lib/event-emitter.c
queue.o:
	gcc -c ./lib/queue.c
pqueue.o:
	gcc -static -Wno-incompatible-pointer-types -c ./lib/pqueue.c
output: pqueue.o queue.o event-emitter.o llist.o test.o test_queue.o
	gcc -o test test.o llist.o event-emitter.o -D_REENTRANT
	gcc -o test_queue test_queue.o llist.o event-emitter.o queue.o pqueue.o -D_REENTRANT
all: output clean
