#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <curl/curl.h>
#include <curl/easy.h>
#include <inttypes.h>
#include <stdbool.h>

// File header structure
typedef struct {
    uint64_t magic;
    uint32_t datasize;
    uint16_t n_blocks;
    uint16_t zeros;
} __attribute__((packed)) binflag_header_t;

// Data blocks structureSS
typedef struct {
    uint32_t offset;
    uint16_t cksum;
    uint16_t length;
    uint8_t payload[0];
} __attribute__((packed)) block_t;

// Flag information structure
typedef struct {
   uint16_t length;
   uint32_t offset[0];
} __attribute__((packed)) flag_t;

uint16_t cksum_compute(uint8_t* payload, uint16_t length) {
    // each 2 byte to
    // printf("In cksum function\n");
    uint16_t check = (payload[0] << 8) | (payload[1]);
    // printf("After check\n");
    for (int i = 2; i < length; i+=2) {
        check ^= (payload[i] << 8) | (payload[i+1]);
        // printf("i:%d, check:%u\n", i, check);
    }
    return check;
}
void get_challenge_file(const char* challenge_url, const char* demo_filename, int* retval) {
    // Download the challenge file using libcurl
    CURL* curl = curl_easy_init();
    retval = 0;
    if (curl) {
        FILE* output = fopen(demo_filename, "wb");
        curl_easy_setopt(curl, CURLOPT_URL, challenge_url);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, output);
        CURLcode res = curl_easy_perform(curl);
        if (res != CURLE_OK) {
            // printf("Failed to download the challenge file\n");
            *retval = 1;
            return;
        } else{
            // printf("Complete download the challenge file\n");
        }
        curl_easy_cleanup(curl);
        fclose(output);
    } else {
        // printf("Failed to initialize libcurl");
        *retval = 1;
        return;
    }
    return;
}

// int cmpfunc(const void *ptr1, const void *ptr2) {
//     const block_t *aptr = (block_t*)ptr1;
//     const block_t *bptr = (block_t*)ptr2;
//     return (aptr->offset > bptr->offset) - (aptr->offset < bptr->offset);
// }
bool to_hex(char* dest, size_t dest_len, const uint8_t* values, size_t val_len) {
    if(dest_len < (val_len*2+1)) /* check that dest is large enough */
        return false;
    *dest = '\0'; /* in case val_len==0 */
    while(val_len--) {
        /* sprintf directly to where dest points */
        sprintf(dest, "%02X", *values);
        dest += 2;
        ++values;
    }
    return true;
}

