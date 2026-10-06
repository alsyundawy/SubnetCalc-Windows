#ifndef SUBNETCALC_ENGINE_CIDR_H
#define SUBNETCALC_ENGINE_CIDR_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

typedef struct {
    uint32_t network;
    int prefix;
} cidr_block_t;

size_t cidr_aggregate(cidr_block_t *blocks, size_t count);

#endif /* SUBNETCALC_ENGINE_CIDR_H */
