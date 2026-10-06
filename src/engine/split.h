#ifndef SUBNETCALC_ENGINE_SPLIT_H
#define SUBNETCALC_ENGINE_SPLIT_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define MAX_FLSM_ROWS 4096
#define MAX_VLSM_ROWS 4096

typedef struct {
    uint32_t network;
    uint32_t broadcast;
    uint32_t usable_start;
    uint32_t usable_end;
    uint32_t mask;
    int prefix;
    uint32_t host_count;
} subnet_slice_t;

typedef struct {
    char name[64];
    uint32_t needed_hosts;
    int allocated_prefix;
    uint32_t network;
    uint32_t broadcast;
    uint32_t usable_start;
    uint32_t usable_end;
    uint32_t allocated_hosts;
    uint32_t wasted_hosts;
} vlsm_req_t;

typedef struct {
    uint32_t total_requested;
    uint32_t total_allocated;
    uint32_t total_wasted;
    double efficiency_percent;
    bool fits;
} vlsm_summary_t;

size_t flsm_split(uint32_t base_ip, int base_prefix, int target_prefix, subnet_slice_t *out_slices,
                  size_t max_slices);

bool vlsm_calculate(uint32_t base_ip, int base_prefix, vlsm_req_t *reqs, size_t req_count,
                    vlsm_summary_t *out_summary);

#endif /* SUBNETCALC_ENGINE_SPLIT_H */