int main() {
    const char* challenge_url = "https://inp.zoolab.org/binflag/challenge?id=110611008";
    const char* demo_filename = "challenge.bin";
    int retval;
    get_challenge_file(challenge_url, demo_filename, &retval);
    if (retval != 0) {
        printf("Error: get_challenge_file\n");
        return retval;
    }
    // const char* demo_filename = "demo1.bin";
    FILE *file = fopen(demo_filename, "rb");
    if (file == NULL) {
        fprintf(stderr, "Failed to open the challenge file\n");
        return 1;
    }
    // Read and process the header
    binflag_header_t header;
    // if read binary file, return value is the same as count arg
    int ret = fread(&header, 1, 16, file);
    // printf("num byte read header :%d\n", ret);
    if (ret != 16) {
        fprintf(stderr, "Failed to read the header\n");
        fclose(file);
        return 1;
    }
    header.magic = be64toh(header.magic);
    header.datasize = be32toh(header.datasize);
    header.n_blocks = be16toh(header.n_blocks);
    header.zeros = be16toh(header.zeros);
    // printf("Magic number: %u\n", header.magic);
    // printf("Datasize: %u\n", header.datasize);
    // printf("n_blocks: %u\n", header.n_blocks);
    
    uint8_t* dictionary = (uint8_t*) malloc(sizeof(uint8_t)*header.datasize);
    int total_blocks = header.n_blocks;
    // Process data blocks
    block_t blocks[total_blocks];
    block_t *correct_blocks = (block_t*) malloc(total_blocks * sizeof(block_t));
    int correct_blocks_size = 0;
    for (int i = 0; i < total_blocks; i++) {
        int ret = fread(&blocks[i], 1, 8, file);
        // printf("num byte read block :%d\n", ret);
        if (ret != 8) {
            fprintf(stderr, "Failed to read block %d\n", i);
            fclose(file);
            return 1;
        }
        blocks[i].offset = be32toh(blocks[i].offset);
        blocks[i].cksum = be16toh(blocks[i].cksum);
        blocks[i].length = be16toh(blocks[i].length);
        // printf("offset: %u, cksum: %u, length: %u\n", blocks[i].offset, blocks[i].cksum, blocks[i].length);

        uint8_t* payload = (uint8_t*) malloc(sizeof(uint8_t) * blocks[i].length);
        // fread(block.payload);
        if (blocks[i].length > 0) {
            int ret = fread(payload, 1, blocks[i].length, file);
            // printf("After read %d bytes\n", ret);
            // printf("payload total byte read: %d\n", ret);
            if (ret != blocks[i].length) {
                fprintf(stderr, "Failed to read payload for blocks %d\n", i);
                free(payload);
                free(dictionary);
                fclose(file);
                return 1;
            }
            // printf("Print payload: \n");
            // for (int j = 0; j < blocks[i].length; j++) {
            //     printf("%" PRIu8 "\n", blocks[i].payload[j]);
            // }
            
            // uint8_t result[2];
            // result[0] = blocks[i].payload[0];
            // result[1] = blocks[i].payload[1];
            // uint8_t temp[2];
            // temp[0] = blocks[i].payload[2];
            // temp[1] = blocks[i].payload[3];
            // bxor(result, temp);
            // // for(int i = 0; i < 2; i ++) {
            // //     printf("result%d: %"PRIu8"\n", i, result[i]);
            // // }
            // int counter = 4;
            // while(counter < blocks[i].length) {
            //     uint8_t temp[2];
            //     temp[0] = blocks[i].payload[counter];
            //     temp[1] = blocks[i].payload[counter+1];
            //     bxor(result, temp);
            //     counter += 2;
            // }
        }
        // printf("Print payload: \n");
        // for (int j = 0; j < blocks[i].length; j++) {
        //     printf("%" PRIu8 "\n", payload[j]);
        // }
        // calculate cksum
        uint16_t cksum_result = cksum_compute(payload, blocks[i].length);

        // printf("result cksum: %u\n", cksum_result);
        if (cksum_result == blocks[i].cksum) {
            // printf("####cksum correct!####\n");
            for (int j = blocks[i].offset; j < blocks[i].offset+blocks[i].length; j ++) {
                dictionary[j] = payload[j-blocks[i].offset];
            }
            
        }
        free(payload);
    }
    // for (int j = 0; j < header.datasize; j ++) {
    //             printf("dict%d: %u\n", j, dictionary[j]);
    // }

    // sort block use offset 
    // qsort(correct_blocks, total_blocks, sizeof(blocks[0]), cmpfunc);
    // for (int i = 0; i < total_blocks; i ++) {
    //     printf("seq%d offset: %u\n", i, correct_blocks[i].offset);
    // }
    flag_t flag_info;
    ret = fread(&flag_info, 1, sizeof(flag_info), file);
    if (ret != 2) {
        fprintf(stderr, "Failed to read the header\n");
        fclose(file);
        return 1;
    }
    flag_info.length = be16toh(flag_info.length);
    // printf("flag info length: %d\n", flag_info.length);
    uint32_t* flags = (uint32_t*) malloc(sizeof(uint32_t) * flag_info.length);
    // fread(block.payload);
    if (flag_info.length > 0) {
        int ret = fread(flags, 4, flag_info.length, file);
        // printf("read total %d bytes\n", ret);
        if (ret != flag_info.length) {
            fprintf(stderr, "Failed to read the flags\n");
            fclose(file);
            return 1;
        }
    }
    uint8_t* ans_byte = (uint8_t*) malloc(sizeof(uint8_t)*flag_info.length*2);
    size_t ans_count = 0;
    // printf("dict%d:%u\n", 10, dictionary[10]);
    for (int i = 0; i < flag_info.length; i++) {
        flags[i] = ntohl(flags[i]);
        // printf("in for loop\n");
        ans_byte[ans_count++] = dictionary[(size_t)(flags[i])];
        ans_byte[ans_count++] = dictionary[(size_t)(flags[i]+1)];
    }
    // printf("ans byte length: %d\n", ans_count);

    char ans_hex[ans_count*2+1];/* one extra for \0 */
    if(to_hex(ans_hex, sizeof(ans_hex), ans_byte, ans_count)) {
        printf("%s", ans_hex);
    }
    fclose(file);
    return 0;
}