#include "ula.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if defined(_WIN32)
#define WINVER 0x0601
#define _WIN32_WINNT 0x0601
#include <windows.h>
#include <bcrypt.h>
#include <wincrypt.h>
#endif

static bool get_csprng_entropy(uint8_t *bytes, size_t count) {
#if defined(_WIN32)
    /* Windows 7 SP1 BCryptGenRandom */
    NTSTATUS status = BCryptGenRandom(NULL, bytes, (ULONG)count, BCRYPT_USE_SYSTEM_PREFERRED_RNG);
    if (status == 0) {
        return true;
    }
    /* Fallback to CryptGenRandom */
    HCRYPTPROV hProv = 0;
    if (CryptAcquireContext(&hProv, NULL, NULL, PROV_RSA_FULL,
                            CRYPT_VERIFYCONTEXT | CRYPT_SILENT)) {
        BOOL ok = CryptGenRandom(hProv, (DWORD)count, bytes);
        CryptReleaseContext(hProv, 0);
        if (ok) {
            return true;
        }
    }
    return false;
#else
    /* Non-Windows POSIX host testing (/dev/urandom) */
    FILE *fp = fopen("/dev/urandom", "rb");
    if (fp) {
        size_t n = fread(bytes, 1, count, fp);
        fclose(fp);
        return (n == count);
    }
    return false;
#endif
}

bool ula_generate(ipv6_addr_t *out_prefix48, ipv6_addr_t *out_subnet64, char *out_prefix48_str,
                  size_t prefix_len, char *out_subnet64_str, size_t subnet_len) {
    uint8_t rand_bytes[5] = {0};
    if (!get_csprng_entropy(rand_bytes, 5)) {
        return false;
    }

    /* RFC 4193: Prefix fd (7 bits fc00::/7 + L bit 1) + 40-bit Global ID */
    uint16_t w0 = (uint16_t)(0xFD00U | (uint16_t)rand_bytes[0]);
    uint16_t w1 = (uint16_t)(((uint16_t)rand_bytes[1] << 8) | (uint16_t)rand_bytes[2]);
    uint16_t w2 = (uint16_t)(((uint16_t)rand_bytes[3] << 8) | (uint16_t)rand_bytes[4]);

    if (out_prefix48) {
        out_prefix48->w[0] = w0;
        out_prefix48->w[1] = w1;
        out_prefix48->w[2] = w2;
        for (int i = 3; i < 8; i++) {
            out_prefix48->w[i] = 0;
        }
    }
    if (out_subnet64) {
        out_subnet64->w[0] = w0;
        out_subnet64->w[1] = w1;
        out_subnet64->w[2] = w2;
        out_subnet64->w[3] = 0x0001; /* Default subnet 1 */
        for (int i = 4; i < 8; i++) {
            out_subnet64->w[i] = 0;
        }
    }

    if (out_prefix48_str && prefix_len > 0) {
        snprintf(out_prefix48_str, prefix_len, "fd%02x:%02x%02x:%02x%02x::/48", rand_bytes[0],
                 rand_bytes[1], rand_bytes[2], rand_bytes[3], rand_bytes[4]);
    }
    if (out_subnet64_str && subnet_len > 0) {
        snprintf(out_subnet64_str, subnet_len, "fd%02x:%02x%02x:%02x%02x:0001::/64", rand_bytes[0],
                 rand_bytes[1], rand_bytes[2], rand_bytes[3], rand_bytes[4]);
    }

    return true;
}
