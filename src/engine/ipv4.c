#include "ipv4.h"
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

bool ipv4_parse(const char *str, uint32_t *out_ip) {
    if (!str || !out_ip) {
        return false;
    }
    while (isspace((unsigned char)*str)) {
        str++;
    }

    unsigned int octets[4] = {0};
    int count = 0;
    const char *p = str;

    while (*p && count < 4) {
        if (!isdigit((unsigned char)*p)) {
            return false;
        }
        char *endptr = NULL;
        unsigned long val = strtoul(p, &endptr, 10);
        if (val > 255 || endptr == p) {
            return false;
        }
        octets[count++] = (unsigned int)val;
        p = endptr;
        if (*p == '.') {
            p++;
            if (!isdigit((unsigned char)*p)) {
                return false;
            }
        } else if (*p == '\0' || *p == '/' || isspace((unsigned char)*p)) {
            break;
        } else {
            return false;
        }
    }

    if (count != 4) {
        return false;
    }
    while (isspace((unsigned char)*p)) {
        p++;
    }
    if (*p != 0 && *p != '/') {
        return false;
    }

    *out_ip = ((uint32_t)octets[0] << 24) | ((uint32_t)octets[1] << 16) |
              ((uint32_t)octets[2] << 8) | (uint32_t)octets[3];
    return true;
}

void ipv4_format(uint32_t ip, char *buf, size_t buflen) {
    if (!buf || buflen == 0) {
        return;
    }
    snprintf(buf, buflen, "%u.%u.%u.%u", (ip >> 24) & 0xFF, (ip >> 16) & 0xFF, (ip >> 8) & 0xFF,
             ip & 0xFF);
}

bool ipv4_mask_is_contiguous(uint32_t mask, int *out_prefix) {
    uint32_t inv = ~mask;
    if ((inv & (inv + 1)) != 0) {
        return false;
    }
    int prefix = 0;
    for (int i = 31; i >= 0; i--) {
        if ((mask >> (unsigned int)i) & 1U) {
            prefix++;
        } else {
            break;
        }
    }
    if (out_prefix) {
        *out_prefix = prefix;
    }
    return true;
}

uint32_t ipv4_prefix_to_mask(int prefix) {
    if (prefix <= 0) {
        return 0;
    }
    if (prefix >= 32) {
        return 0xFFFFFFFFU;
    }
    return 0xFFFFFFFFU << (unsigned int)(32 - prefix);
}

int ipv4_mask_to_prefix(uint32_t mask) {
    int prefix = 0;
    if (ipv4_mask_is_contiguous(mask, &prefix)) {
        return prefix;
    }
    prefix = 0;
    for (int i = 0; i < 32; i++) {
        if ((mask >> (unsigned int)i) & 1U) {
            prefix++;
        }
    }
    return prefix;
}

uint32_t ipv4_network_address(uint32_t ip, int prefix) {
    uint32_t mask = ipv4_prefix_to_mask(prefix);
    return ip & mask;
}

uint32_t ipv4_broadcast_address(uint32_t ip, int prefix) {
    uint32_t mask = ipv4_prefix_to_mask(prefix);
    return ip | ~mask;
}

uint32_t ipv4_wildcard_mask(int prefix) {
    return ~ipv4_prefix_to_mask(prefix);
}

char ipv4_net_class(uint32_t ip) {
    uint32_t first_byte = (ip >> 24) & 0xFF;
    if (first_byte <= 127) {
        return 'A';
    }
    if (first_byte <= 191) {
        return 'B';
    }
    if (first_byte <= 223) {
        return 'C';
    }
    if (first_byte <= 239) {
        return 'D';
    }
    return 'E';
}

int ipv4_class_bits(char net_class) {
    switch (net_class) {
    case 'A':
        return 8;
    case 'B':
        return 16;
    case 'C':
        return 24;
    default:
        return 32;
    }
}

int ipv4_subnet_bits(char net_class, int prefix) {
    int cbits = ipv4_class_bits(net_class);
    if (prefix > cbits) {
        return prefix - cbits;
    }
    return 0;
}

int ipv4_host_bits(int prefix) {
    if (prefix < 0) {
        return 32;
    }
    if (prefix > 32) {
        return 0;
    }
    return 32 - prefix;
}

