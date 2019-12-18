CFLAGS=-O3 -Wall -Wextra

all: h1 h2

h1:
	gcc $(CFLAGS) h1.c tiny_sha3/sha3.c -o h1

h2:
	gcc $(CFLAGS) h2.c tiny_sha3/sha3.c -o h2 

clean:
	@echo "Cleaning up..."
	rm -f h1 h2
