#ifndef SUBNETCALC_ENGINE_IPV6_H
#define SUBNETCALC_ENGINE_IPV6_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define IPV6_COMPACT_BUFFER_SIZE 48
#define IPV6_EXPANDED_BUFFER_SIZE 40
#define IPV6_ARPA_BUFFER_SIZE 76

typedef struct {
    uint16_t w[8]; /* 8 16-bit words in host order */
} ipv6_addr_t;

bool ipv6_parse(const char *str, ipv6_addr_t *out_addr);
void ipv6_format_compact(const ipv6_addr_t *addr, char *buf, size_t buflen);
void ipv6_format_expanded(const ipv6_addr_t *addr, char *buf, size_t buflen);
void ipv6_apply_mask(const ipv6_addr_t *addr, int prefix, ipv6_addr_t *out_net);
void ipv6_range(const ipv6_addr_t *addr, int prefix, ipv6_addr_t *out_start, ipv6_addr_t *out_end);
void ipv6_arpa(const ipv6_addr_t *addr, char *buf, size_t buflen);
void ipv6_total_hosts(int prefix, char *buf, size_t buflen);
bool ipv6_is_ipv4_mapped(const ipv6_addr_t *addr, uint32_t *out_ipv4);
bool ipv6_is_6to4(const ipv6_addr_t *addr, uint32_t *out_ipv4);
void ipv6_from_ipv4_mapped(uint32_t ipv4, ipv6_addr_t *out_addr);
void ipv6_from_ipv4_6to4(uint32_t ipv4, ipv6_addr_t *out_addr);

#endif /* SUBNETCALC_ENGINE_IPV6_H */
