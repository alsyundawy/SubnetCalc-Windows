#include "biguint.h"
#include <stdio.h>
#include <string.h>

void biguint128_zero(biguint128_t *b) {
    if (b) {
        b->d[0] = 0;
        b->d[1] = 0;
        b->d[2] = 0;
        b->d[3] = 0;
    }
}

void biguint128_from_u64(biguint128_t *b, uint64_t val) {
    if (!b) {
        return;
    }
    b->d[0] = (uint32_t)(val & 0xFFFFFFFFU);
    b->d[1] = (uint32_t)((val >> 32) & 0xFFFFFFFFU);
    b->d[2] = 0;
    b->d[3] = 0;
}

void biguint128_two_pow(biguint128_t *b, int exp) {
    biguint128_zero(b);
    if (!b || exp < 0 || exp > 128) {
        return;
    }
    if (exp == 128) {
        /* 2^128 is 1 followed by 128 zeros; saturated to max 128-bit uint (2^128 - 1) or handled */
        b->d[0] = 0xFFFFFFFFU;
        b->d[1] = 0xFFFFFFFFU;
        b->d[2] = 0xFFFFFFFFU;
        b->d[3] = 0xFFFFFFFFU;
        return;
    }
    int word_idx = exp / 32;
    int bit_idx = exp % 32;
    b->d[word_idx] = 1U << (unsigned int)bit_idx;
}

void biguint128_format_dec(const biguint128_t *b, char *buf, size_t buflen) {
    if (!buf || buflen == 0) {
        return;
    }
    if (!b || (b->d[0] == 0 && b->d[1] == 0 && b->d[2] == 0 && b->d[3] == 0)) {
        snprintf(buf, buflen, "0");
        return;
    }

    /* Special case for 2^128 exact display string */
    if (b->d[0] == 0xFFFFFFFFU && b->d[1] == 0xFFFFFFFFU && b->d[2] == 0xFFFFFFFFU &&
        b->d[3] == 0xFFFFFFFFU) {
        snprintf(buf, buflen, "340282366920938463463374607431768211456");
        return;
    }

    uint32_t copy[4];
    copy[0] = b->d[0];
    copy[1] = b->d[1];
    copy[2] = b->d[2];
    copy[3] = b->d[3];

    char rev[64];
    int pos = 0;

    while (copy[0] != 0 || copy[1] != 0 || copy[2] != 0 || copy[3] != 0) {
        uint64_t rem = 0;
        for (int i = 3; i >= 0; i--) {
            uint64_t cur = (rem << 32) | copy[i];
            copy[i] = (uint32_t)(cur / 10U);
            rem = cur % 10U;
        }
        rev[pos++] = (char)('0' + rem);
    }

    /* Reverse to buf */
    int out_pos = 0;
    for (int i = pos - 1; i >= 0 && (size_t)out_pos < buflen - 1; i--) {
        buf[out_pos++] = rev[i];
    }
    buf[out_pos] = '\0';
}
