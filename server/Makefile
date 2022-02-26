objects = main.o server.o  servershell.o

server : $(objects)
	gcc -o server $(objects) -lpthread

main.o : server.h servershell.h

.PHONY : clean
clean:
	rm server $(objects)
