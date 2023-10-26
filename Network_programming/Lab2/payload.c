#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <curl/curl.h>
#include <curl/easy.h>
#include <inttypes.h>
// Data blocks structureSS
typedef struct {
    uint32_t offset;
    uint16_t cksum;
    uint16_t length;
    uint8_t payload[0];
} __attribute__((packed)) block_t;
void foo(char *buf, int count) {
    for(int i = 0; i < count; ++i)
        buf[i] = i;
}
int main() {
    char arr[10] = {0};
    foo(arr, 10);
    // No need to deallocate because we allocated 
    // arr with automatic storage duration.
    // If we had dynamically allocated it
    // (i.e. malloc or some variant) then we 
    // would need to call free(arr)
    for (int i = 0; i < 10; i ++) {
        printf("arr %d: %d\n", i , arr[i]);
    }
    // int arr1[] = {1, 2, 3 ,4};
    // int arr2[] = {3, 4, 5, 6};
    // arr1 = arr2;
}
