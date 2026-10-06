#ifndef SUBNETCALC_ENGINE_BIGUINT_H
#define SUBNETCALC_ENGINE_BIGUINT_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

typedef struct {
    uint32_t d[4]; /* d[0] is lowest 32 bits, d[3] is highest 32 bits */
} biguint128_t;

void biguint128_zero(biguint128_t *b);
void biguint128_from_u64(biguint128_t *b, uint64_t val);
void biguint128_two_pow(biguint128_t *b, int exp);
void biguint128_format_dec(const biguint128_t *b, char *buf, size_t buflen);

#endif /* SUBNETCALC_ENGINE_BIGUINT_H */
