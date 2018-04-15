all:
	gcc -c safe_queue.c -o safe_queue.o -pthread
	gcc safe_queue.o test.c -pthread -o test
	rm safe_queue.o
install:
	gcc -c safe_queue.c -o safe_queue.o -pthread
	sudo mv safe_queue.o /usr/lib/libsafequeue.so
	sudo cp safe_queue.h /usr/include
remove:
	sudo rm /usr/lib/libsafequeue.so /usr/include/safe_queue.h
clean:
	rm test
