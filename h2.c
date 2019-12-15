#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

#include "tiny_sha3/sha3.h"

#define HEADER_SIZE 80
#define SHA3_OUT_SIZE 64
#define N_INDEXES 272
#define CHUNK_SIZE 4096
#define ALIGN 16

const char *dat_file_name = "verthash.dat";
const char *input_header_hex = "000000203a297b4b7685170d7644b43e5a6056234cc2414edde454a87580e1967d14c1078c13ea916117b0608732f3f65c2e03b81322efc0a62bcee77d8a9371261970a58a5a715da80e031b02560ad8";

int main() {
    FILE *datfile = fopen(dat_file_name, "rb");
    fseek(datfile, 0, SEEK_END);
    const size_t datfile_sz = ftell(datfile);

    fseek(datfile, 0, SEEK_SET);

    unsigned char *blob_bytes = malloc(datfile_sz);
    uint32_t *blob_index = (uint32_t *) blob_bytes;

    printf("reading data file...\n");

    fread(blob_bytes, 1, datfile_sz, datfile);
    fclose(datfile);

    unsigned char input_header[HEADER_SIZE];

    char *pos = (char *) input_header_hex;
    for(size_t count = 0; count < HEADER_SIZE; count++) {
        sscanf(pos, "%2hhx", &input_header[count]);
        pos += 2;
    }

    printf("sha3 hashes...\n");

    clock_t start, end;
    double cpu_time_used_hashing, cpu_time_used_mem;

    start = clock();

    unsigned char p0[SHA3_OUT_SIZE];
    sha3(input_header, HEADER_SIZE, p0, SHA3_OUT_SIZE);

    uint32_t *p0_index = (uint32_t *) p0;
    uint32_t seek_indexes[N_INDEXES+1];

    uint32_t val1, val2;
    size_t n = 0;

    for(size_t x = 0; x < SHA3_OUT_SIZE/sizeof(uint32_t); x++) {
        val1 = *(p0_index + x);
        for(size_t y = x; y < SHA3_OUT_SIZE/sizeof(uint32_t); y++) {
            val2 = *(p0_index + y);
            if(x != y) seek_indexes[n] = val1 ^ val2;
            else seek_indexes[n] = val1;
            n++;
        }
    }

    p0_index = (uint32_t *) (input_header+4);
    for(size_t x = 0; x < SHA3_OUT_SIZE/sizeof(uint32_t); x++) {
        val1 = *(p0_index + x);
        for(size_t y = x; y < SHA3_OUT_SIZE/sizeof(uint32_t); y++) {
            val2 = *(p0_index + y);
            if(x != y) seek_indexes[n] = val1 ^ val2;
            else seek_indexes[n] = val1;
            n++;
        }
    }

    unsigned char p1[CHUNK_SIZE];
    uint32_t *p1_index = (uint32_t *) p1;

    input_header[0] += 1;
    sha3(input_header, HEADER_SIZE, p1, SHA3_OUT_SIZE);

    for(size_t i = 1; i < 64; i++) {
	memcpy(p1+i*64, p1+i, 64-i);
	memcpy(p1+i*64+(64-i), p1, i);
    }

    end = clock();
    cpu_time_used_hashing = ((double) (end - start)) / CLOCKS_PER_SEC;

    printf("memory seeks...\n");
    start = clock();

    for(size_t i = 0; i < N_INDEXES; i++) {
	val1 = (seek_indexes[i] % ((datfile_sz-CHUNK_SIZE+16)/ALIGN)) * ALIGN/sizeof(uint32_t);
        for(size_t j = 0; j < CHUNK_SIZE/sizeof(uint32_t); j++) {
            *(p1_index + j) = ( *(p1_index + j) ^ *(blob_index + val1 + j) ) * 0x1000193;
	    seek_indexes[i+1] ^= *(p1_index + j);
        }
    }

    end = clock();
    cpu_time_used_mem = ((double) (end - start)) / CLOCKS_PER_SEC;

    unsigned char hash[32];
    uint32_t *hash_index = (uint32_t *) hash;
    memset(hash, 0, 32);

    for(size_t i = 0; i < 8; i++) {
        for(size_t j = 0; j < 128; j++) {
	    *(hash_index+i) ^= *(p1_index + i*128 + j);
	}
    }

    printf("hash: ");
    for(size_t i = 0; i < 32; i++) {
        printf("%02X", hash[i]);
    }
    printf("\n");

    printf("hash time: %f, mem time: %f, ratio: %f\n", cpu_time_used_hashing, cpu_time_used_mem, cpu_time_used_hashing/(cpu_time_used_hashing+cpu_time_used_mem));

    free(blob_bytes);

    return 0;
}
