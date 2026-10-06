#ifndef SUBNETCALC_ENGINE_ULA_H
#define SUBNETCALC_ENGINE_ULA_H

#include "ipv6.h"
#include <stdbool.h>

bool ula_generate(ipv6_addr_t *out_prefix48, ipv6_addr_t *out_subnet64, char *out_prefix48_str,
                  size_t prefix_len, char *out_subnet64_str, size_t subnet_len);

#endif /* SUBNETCALC_ENGINE_ULA_H */
