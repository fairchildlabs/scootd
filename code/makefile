CC=gcc
CFLAGS=-I.

scootd: scootd.o scootd_util.o 
	$(CC) -o scootd scootd.o scootd_util.o

scootdtest: scootdtest.o scootd_util.o
	$(CC) -o scootdtest scootdtest.o scootd_util.o
	
clean:
	rm -f *.o
	
