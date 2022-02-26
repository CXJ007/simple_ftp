objects = main.o client.o

client : $(objects)
	gcc -o client $(objects) -lpthread

main.o : client.h

.PHONY : clean
clean:
	rm $(objects)