#include <stdint.h>

void init_xorshift32(uint32_t* state) {
    *state  = 0xA634716A;
}

uint32_t xorshift32(uint32_t* state) {
    uint32_t x;

    x = *state;
    x = x ^ (x << 13);
    x = x ^ (x >> 17);
    x = x ^ (x << 5);
    *state = x;
    return x;
}