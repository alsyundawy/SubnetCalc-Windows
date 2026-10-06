#include "engine/cloud.h"
#include "engine/split.h"
#include "engine/cidr.h"
#include "engine/ipv4.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

static void test_cloud_counts(void) {
    uint32_t ip = 0;
    assert(ipv4_parse("10.0.0.0", &ip));

    uint32_t s = 0, e = 0, count = 0;
    char sbuf[32], ebuf[32];

    /* AWS /24 -> 251 hosts, 10.0.0.4 - 10.0.0.254 */
    assert(cloud_profile_usable_range(CLOUD_PROFILE_AWS, ip, 24, &s, &e, &count));
    assert(count == 251);
    ipv4_format(s, sbuf, sizeof(sbuf));
    ipv4_format(e, ebuf, sizeof(ebuf));
    assert(strcmp(sbuf, "10.0.0.4") == 0);
    assert(strcmp(ebuf, "10.0.0.254") == 0);

    /* Azure /24 -> 251 hosts, 10.0.0.4 - 10.0.0.254 */
    assert(cloud_profile_usable_range(CLOUD_PROFILE_AZURE, ip, 24, &s, &e, &count));
    assert(count == 251);
    ipv4_format(s, sbuf, sizeof(sbuf));
    ipv4_format(e, ebuf, sizeof(ebuf));
    assert(strcmp(sbuf, "10.0.0.4") == 0);
    assert(strcmp(ebuf, "10.0.0.254") == 0);

    /* GCP /24 -> 252 hosts, 10.0.0.2 - 10.0.0.253 */
    assert(cloud_profile_usable_range(CLOUD_PROFILE_GCP, ip, 24, &s, &e, &count));
    assert(count == 252);
    ipv4_format(s, sbuf, sizeof(sbuf));
    ipv4_format(e, ebuf, sizeof(ebuf));
    assert(strcmp(sbuf, "10.0.0.2") == 0);
    assert(strcmp(ebuf, "10.0.0.253") == 0);

    /* OCI /24 -> 253 hosts, 10.0.0.2 - 10.0.0.254 */
    assert(cloud_profile_usable_range(CLOUD_PROFILE_OCI, ip, 24, &s, &e, &count));
    assert(count == 253);
    ipv4_format(s, sbuf, sizeof(sbuf));
    ipv4_format(e, ebuf, sizeof(ebuf));
    assert(strcmp(sbuf, "10.0.0.2") == 0);
    assert(strcmp(ebuf, "10.0.0.254") == 0);

    /* Minimum prefix checks */
    assert(!cloud_profile_usable_range(CLOUD_PROFILE_AWS, ip, 29, &s, &e, &count));   /* Min /28 */
    assert(!cloud_profile_usable_range(CLOUD_PROFILE_AZURE, ip, 30, &s, &e, &count)); /* Min /29 */
    assert(!cloud_profile_usable_range(CLOUD_PROFILE_GCP, ip, 30, &s, &e, &count));   /* Min /29 */
    assert(!cloud_profile_usable_range(CLOUD_PROFILE_OCI, ip, 31, &s, &e, &count));   /* Min /30 */
}

static void test_flsm_split(void) {
    uint32_t ip = 0;
    assert(ipv4_parse("192.168.1.0", &ip));

    subnet_slice_t slices[4];
    size_t count = flsm_split(ip, 24, 26, slices, 4);
    assert(count == 4);

    char buf[32];
    ipv4_format(slices[0].network, buf, sizeof(buf));
    assert(strcmp(buf, "192.168.1.0") == 0);

    ipv4_format(slices[1].network, buf, sizeof(buf));
    assert(strcmp(buf, "192.168.1.64") == 0);

    ipv4_format(slices[2].network, buf, sizeof(buf));
    assert(strcmp(buf, "192.168.1.128") == 0);

    ipv4_format(slices[3].network, buf, sizeof(buf));
    assert(strcmp(buf, "192.168.1.192") == 0);
}

static void test_vlsm(void) {
    uint32_t ip = 0;
    assert(ipv4_parse("192.168.1.0", &ip));

    vlsm_req_t reqs[3] = {{"Engineering", 60, 0, 0, 0, 0, 0, 0, 0},
                          {"Sales", 25, 0, 0, 0, 0, 0, 0, 0},
                          {"Servers", 10, 0, 0, 0, 0, 0, 0, 0}};

    vlsm_summary_t sum;
    assert(vlsm_calculate(ip, 24, reqs, 3, &sum));
    assert(sum.fits);
    assert(sum.total_requested == 95);
    /* 60 hosts -> /26 (62 hosts), 25 hosts -> /27 (30 hosts), 10 hosts -> /28 (14 hosts) -> total
     * 106 */
    assert(sum.total_allocated == 106);
    assert(sum.total_wasted == 11);
    assert(sum.efficiency_percent > 89.0 && sum.efficiency_percent < 90.0);
}

static void test_cidr_aggregation(void) {
    cidr_block_t blocks[4] = {
        {0xC0A80000U, 24}, /* 192.168.0.0/24 */
        {0xC0A80100U, 24}, /* 192.168.1.0/24 */
        {0xC0A80200U, 24}, /* 192.168.2.0/24 */
        {0xC0A80300U, 24}  /* 192.168.3.0/24 */
    };

    size_t count = cidr_aggregate(blocks, 4);
    assert(count == 1);
    assert(blocks[0].prefix == 22);
    char buf[32];
    ipv4_format(blocks[0].network, buf, sizeof(buf));
    assert(strcmp(buf, "192.168.0.0") == 0);
}

int main(void) {
    printf("==> Running cloud profiles, FLSM, VLSM, CIDR tests...\n");
    test_cloud_counts();
    test_flsm_split();
    test_vlsm();
    test_cidr_aggregation();
    printf("[PASS] All cloud, FLSM, VLSM, CIDR tests passed successfully.\n");
    return 0;
}
