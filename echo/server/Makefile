CC = gcc
CFLAGS = -g -Wall
LDFLAGS = -lpthread

all: server

csapp.o: ../csapp.c ../csapp.h
	$(CC) $(CFLAGS) -c ../csapp.c

echoserver.o: echoserver.c ../csapp.h
	$(CC) $(CFLAGS) -c echoserver.c

echo.o: echo.c ../csapp.h
	$(CC) $(CFLAGS) -c echo.c

server: echoserver.o ../csapp.o echo.o
	$(CC) $(CFLAGS) echoserver.o ../csapp.o echo.o -o server $(LDFLAGS)

clean:
	rm -f *~ *.o server

