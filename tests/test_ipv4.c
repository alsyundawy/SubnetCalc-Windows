#include "engine/classify.h"
#include "engine/ipv4.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

static void test_parse_and_format(void) {
    uint32_t ip = 0;
    assert(ipv4_parse("192.168.1.10", &ip));
    char buf[IPV4_STR_BUFFER_SIZE];
    ipv4_format(ip, buf, sizeof(buf));
    assert(strcmp(buf, "192.168.1.10") == 0);

    assert(ipv4_parse("0.0.0.0", &ip));
    assert(ip == 0);

    assert(ipv4_parse("255.255.255.255", &ip));
    assert(ip == 0xFFFFFFFFU);

    /* Invalid parses */
    assert(!ipv4_parse("256.1.1.1", &ip));
    assert(!ipv4_parse("192.168.1", &ip));
    assert(!ipv4_parse("192.168.1.1.1", &ip));
    assert(!ipv4_parse("abc.def.ghi.jkl", &ip));
}

static void test_mask_continuity(void) {
    int prefix = 0;
    assert(ipv4_mask_is_contiguous(0xFFFFFF00U, &prefix));
    assert(prefix == 24);

    assert(ipv4_mask_is_contiguous(0xFFFFFFFFU, &prefix));
    assert(prefix == 32);

    assert(ipv4_mask_is_contiguous(0x00000000U, &prefix));
    assert(prefix == 0);

    /* Non-contiguous mask 255.255.0.255 MUST be rejected */
    uint32_t non_contig = 0xFFFF00FFU;
    assert(!ipv4_mask_is_contiguous(non_contig, &prefix));
}

static void test_vectors_table(void) {
    FILE *fp = fopen("tests/vectors.txt", "r");
    if (!fp) {
        fp = fopen("../tests/vectors.txt", "r");
    }
    assert(fp != NULL);

    char line[512];
    int count = 0;
    while (fgets(line, sizeof(line), fp)) {
        if (line[0] == '#' || line[0] == '\n' || line[0] == '\r') {
            continue;
        }
        char *nl = strchr(line, '\r');
        if (nl) {
            *nl = '\0';
        }
        nl = strchr(line, '\n');
        if (nl) {
            *nl = '\0';
        }

        /* Parse tokens */
        char ip_prefix[64], exp_net[64], exp_bcast[64], exp_mask[64], exp_wild[64];
        char exp_class[8], exp_start[64], exp_end[64], exp_hosts[64], exp_classify[128];

        int scanned = sscanf(
            line,
            "%63[^|]|%63[^|]|%63[^|]|%63[^|]|%63[^|]|%7[^|]|%63[^|]|%63[^|]|%63[^|]|%127[^\n]",
            ip_prefix, exp_net, exp_bcast, exp_mask, exp_wild, exp_class, exp_start, exp_end,
            exp_hosts, exp_classify);
        assert(scanned == 10);

        char ip_str[64];
        int prefix = 0;
        char *slash = strchr(ip_prefix, '/');
        assert(slash != NULL);
        *slash = '\0';
        strncpy(ip_str, ip_prefix, sizeof(ip_str) - 1);
        prefix = atoi(slash + 1);

        uint32_t ip = 0;
        assert(ipv4_parse(ip_str, &ip));

        char buf[IPV4_STR_BUFFER_SIZE];

        /* Network */
        uint32_t net = ipv4_network_address(ip, prefix);
        ipv4_format(net, buf, sizeof(buf));
        assert(strcmp(buf, exp_net) == 0);

        /* Broadcast */
        uint32_t bcast = ipv4_broadcast_address(ip, prefix);
        ipv4_format(bcast, buf, sizeof(buf));
        assert(strcmp(buf, exp_bcast) == 0);

        /* Mask */
        uint32_t mask = ipv4_prefix_to_mask(prefix);
        ipv4_format(mask, buf, sizeof(buf));
        assert(strcmp(buf, exp_mask) == 0);

        /* Wildcard */
        uint32_t wild = ipv4_wildcard_mask(prefix);
        ipv4_format(wild, buf, sizeof(buf));
        assert(strcmp(buf, exp_wild) == 0);

        /* Class */
        char c = ipv4_net_class(ip);
        assert(c == exp_class[0]);

        /* Usable range */
        uint32_t start = 0, end = 0, host_count = 0;
        ipv4_usable_range(ip, prefix, &start, &end, &host_count);
        ipv4_format(start, buf, sizeof(buf));
        assert(strcmp(buf, exp_start) == 0);

        ipv4_format(end, buf, sizeof(buf));
        assert(strcmp(buf, exp_end) == 0);

        /* Host count */
        char hosts_str[32];
        snprintf(hosts_str, sizeof(hosts_str), "%u", host_count);
        assert(strcmp(hosts_str, exp_hosts) == 0);

        /* Classification */
        const char *classification = classify_ipv4(ip);
        assert(strcmp(classification, exp_classify) == 0);

        count++;
    }
    fclose(fp);
    printf("[PASS] Verified %d golden IPv4 vectors from vectors.txt\n", count);
}

static void test_rfc3021_and_32(void) {
    uint32_t ip = 0;
    assert(ipv4_parse("10.1.2.3", &ip));

    /* /31 RFC 3021 */
    uint32_t start = 0, end = 0, count = 0;
    ipv4_usable_range(ip, 31, &start, &end, &count);
    assert(count == 2);
    char buf[IPV4_STR_BUFFER_SIZE];
    ipv4_format(start, buf, sizeof(buf));
    assert(strcmp(buf, "10.1.2.2") == 0);
    ipv4_format(end, buf, sizeof(buf));
    assert(strcmp(buf, "10.1.2.3") == 0);

    /* /32 Single Host */
    ipv4_usable_range(ip, 32, &start, &end, &count);
    assert(count == 1);
    ipv4_format(start, buf, sizeof(buf));
    assert(strcmp(buf, "10.1.2.3") == 0);
    ipv4_format(end, buf, sizeof(buf));
    assert(strcmp(buf, "10.1.2.3") == 0);
}

static void test_bitmap_and_representations(void) {
    uint32_t ip = 0;
    assert(ipv4_parse("192.168.1.10", &ip));
    char bin[IPV4_STR_BUFFER_SIZE];
    char hex[IPV4_STR_BUFFER_SIZE];
    char bmap[IPV4_BITMAP_BUFFER_SIZE];

    ipv4_binarize(ip, true, bin, sizeof(bin));
    assert(strcmp(bin, "11000000.10101000.00000001.00001010") == 0);

    ipv4_hexarize(ip, true, hex, sizeof(hex));
    assert(strcmp(hex, "C0.A8.01.0A") == 0);

    char c = ipv4_net_class(ip);
    ipv4_bitmap(c, 24, true, bmap, sizeof(bmap));
    assert(strcmp(bmap, "nnnnnnnn.nnnnnnnn.nnnnnnnn.hhhhhhhh") == 0);

    ipv4_bitmap(c, 26, true, bmap, sizeof(bmap));
    assert(strcmp(bmap, "nnnnnnnn.nnnnnnnn.nnnnnnnn.sshhhhhh") == 0);
}

int main(void) {
    printf("==> Running IPv4 engine unit tests...\n");
    test_parse_and_format();
    test_mask_continuity();
    test_vectors_table();
    test_rfc3021_and_32();
    test_bitmap_and_representations();
    printf("[PASS] All IPv4 engine tests passed successfully.\n");
    return 0;
}
