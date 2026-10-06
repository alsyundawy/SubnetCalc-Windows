#include "engine/export.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

static void test_cwe1236_sanitization(void) {
    char out[128];

    /* Dangerous formula injection prefix '=' */
    export_sanitize_csv_cell("=cmd|'/C calc'!A0", out, sizeof(out));
    assert(strcmp(out, "\"'=cmd|'/C calc'!A0\"") == 0);

    /* Dangerous '+' */
    export_sanitize_csv_cell("+1+1", out, sizeof(out));
    assert(strcmp(out, "\"'+1+1\"") == 0);

    /* Dangerous '@' */
    export_sanitize_csv_cell("@SUM(1,2)", out, sizeof(out));
    assert(strcmp(out, "\"'@SUM(1,2)\"") == 0);

    /* Standard cell with comma */
    export_sanitize_csv_cell("Hello, World", out, sizeof(out));
    assert(strcmp(out, "\"Hello, World\"") == 0);

    /* Normal cell */
    export_sanitize_csv_cell("192.168.1.0", out, sizeof(out));
    assert(strcmp(out, "192.168.1.0") == 0);
}

static void test_ascii_table(void) {
    const char *headers[] = {"Subnet", "Mask", "Hosts"};
    const char *row1[] = {"192.168.1.0", "255.255.255.0", "254"};
    const char *row2[] = {"10.0.0.0", "255.0.0.0", "16777214"};
    const char *const *rows[] = {row1, row2};

    FILE *tmp = tmpfile();
    assert(tmp != NULL);
    export_write_ascii_table(tmp, headers, 3, rows, 2);
    rewind(tmp);

    char buf[1024];
    size_t n = fread(buf, 1, sizeof(buf) - 1, tmp);
    buf[n] = '\0';
    fclose(tmp);

    assert(strstr(buf, "| Subnet") != NULL);
    assert(strstr(buf, "| 192.168.1.0") != NULL);
    assert(strstr(buf, "| 16777214") != NULL);
}

int main(void) {
    printf("==> Running export engine unit tests...\n");
    test_cwe1236_sanitization();
    test_ascii_table();
    printf("[PASS] All export engine tests passed successfully.\n");
    return 0;
}
