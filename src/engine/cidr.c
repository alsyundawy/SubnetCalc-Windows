#include "cidr.h"
#include "ipv4.h"
#include <stdlib.h>

static int compare_cidr(const void *a, const void *b) {
    const cidr_block_t *ca = (const cidr_block_t *)a;
    const cidr_block_t *cb = (const cidr_block_t *)b;
    if (ca->network < cb->network)
        return -1;
    if (ca->network > cb->network)
        return 1;
    if (ca->prefix < cb->prefix)
        return -1;
    if (ca->prefix > cb->prefix)
        return 1;
    return 0;
}

size_t cidr_aggregate(cidr_block_t *blocks, size_t count) {
    if (!blocks || count <= 1) {
        return count;
    }

    /* Normalize networks to prefix boundary */
    for (size_t i = 0; i < count; i++) {
        blocks[i].network = ipv4_network_address(blocks[i].network, blocks[i].prefix);
    }

    qsort(blocks, count, sizeof(cidr_block_t), compare_cidr);

    bool changed = true;
    while (changed) {
        changed = false;
        size_t write_idx = 0;

        for (size_t i = 0; i < count; i++) {
            if (i + 1 < count && blocks[i].prefix == blocks[i + 1].prefix && blocks[i].prefix > 0) {
                int p = blocks[i].prefix;
                uint32_t block_size = (p == 32) ? 1 : (1U << (unsigned int)(32 - p));
                uint32_t parent_mask = ipv4_prefix_to_mask(p - 1);
                /* Check if adjacent and aligned to supernet boundary */
                if ((blocks[i].network + block_size == blocks[i + 1].network) &&
                    ((blocks[i].network & parent_mask) == blocks[i].network)) {
                    blocks[write_idx].network = blocks[i].network;
                    blocks[write_idx].prefix = p - 1;
                    write_idx++;
                    i++; /* skip i+1 */
                    changed = true;
                    continue;
                }
            }
            blocks[write_idx++] = blocks[i];
        }
        count = write_idx;
        if (changed) {
            qsort(blocks, count, sizeof(cidr_block_t), compare_cidr);
        }
    }
    return count;
}
