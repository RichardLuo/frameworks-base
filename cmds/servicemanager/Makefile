all: servicemanager 

servicemanager: binder.o service_manager.o
	gcc -m32 -o $@ $^

clean:
	rm -f *.o servicemanager

%.o: %.c
	gcc -I.. $(CFLAGS) -c -m32 -o $@ $<
