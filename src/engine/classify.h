#ifndef SUBNETCALC_ENGINE_CLASSIFY_H
#define SUBNETCALC_ENGINE_CLASSIFY_H

#include <stdint.h>

const char *classify_ipv4(uint32_t ip);
const char *classify_ipv6(const uint16_t words[8]);
const char *classify_any(const char *addr_str);

#endif /* SUBNETCALC_ENGINE_CLASSIFY_H */
