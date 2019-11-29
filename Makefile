all: h1 h2

h1:
	gcc -O3 h1.c tiny_sha3/sha3.c -o h1

h2:
	gcc -O3 h2.c BLAKE2/sse/blake2b.c -o h2 

clean:
	@echo "Cleaning up..."
	rm -f h1 h2
