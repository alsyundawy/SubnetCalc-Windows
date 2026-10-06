#include "ipv6.h"
#include "biguint.h"
#include "ipv4.h"
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

bool ipv6_parse(const char *str, ipv6_addr_t *out_addr) {
    if (!str || !out_addr) {
        return false;
    }
    while (isspace((unsigned char)*str)) {
        str++;
    }

    uint16_t left[8] = {0};
    uint16_t right[8] = {0};
    int left_count = 0;
    int right_count = 0;
    bool has_double_colon = false;

    const char *p = str;

    /* Check for leading :: */
    if (p[0] == ':' && p[1] == ':') {
        has_double_colon = true;
        p += 2;
    } else if (p[0] == ':') {
        return false; /* Invalid single leading colon */
    }

    while (*p && *p != '/' && !isspace((unsigned char)*p)) {
        /* Check for :: inside */
        if (p[0] == ':' && p[1] == ':') {
            if (has_double_colon) {
                return false; /* More than one :: */
            }
            has_double_colon = true;
            p += 2;
            continue;
        }

        /* Check for IPv4-mapped address at end */
        const char *dot = strchr(p, '.');
        const char *colon = strchr(p, ':');
        if (dot != NULL && (colon == NULL || dot < colon)) {
            uint32_t ipv4 = 0;
            if (!ipv4_parse(p, &ipv4)) {
                return false;
            }
            uint16_t w1 = (uint16_t)((ipv4 >> 16) & 0xFFFFU);
            uint16_t w2 = (uint16_t)(ipv4 & 0xFFFFU);
            if (has_double_colon) {
                if (right_count + 2 > 8) {
                    return false;
                }
                right[right_count++] = w1;
                right[right_count++] = w2;
            } else {
                if (left_count + 2 > 8) {
                    return false;
                }
                left[left_count++] = w1;
                left[left_count++] = w2;
            }
            break;
        }

        /* Parse hex word */
        char hex_buf[5] = {0};
        int hlen = 0;
        while (*p && isxdigit((unsigned char)*p) && hlen < 4) {
            hex_buf[hlen++] = *p++;
        }
        if (hlen == 0 || (isxdigit((unsigned char)*p) && hlen == 4)) {
            return false; /* > 4 hex digits */
        }
        uint16_t val = (uint16_t)strtoul(hex_buf, NULL, 16);

        if (has_double_colon) {
            if (right_count >= 8) {
                return false;
            }
            right[right_count++] = val;
        } else {
            if (left_count >= 8) {
                return false;
            }
            left[left_count++] = val;
        }

        if (*p == ':') {
            if (p[1] == ':') {
                if (has_double_colon) {
                    return false;
                }
                has_double_colon = true;
                p += 2;
            } else {
                p++;
                if (*p == '\0' || *p == '/' || isspace((unsigned char)*p)) {
                    return false; /* Trailing single colon */
                }
            }
        } else if (*p == '\0' || *p == '/' || isspace((unsigned char)*p)) {
            break;
        } else {
            return false;
        }
    }

    if (!has_double_colon) {
        if (left_count != 8) {
            return false;
        }
        for (int i = 0; i < 8; i++) {
            out_addr->w[i] = left[i];
        }
    } else {
        int total = left_count + right_count;
        if (total > 7) {
            return false;
        }
        int zeros = 8 - total;
        int idx = 0;
        for (int i = 0; i < left_count; i++) {
            out_addr->w[idx++] = left[i];
        }
        for (int i = 0; i < zeros; i++) {
            out_addr->w[idx++] = 0;
        }
        for (int i = 0; i < right_count; i++) {
            out_addr->w[idx++] = right[i];
        }
    }

    return true;
}

void ipv6_format_expanded(const ipv6_addr_t *addr, char *buf, size_t buflen) {
    if (!addr || !buf || buflen == 0) {
        return;
    }
    snprintf(buf, buflen, "%04x:%04x:%04x:%04x:%04x:%04x:%04x:%04x", addr->w[0], addr->w[1],
             addr->w[2], addr->w[3], addr->w[4], addr->w[5], addr->w[6], addr->w[7]);
}

void ipv6_format_compact(const ipv6_addr_t *addr, char *buf, size_t buflen) {
    if (!addr || !buf || buflen == 0) {
        return;
    }

    /* Find longest run of zeros */
    int best_start = -1;
    int best_len = 0;
    int cur_start = -1;
    int cur_len = 0;

    for (int i = 0; i < 8; i++) {
        if (addr->w[i] == 0) {
            if (cur_start == -1) {
                cur_start = i;
                cur_len = 1;
            } else {
                cur_len++;
            }
            if (cur_len > best_len) {
                best_len = cur_len;
                best_start = cur_start;
            }
        } else {
            cur_start = -1;
            cur_len = 0;
        }
    }

    /* RFC 5952: A single 16-bit 0 field MUST NOT be compressed */
    if (best_len < 2) {
        best_start = -1;
        best_len = 0;
    }

    char tmp[64] = {0};
    int pos = 0;

    for (int i = 0; i < 8; i++) {
        if (best_start != -1 && i == best_start) {
            tmp[pos++] = ':';
            tmp[pos++] = ':';
            i += best_len - 1;
            continue;
        }
        if (i > 0 && !(best_start != -1 && i == best_start + best_len)) {
            if (pos > 0 && tmp[pos - 1] != ':') {
                tmp[pos++] = ':';
            }
        }
        pos += snprintf(tmp + pos, sizeof(tmp) - (size_t)pos, "%x", addr->w[i]);
    }

    tmp[pos] = '\0';
    snprintf(buf, buflen, "%s", tmp);
}

