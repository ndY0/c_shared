CFLAGS=-Wall -g

clean:
	rm -rf *.o
test.o:
	gcc -lpthread -c test.c
test_queue.o:
	gcc -c test_queue.c
llist.o:
	gcc -Wno-incompatible-pointer-types -c ./lib/llist.c
event-emitter.o:
	gcc -Wno-incompatible-pointer-types -Wno-pointer-to-int-cast -c ./lib/event-emitter.c
queue.o:
	gcc -c ./lib/queue.c
pqueue.o:
	gcc -static -Wno-incompatible-pointer-types -c ./lib/pqueue.c
output: pqueue.o queue.o event-emitter.o llist.o test.o test_queue.o
	gcc -o test llist.o event-emitter.o test.o -D_REENTRANT
	gcc -o test_queue llist.o event-emitter.o queue.o pqueue.o test_pueue.o -D_REENTRANT
all: output clean
