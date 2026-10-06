#ifndef SUBNETCALC_ENGINE_IPV4_H
#define SUBNETCALC_ENGINE_IPV4_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define IPV4_STR_BUFFER_SIZE 36
#define IPV4_BITMAP_BUFFER_SIZE 40

bool ipv4_parse(const char *str, uint32_t *out_ip);
void ipv4_format(uint32_t ip, char *buf, size_t buflen);
bool ipv4_mask_is_contiguous(uint32_t mask, int *out_prefix);
uint32_t ipv4_prefix_to_mask(int prefix);
int ipv4_mask_to_prefix(uint32_t mask);
uint32_t ipv4_network_address(uint32_t ip, int prefix);
uint32_t ipv4_broadcast_address(uint32_t ip, int prefix);
uint32_t ipv4_wildcard_mask(int prefix);
char ipv4_net_class(uint32_t ip);
int ipv4_class_bits(char net_class);
int ipv4_subnet_bits(char net_class, int prefix);
int ipv4_host_bits(int prefix);
uint32_t ipv4_max_hosts(int prefix);
uint64_t ipv4_max_subnets(int subnet_bits);
uint64_t ipv4_max_cidr_subnets(int prefix);
uint64_t ipv4_max_cidr_supernet(char net_class, int prefix);
void ipv4_usable_range(uint32_t ip, int prefix, uint32_t *out_start, uint32_t *out_end,
                       uint32_t *out_count);
void ipv4_binarize(uint32_t ip, bool dotted, char *buf, size_t buflen);
void ipv4_hexarize(uint32_t ip, bool dotted, char *buf, size_t buflen);
void ipv4_bitmap(char net_class, int prefix, bool dotted, char *buf, size_t buflen);

#endif /* SUBNETCALC_ENGINE_IPV4_H */
