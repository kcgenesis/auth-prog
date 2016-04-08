CC=gcc
CFLAGS= -c -Wall

auth.exe: auth.o
	$(CC) auth.o -o auth.exe

auth.o: auth.c
	$(CC) $(CFLAGS) auth.c
clean: 
	rm *.o *.~ *.stackdump