void ipv6_apply_mask(const ipv6_addr_t *addr, int prefix, ipv6_addr_t *out_net) {
    if (!addr || !out_net) {
        return;
    }
    if (prefix < 0) {
        prefix = 0;
    }
    if (prefix >= 128) {
        *out_net = *addr;
        return;
    }
    if (prefix <= 0) {
        for (int i = 0; i < 8; i++) {
            out_net->w[i] = 0;
        }
        return;
    }

    for (int i = 0; i < 8; i++) {
        int word_bits = prefix - (i * 16);
        if (word_bits >= 16) {
            out_net->w[i] = addr->w[i];
        } else if (word_bits <= 0) {
            out_net->w[i] = 0;
        } else {
            uint16_t mask = (uint16_t)(0xFFFFU << (16 - word_bits));
            out_net->w[i] = addr->w[i] & mask;
        }
    }
}

void ipv6_range(const ipv6_addr_t *addr, int prefix, ipv6_addr_t *out_start, ipv6_addr_t *out_end) {
    ipv6_addr_t net;
    ipv6_apply_mask(addr, prefix, &net);

    if (out_start) {
        *out_start = net;
    }
    if (out_end) {
        for (int i = 0; i < 8; i++) {
            int word_bits = prefix - (i * 16);
            if (word_bits >= 16) {
                out_end->w[i] = net.w[i];
            } else if (word_bits <= 0) {
                out_end->w[i] = 0xFFFFU;
            } else {
                uint16_t host_mask = (uint16_t)(0xFFFFU >> word_bits);
                out_end->w[i] = net.w[i] | host_mask;
            }
        }
    }
}

void ipv6_arpa(const ipv6_addr_t *addr, char *buf, size_t buflen) {
    if (!addr || !buf || buflen == 0) {
        return;
    }
    char expanded[40];
    ipv6_format_expanded(addr, expanded, sizeof(expanded));

    char hex_digits[33] = {0};
    int digit_idx = 0;
    for (size_t i = 0; i < strlen(expanded); i++) {
        if (isxdigit((unsigned char)expanded[i])) {
            hex_digits[digit_idx++] = (char)tolower((unsigned char)expanded[i]);
        }
    }

    char tmp[80] = {0};
    int pos = 0;
    for (int i = 31; i >= 0; i--) {
        tmp[pos++] = hex_digits[i];
        tmp[pos++] = '.';
    }
    snprintf(tmp + pos, sizeof(tmp) - (size_t)pos, "ip6.arpa");
    snprintf(buf, buflen, "%s", tmp);
}

void ipv6_total_hosts(int prefix, char *buf, size_t buflen) {
    if (!buf || buflen == 0) {
        return;
    }
    if (prefix < 0) {
        prefix = 0;
    }
    if (prefix > 128) {
        prefix = 128;
    }
    int host_bits = 128 - prefix;
    biguint128_t val;
    biguint128_two_pow(&val, host_bits);
    biguint128_format_dec(&val, buf, buflen);
}

bool ipv6_is_ipv4_mapped(const ipv6_addr_t *addr, uint32_t *out_ipv4) {
    if (!addr) {
        return false;
    }
    for (int i = 0; i < 5; i++) {
        if (addr->w[i] != 0) {
            return false;
        }
    }
    if (addr->w[5] != 0xFFFFU) {
        return false;
    }
    if (out_ipv4) {
        *out_ipv4 = ((uint32_t)addr->w[6] << 16) | (uint32_t)addr->w[7];
    }
    return true;
}

bool ipv6_is_6to4(const ipv6_addr_t *addr, uint32_t *out_ipv4) {
    if (!addr || addr->w[0] != 0x2002) {
        return false;
    }
    if (out_ipv4) {
        *out_ipv4 = ((uint32_t)addr->w[1] << 16) | (uint32_t)addr->w[2];
    }
    return true;
}

void ipv6_from_ipv4_mapped(uint32_t ipv4, ipv6_addr_t *out_addr) {
    if (!out_addr) {
        return;
    }
    for (int i = 0; i < 5; i++) {
        out_addr->w[i] = 0;
    }
    out_addr->w[5] = 0xFFFFU;
    out_addr->w[6] = (uint16_t)((ipv4 >> 16) & 0xFFFFU);
    out_addr->w[7] = (uint16_t)(ipv4 & 0xFFFFU);
}

void ipv6_from_ipv4_6to4(uint32_t ipv4, ipv6_addr_t *out_addr) {
    if (!out_addr) {
        return;
    }
    out_addr->w[0] = 0x2002;
    out_addr->w[1] = (uint16_t)((ipv4 >> 16) & 0xFFFFU);
    out_addr->w[2] = (uint16_t)(ipv4 & 0xFFFFU);
    for (int i = 3; i < 8; i++) {
        out_addr->w[i] = 0;
    }
}