uint32_t ipv4_max_hosts(int prefix) {
    if (prefix == 31) {
        return 2;
    }
    if (prefix == 32) {
        return 1;
    }
    if (prefix <= 0) {
        return 4294967294U;
    }
    uint32_t host_span = 1U << (unsigned int)(32 - prefix);
    return host_span >= 2 ? host_span - 2 : 0;
}

uint64_t ipv4_max_subnets(int subnet_bits) {
    if (subnet_bits <= 0) {
        return 1;
    }
    if (subnet_bits >= 32) {
        return 0xFFFFFFFFU;
    }
    return (uint64_t)1ULL << (unsigned int)subnet_bits;
}

uint64_t ipv4_max_cidr_subnets(int prefix) {
    if (prefix < 0) {
        prefix = 0;
    }
    if (prefix > 32) {
        prefix = 32;
    }
    return (uint64_t)1ULL << (unsigned int)(32 - prefix);
}

uint64_t ipv4_max_cidr_supernet(char net_class, int prefix) {
    int cbits = ipv4_class_bits(net_class);
    if (cbits - prefix > 0) {
        return (uint64_t)1ULL << (unsigned int)(cbits - prefix);
    }
    return 1;
}

void ipv4_usable_range(uint32_t ip, int prefix, uint32_t *out_start, uint32_t *out_end,
                       uint32_t *out_count) {
    uint32_t net = ipv4_network_address(ip, prefix);
    uint32_t bcast = ipv4_broadcast_address(ip, prefix);

    if (prefix == 31) {
        if (out_start) {
            *out_start = net;
        }
        if (out_end) {
            *out_end = bcast;
        }
        if (out_count) {
            *out_count = 2;
        }
    } else if (prefix == 32) {
        if (out_start) {
            *out_start = ip;
        }
        if (out_end) {
            *out_end = ip;
        }
        if (out_count) {
            *out_count = 1;
        }
    } else if (prefix <= 0) {
        if (out_start) {
            *out_start = 1;
        }
        if (out_end) {
            *out_end = 0xFFFFFFFEU;
        }
        if (out_count) {
            *out_count = 4294967294U;
        }
    } else {
        if (out_start) {
            *out_start = net + 1;
        }
        if (out_end) {
            *out_end = bcast > 0 ? bcast - 1 : 0;
        }
        if (out_count) {
            uint32_t total = (1U << (unsigned int)(32 - prefix));
            *out_count = total >= 2 ? total - 2 : 0;
        }
    }
}

void ipv4_binarize(uint32_t ip, bool dotted, char *buf, size_t buflen) {
    if (!buf || buflen == 0) {
        return;
    }
    char tmp[36];
    int pos = 0;
    for (int byte = 3; byte >= 0; byte--) {
        uint32_t b = (ip >> (unsigned int)(byte * 8)) & 0xFF;
        for (int bit = 7; bit >= 0; bit--) {
            tmp[pos++] = ((b >> (unsigned int)bit) & 1U) ? '1' : '0';
        }
        if (byte > 0 && dotted) {
            tmp[pos++] = '.';
        }
    }
    tmp[pos] = '\0';
    snprintf(buf, buflen, "%s", tmp);
}

void ipv4_hexarize(uint32_t ip, bool dotted, char *buf, size_t buflen) {
    if (!buf || buflen == 0) {
        return;
    }
    if (dotted) {
        snprintf(buf, buflen, "%02X.%02X.%02X.%02X", (ip >> 24) & 0xFF, (ip >> 16) & 0xFF,
                 (ip >> 8) & 0xFF, ip & 0xFF);
    } else {
        snprintf(buf, buflen, "%08X", ip);
    }
}

void ipv4_bitmap(char net_class, int prefix, bool dotted, char *buf, size_t buflen) {
    if (!buf || buflen == 0) {
        return;
    }
    int cbits = ipv4_class_bits(net_class);
    int net_bits = (prefix > cbits) ? cbits : prefix;
    int sbits = (prefix > cbits) ? (prefix - cbits) : 0;

    char tmp[40];
    int pos = 0;
    for (int i = 0; i < 32; i++) {
        if (i < net_bits) {
            tmp[pos++] = 'n';
        } else if (i < net_bits + sbits) {
            tmp[pos++] = 's';
        } else {
            tmp[pos++] = 'h';
        }
        if (i < 31 && (i + 1) % 8 == 0 && dotted) {
            tmp[pos++] = '.';
        }
    }
    tmp[pos] = '\0';
    snprintf(buf, buflen, "%s", tmp);
}
