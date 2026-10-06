#include "classify.h"
#include "ipv4.h"
#include <ctype.h>
#include <string.h>

const char *classify_ipv4(uint32_t ip) {
    uint32_t b1 = (ip >> 24) & 0xFF;
    uint32_t b2 = (ip >> 16) & 0xFF;

    if (b1 == 0) {
        return "RFC 1122 This Host on This Network";
    }
    if (b1 == 10) {
        return "RFC 1918 Private (Class A)";
    }
    if (b1 == 172 && (b2 >= 16 && b2 <= 31)) {
        return "RFC 1918 Private (Class B)";
    }
    if (b1 == 192 && b2 == 168) {
        return "RFC 1918 Private (Class C)";
    }
    if (b1 == 100 && (b2 >= 64 && b2 <= 127)) {
        return "RFC 6598 CGNAT / Shared Space";
    }
    if (b1 == 127) {
        return "RFC 1122 Loopback";
    }
    if (b1 == 169 && b2 == 254) {
        return "RFC 3927 Link-Local (APIPA)";
    }
    if (b1 >= 224 && b1 <= 239) {
        return "RFC 5771 Multicast (Class D)";
    }
    if (b1 >= 240) {
        return "RFC 1122 Reserved / Experimental (Class E)";
    }
    return "Public Routable IPv4";
}

const char *classify_ipv6(const uint16_t words[8]) {
    if (!words) {
        return "Invalid IPv6";
    }
    bool all_zero_except_last = true;
    for (int i = 0; i < 7; i++) {
        if (words[i] != 0) {
            all_zero_except_last = false;
            break;
        }
    }
    if (all_zero_except_last && words[7] == 1) {
        return "RFC 4291 Loopback";
    }
    if (all_zero_except_last && words[7] == 0) {
        return "RFC 4291 Unspecified";
    }

    uint16_t w0 = words[0];
    if ((w0 & 0xFE00) == 0xFC00) {
        return "RFC 4193 Unique Local (ULA Private)";
    }
    if ((w0 & 0xFFC0) == 0xFE80) {
        return "RFC 4291 Link-Local Unicast";
    }
    if ((w0 & 0xFF00) == 0xFF00) {
        return "RFC 4291 Multicast";
    }
    if (w0 == 0x2001 && words[1] == 0x0DB8) {
        return "RFC 3849 Documentation";
    }
    if ((w0 & 0xE000) == 0x2000) {
        return "RFC 4291 Global Unicast (Public)";
    }

    bool first5_zero = true;
    for (int i = 0; i < 5; i++) {
        if (words[i] != 0) {
            first5_zero = false;
            break;
        }
    }
    if (first5_zero && words[5] == 0xFFFF) {
        return "RFC 4291 IPv4-Mapped";
    }
    if (w0 == 0x0064 && words[1] == 0xFF9B) {
        return "RFC 6052 IPv4-IPv6 Translation";
    }

    return "IPv6";
}

const char *classify_any(const char *addr_str) {
    if (!addr_str) {
        return "Invalid IP";
    }
    while (isspace((unsigned char)*addr_str)) {
        addr_str++;
    }
    if (strchr(addr_str, ':')) {
        return "IPv6";
    }
    uint32_t ip = 0;
    if (ipv4_parse(addr_str, &ip)) {
        return classify_ipv4(ip);
    }
    return "Invalid IP";
}
