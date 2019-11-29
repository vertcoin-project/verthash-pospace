#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <time.h>

#include "tiny_sha3/sha3.h"

#define HEADER_SIZE 80
#define SHA3_OUT_SIZE 64
#define N_INDEXES 18496
#define N_INDEX_COMPONENTS 272

const char *dat_file_name = "dat_file.dat";
const char *input_header_hex = "000000203a297b4b7685170d7644b43e5a6056234cc2414edde454a87580e1967d14c1078c13ea916117b0608732f3f65c2e03b81322efc0a62bcee77d8a9371261970a58a5a715da80e031b02560ad8";

int main() {
    FILE *datfile = fopen(dat_file_name, "rb");
    fseek(datfile, 0, SEEK_END);
    const size_t datfile_sz = ftell(datfile);

    fseek(datfile, 0, SEEK_SET);

    unsigned char *blob_bytes = malloc(datfile_sz);

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

    unsigned char p1[SHA3_OUT_SIZE];
    input_header[0] += 1;
    sha3(input_header, HEADER_SIZE, p1, SHA3_OUT_SIZE);

    uint32_t *p0_index = (uint32_t *) p0;
    uint32_t seek_index_components[N_INDEX_COMPONENTS];
    uint32_t seek_indexes[N_INDEXES];

    size_t n = 0;
    for(size_t x = 0; x < SHA3_OUT_SIZE/sizeof(uint32_t); x++) {
        uint32_t val1 = *(p0_index + x);
        for(size_t y = x; y < SHA3_OUT_SIZE/sizeof(uint32_t); y++) {
            uint32_t val2 = *(p0_index + y);
            if(x != y) seek_index_components[n] = val1 ^ val2;
            else seek_index_components[n] = val1;
            n++;
        }
    }

    p0_index = (uint32_t *) (input_header+4);
    for(size_t x = 0; x < SHA3_OUT_SIZE/sizeof(uint32_t); x++) {
        uint32_t val1 = *(p0_index + x);
        for(size_t y = x; y < SHA3_OUT_SIZE/sizeof(uint32_t); y++) {
            uint32_t val2 = *(p0_index + y);
            if(x != y) seek_index_components[n] = val1 ^ val2;
            else seek_index_components[n] = val1;
            n++;
        }
    }

    p0_index = seek_index_components;
    n = 0;
    for(size_t x = 0; x < N_INDEX_COMPONENTS/2; x++) {
        uint32_t val1 = *(p0_index + x);
        for(size_t y = N_INDEX_COMPONENTS/2; y < N_INDEX_COMPONENTS; y++) {
            uint32_t val2 = *(p0_index + y);
            seek_indexes[n] = val1 ^ val2;
            n++;
        }
    }

    end = clock();
    cpu_time_used_hashing = ((double) (end - start)) / CLOCKS_PER_SEC;

    printf("memory seeks...\n");
    start = clock();
    for(size_t i = 0; i < N_INDEXES; i++) {
        for(size_t i2 = 0; i2 < SHA3_OUT_SIZE; i2++) {
            p1[i2] ^= *(blob_bytes + ((seek_indexes[i] + i2) % datfile_sz));
        }
    }
    end = clock();
    cpu_time_used_mem = ((double) (end - start)) / CLOCKS_PER_SEC;

    for(size_t i = 0; i < SHA3_OUT_SIZE; i++) {
        printf("%02X", p1[i]);
    }
    printf("\n");

    printf("hash time: %f, mem time: %f, ratio: %f\n", cpu_time_used_hashing, cpu_time_used_mem, cpu_time_used_hashing/(cpu_time_used_hashing+cpu_time_used_mem));

    free(blob_bytes);

    return 0;
}
