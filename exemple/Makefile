MY_CFLAGS = -I..
MY_LDFLAGS = -L.. -lmysac-static

all: client client_iofunc client_stmt client_bigmem

dump_response.o: dump_response.c
	$(CC) -c -o dump_response.o dump_response.c $(CFLAGS) $(MY_CFLAGS)

client: client.o
	$(CC) -o client client.o $(LDFLAGS) $(MY_LDFLAGS)

client.o: client.c
	$(CC) -c -o client.o client.c $(CFLAGS) $(MY_CFLAGS)

client_iofunc: client_iofunc.o
	$(CC) -o client_iofunc client_iofunc.o $(LDFLAGS) $(MY_LDFLAGS)

client_iofunc.o: client_iofunc.c
	$(CC) -c -o client_iofunc.o client_iofunc.c $(CFLAGS) $(MY_CFLAGS)

client_stmt: client_stmt.o
	$(CC) -o client_stmt client_stmt.o $(LDFLAGS) $(MY_LDFLAGS)

client_stmt.o: client_stmt.c
	$(CC) -c -o client_stmt.o client_stmt.c $(CFLAGS) $(MY_CFLAGS)

client_bigmem: client_bigmem.o dump_response.o
	$(CC) -o client_bigmem client_bigmem.o dump_response.o $(LDFLAGS) $(MY_LDFLAGS)

client_bigmem.o: client_bigmem.c
	$(CC) -c -o client_bigmem.o client_bigmem.c $(CFLAGS) $(MY_CFLAGS)

clean:
	rm -f *.o client client_iofunc client_stmt
