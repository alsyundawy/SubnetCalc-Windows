#include "split.h"
#include "ipv4.h"
#include <stdlib.h>
#include <string.h>

static uint32_t safe_block_size(int prefix) {
    if (prefix >= 32)
        return 1;
    if (prefix <= 0)
        return 0xFFFFFFFFU;
    return 1U << (unsigned int)(32 - prefix);
}

size_t flsm_split(uint32_t base_ip, int base_prefix, int target_prefix, subnet_slice_t *out_slices,
                  size_t max_slices) {
    if (base_prefix > target_prefix || target_prefix > 32 || !out_slices || max_slices == 0) {
        return 0;
    }
    uint32_t net = ipv4_network_address(base_ip, base_prefix);
    size_t count = (size_t)1ULL << (unsigned int)(target_prefix - base_prefix);
    if (count > max_slices) {
        count = max_slices;
    }
    if (count > MAX_FLSM_ROWS) {
        count = MAX_FLSM_ROWS;
    }

    uint32_t step = safe_block_size(target_prefix);
    uint32_t mask = ipv4_prefix_to_mask(target_prefix);

    for (size_t i = 0; i < count; i++) {
        uint32_t cur_net = net + (uint32_t)(i * step);
        uint32_t cur_bcast = cur_net + step - 1;
        uint32_t u_start = 0, u_end = 0, u_count = 0;
        ipv4_usable_range(cur_net, target_prefix, &u_start, &u_end, &u_count);

        out_slices[i].network = cur_net;
        out_slices[i].broadcast = cur_bcast;
        out_slices[i].usable_start = u_start;
        out_slices[i].usable_end = u_end;
        out_slices[i].mask = mask;
        out_slices[i].prefix = target_prefix;
        out_slices[i].host_count = u_count;
    }
    return count;
}

static int compare_vlsm_desc(const void *a, const void *b) {
    const vlsm_req_t *ra = (const vlsm_req_t *)a;
    const vlsm_req_t *rb = (const vlsm_req_t *)b;
    if (ra->needed_hosts < rb->needed_hosts)
        return 1;
    if (ra->needed_hosts > rb->needed_hosts)
        return -1;
    return 0;
}

static int best_prefix_for_hosts(uint32_t needed) {
    if (needed <= 1)
        return 32;
    if (needed == 2)
        return 31;
    for (int p = 30; p > 0; p--) {
        uint32_t cap = (1U << (unsigned int)(32 - p)) - 2;
        if (cap >= needed) {
            return p;
        }
    }
    return 0;
}

bool vlsm_calculate(uint32_t base_ip, int base_prefix, vlsm_req_t *reqs, size_t req_count,
                    vlsm_summary_t *out_summary) {
    if (!reqs || req_count == 0 || req_count > MAX_VLSM_ROWS) {
        return false;
    }
    if (base_prefix < 0 || base_prefix > 32) {
        return false;
    }

    /* Sort descending */
    qsort(reqs, req_count, sizeof(vlsm_req_t), compare_vlsm_desc);

    uint32_t cur_ip = ipv4_network_address(base_ip, base_prefix);
    uint32_t base_bcast = ipv4_broadcast_address(base_ip, base_prefix);

    uint32_t total_req = 0;
    uint32_t total_alloc = 0;
    bool fits = true;

    for (size_t i = 0; i < req_count; i++) {
        int pref = best_prefix_for_hosts(reqs[i].needed_hosts);
        uint32_t block_size = safe_block_size(pref);

        /* Align cur_ip to block_size boundary */
        if (block_size > 1 && (cur_ip % block_size != 0)) {
            cur_ip += block_size - (cur_ip % block_size);
        }

        uint32_t net = cur_ip;
        uint32_t bcast = net + block_size - 1;

        if (bcast > base_bcast || net > bcast) {
            fits = false;
        }

        uint32_t u_start = 0, u_end = 0, u_count = 0;
        ipv4_usable_range(net, pref, &u_start, &u_end, &u_count);

        reqs[i].allocated_prefix = pref;
        reqs[i].network = net;
        reqs[i].broadcast = bcast;
        reqs[i].usable_start = u_start;
        reqs[i].usable_end = u_end;
        reqs[i].allocated_hosts = u_count;
        reqs[i].wasted_hosts =
            (u_count >= reqs[i].needed_hosts) ? (u_count - reqs[i].needed_hosts) : 0;

        total_req += reqs[i].needed_hosts;
        total_alloc += u_count;

        cur_ip = bcast + 1;
    }

    if (out_summary) {
        out_summary->total_requested = total_req;
        out_summary->total_allocated = total_alloc;
        out_summary->total_wasted = (total_alloc >= total_req) ? (total_alloc - total_req) : 0;
        out_summary->efficiency_percent =
            (total_alloc > 0) ? (((double)total_req / (double)total_alloc) * 100.0) : 0.0;
        out_summary->fits = fits;
    }

    return fits;
}
