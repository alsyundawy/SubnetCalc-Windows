#include "engine/biguint.h"
#include "engine/classify.h"
#include "engine/ipv6.h"
#include "engine/ula.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

static void test_rfc5952_compression(void) {
    ipv6_addr_t addr;
    char buf[IPV6_COMPACT_BUFFER_SIZE];

    /* 2001:db8::1 */
    assert(ipv6_parse("2001:db8::1", &addr));
    ipv6_format_compact(&addr, buf, sizeof(buf));
    assert(strcmp(buf, "2001:db8::1") == 0);

    /* ::1 */
    assert(ipv6_parse("::1", &addr));
    ipv6_format_compact(&addr, buf, sizeof(buf));
    assert(strcmp(buf, "::1") == 0);

    /* 2001:db8:0:0:1:: */
    assert(ipv6_parse("2001:db8:0:0:1:0:0:0", &addr));
    ipv6_format_compact(&addr, buf, sizeof(buf));
    /* Longest sequence of zeros is at end (3 words) */
    assert(strcmp(buf, "2001:db8:0:0:1::") == 0);

    /* Single zero group must NOT be compressed per RFC 5952 */
    assert(ipv6_parse("2001:db8:0:1:1:1:1:1", &addr));
    ipv6_format_compact(&addr, buf, sizeof(buf));
    assert(strcmp(buf, "2001:db8:0:1:1:1:1:1") == 0);
}

static void test_parsing_rules(void) {
    ipv6_addr_t addr;

    /* Reject multiple :: */
    assert(!ipv6_parse("2001::1::2", &addr));

    /* Reject > 8 groups */
    assert(!ipv6_parse("1:2:3:4:5:6:7:8:9", &addr));

    /* Reject single leading colon */
    assert(!ipv6_parse(":2001:db8::1", &addr));

    /* Reject single trailing colon */
    assert(!ipv6_parse("2001:db8::1:", &addr));

    /* Accept IPv4-mapped notation */
    assert(ipv6_parse("::ffff:192.0.2.1", &addr));
    uint32_t ip4 = 0;
    assert(ipv6_is_ipv4_mapped(&addr, &ip4));
    assert(ip4 == 0xC0000201U);
}

static void test_arpa_and_range(void) {
    ipv6_addr_t addr;
    assert(ipv6_parse("2001:db8::1", &addr));

    char arpa[IPV6_ARPA_BUFFER_SIZE];
    ipv6_arpa(&addr, arpa, sizeof(arpa));
    assert(strcmp(arpa,
                  "1.0.0.0.0.0.0.0.0.0.0.0.0.0.0.0.0.0.0.0.0.0.0.0.8.b.d.0.1.0.0.2.ip6.arpa") == 0);

    /* /64 range */
    ipv6_addr_t start, end;
    ipv6_range(&addr, 64, &start, &end);

    char sbuf[IPV6_COMPACT_BUFFER_SIZE], ebuf[IPV6_COMPACT_BUFFER_SIZE];
    ipv6_format_compact(&start, sbuf, sizeof(sbuf));
    ipv6_format_compact(&end, ebuf, sizeof(ebuf));
    assert(strcmp(sbuf, "2001:db8::") == 0);
    assert(strcmp(ebuf, "2001:db8::ffff:ffff:ffff:ffff") == 0);
}

static void test_biguint_counts(void) {
    char buf[64];
    ipv6_total_hosts(128, buf, sizeof(buf));
    assert(strcmp(buf, "1") == 0);

    ipv6_total_hosts(127, buf, sizeof(buf));
    assert(strcmp(buf, "2") == 0);

    ipv6_total_hosts(64, buf, sizeof(buf));
    assert(strcmp(buf, "18446744073709551616") == 0);

    ipv6_total_hosts(0, buf, sizeof(buf));
    assert(strcmp(buf, "340282366920938463463374607431768211456") == 0);
}

static void test_mapped_and_6to4(void) {
    uint32_t orig_ip = (192U << 24) | (168U << 16) | (1U << 8) | 10U;

    ipv6_addr_t mapped;
    ipv6_from_ipv4_mapped(orig_ip, &mapped);
    char buf[IPV6_COMPACT_BUFFER_SIZE];
    ipv6_format_compact(&mapped, buf, sizeof(buf));
    assert(strcmp(buf, "::ffff:c0a8:10a") == 0);

    uint32_t extracted_ip = 0;
    assert(ipv6_is_ipv4_mapped(&mapped, &extracted_ip));
    assert(extracted_ip == orig_ip);

    ipv6_addr_t _6to4;
    ipv6_from_ipv4_6to4(orig_ip, &_6to4);
    assert(ipv6_is_6to4(&_6to4, &extracted_ip));
    assert(extracted_ip == orig_ip);
}

static void test_ula_generation(void) {
    char p48[64] = {0};
    char s64[64] = {0};
    ipv6_addr_t addr48, addr64;

    assert(ula_generate(&addr48, &addr64, p48, sizeof(p48), s64, sizeof(s64)));

    /* Verify starts with fd */
    assert(p48[0] == 'f' && p48[1] == 'd');
    assert(s64[0] == 'f' && s64[1] == 'd');
    assert(strstr(p48, "::/48") != NULL);
    assert(strstr(s64, ":0001::/64") != NULL);

    /* Classification of ULA address */
    assert(strcmp(classify_ipv6(addr48.w), "RFC 4193 Unique Local (ULA Private)") == 0);
}

int main(void) {
    printf("==> Running IPv6 engine unit tests...\n");
    test_rfc5952_compression();
    test_parsing_rules();
    test_arpa_and_range();
    test_biguint_counts();
    test_mapped_and_6to4();
    test_ula_generation();
    printf("[PASS] All IPv6 engine tests passed successfully.\n");
    return 0;
}
