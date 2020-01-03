#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <time.h>

#include "tiny_sha3/sha3.h"

#define HEADER_SIZE 80
#define PREV_COINB_SIZE 64
#define P0_SIZE 64
#define HASH_OUT_SIZE 32
#define N_P0_COMPONENTS ((P0_SIZE/sizeof(uint32_t))*((P0_SIZE/sizeof(uint32_t))-1))/2
#define N_PREV_COINB_COMPONENTS ((PREV_COINB_SIZE/sizeof(uint32_t))*((PREV_COINB_SIZE/sizeof(uint32_t))-1))/2
#define N_INDEXES N_P0_COMPONENTS*N_PREV_COINB_COMPONENTS
#define BYTE_ALIGNMENT 16

const char* dat_file_name = "verthash.dat";
const char* input_header_hex = "000000203a297b4b7685170d7644b43e5a6056234cc2414edde454a87580e1967d14c1078c13ea916117b0608732f3f65c2e03b81322efc0a62bcee77d8a9371261970a58a5a715da80e031b02560ad8";

inline uint32_t fnv1a(const uint32_t a, const uint32_t b) {
    return (a ^ b) * 0x1000193;
}

int main() {
    FILE* datfile = fopen(dat_file_name, "rb");
    fseek(datfile, 0, SEEK_END);
    const size_t datfile_sz = ftell(datfile);

    fseek(datfile, 0, SEEK_SET);
    
    printf("allocating buffer...\n");

    unsigned char* blob_bytes = malloc(datfile_sz);

    printf("reading data file...\n");

    const size_t bytes_read = fread(blob_bytes, 1, datfile_sz, datfile);
    if(bytes_read != datfile_sz) {
        return -1;
    }

    fclose(datfile);

    unsigned char input_header[HEADER_SIZE];
    for(size_t count = 0; count < HEADER_SIZE; count++) {
        sscanf(input_header_hex + count*2, "%2hhx", &input_header[count]);
    }

    printf("SHA3 hashes...\n");

    clock_t start, end;
    double cpu_time_used_hashing, cpu_time_used_mem;

    start = clock();

    unsigned char p0[P0_SIZE];
    sha3(&input_header[0], HEADER_SIZE, &p0[0], P0_SIZE);

    unsigned char p1[HASH_OUT_SIZE];
    input_header[0] += 1;
    sha3(&input_header[0], HEADER_SIZE, &p1[0], HASH_OUT_SIZE);

    uint32_t* p0_index = (uint32_t*)p0;
    uint32_t seek_index_components[N_P0_COMPONENTS+N_PREV_COINB_COMPONENTS];
    uint32_t seek_indexes[N_INDEXES];

    size_t n = 0;
    for(size_t x = 0; x < P0_SIZE/sizeof(uint32_t); x++) {
        const uint32_t val1 = *(p0_index + x);
        for(size_t y = x+1; y < P0_SIZE/sizeof(uint32_t); y++) {
            const uint32_t val2 = *(p0_index + y);
            seek_index_components[n] = fnv1a(val1, val2);
            n++;
        }
    }

    p0_index = (uint32_t*)(input_header + 4);
    for(size_t x = 0; x < PREV_COINB_SIZE/sizeof(uint32_t); x++) {
        const uint32_t val1 = *(p0_index + x);
        for(size_t y = x+1; y < PREV_COINB_SIZE/sizeof(uint32_t); y++) {
            const uint32_t val2 = *(p0_index + y);
            seek_index_components[n] = fnv1a(val1, val2);
            n++;
        }
    }

    p0_index = &seek_index_components[0];
    n = 0;
    for(size_t x = 0; x < N_P0_COMPONENTS; x++) {
        const uint32_t val1 = *(p0_index + x);
        for(size_t y = N_P0_COMPONENTS; y < N_P0_COMPONENTS+N_PREV_COINB_COMPONENTS; y++) {
            const uint32_t val2 = *(p0_index + y);
            seek_indexes[n] = fnv1a(val1, val2);
            n++;
        }
    }

    end = clock();
    cpu_time_used_hashing = ((double) (end - start)) / CLOCKS_PER_SEC;

    printf("memory seeks...\n");
    start = clock();
    uint32_t* p1_32 = (uint32_t*)p1;
    uint32_t* blob_bytes_32 = (uint32_t*)blob_bytes;
    uint32_t value_accumulator = 0;
    const uint32_t mdiv = ((datfile_sz - HASH_OUT_SIZE)/BYTE_ALIGNMENT) + 1;
    for(size_t i = 0; i < N_INDEXES; i++) {
        const uint32_t offset = (fnv1a(seek_indexes[i], value_accumulator) % mdiv) * BYTE_ALIGNMENT/sizeof(uint32_t); 
        for(size_t i2 = 0; i2 < HASH_OUT_SIZE/sizeof(uint32_t); i2++) {
            const uint32_t value = *(blob_bytes_32 + offset + i2);
            uint32_t* p1_ptr = p1_32 + i2;
            *p1_ptr = fnv1a(*p1_ptr, value);

            value_accumulator = fnv1a(value_accumulator, value);
        }
    }

    end = clock();
    cpu_time_used_mem = ((double) (end - start)) / CLOCKS_PER_SEC;

    for(size_t i = 0; i < HASH_OUT_SIZE; i++) {
        printf("%02X", p1[i]);
    }
    printf("\n");

    printf("hash time: %f, mem time: %f, ratio: %f\n", cpu_time_used_hashing, cpu_time_used_mem, cpu_time_used_hashing/(cpu_time_used_hashing+cpu_time_used_mem));

    free(blob_bytes);

    return 0;
}
