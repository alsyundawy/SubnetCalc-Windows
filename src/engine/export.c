#include "export.h"
#include <ctype.h>
#include <string.h>

void export_sanitize_csv_cell(const char *in_val, char *out_val, size_t out_len) {
    if (!out_val || out_len == 0) {
        return;
    }
    if (!in_val) {
        out_val[0] = '\0';
        return;
    }

    /* CWE-1236: Prevent CSV Formula Injection */
    bool needs_prefix = false;
    char first = in_val[0];
    if (first == '=' || first == '+' || first == '-' || first == '@' || first == '|' ||
        first == '\t') {
        needs_prefix = true;
    }

    bool needs_quotes = false;
    if (strchr(in_val, ',') || strchr(in_val, '"') || strchr(in_val, '\n') ||
        strchr(in_val, '\r') || needs_prefix) {
        needs_quotes = true;
    }

    size_t pos = 0;
    if (needs_quotes && pos < out_len - 1) {
        out_val[pos++] = '"';
    }
    if (needs_prefix && pos < out_len - 1) {
        out_val[pos++] = '\'';
    }

    for (size_t i = 0; in_val[i] != '\0' && pos < out_len - 1; i++) {
        if (in_val[i] == '"') {
            if (pos < out_len - 2) {
                out_val[pos++] = '"';
                out_val[pos++] = '"';
            }
        } else {
            out_val[pos++] = in_val[i];
        }
    }

    if (needs_quotes && pos < out_len - 1) {
        out_val[pos++] = '"';
    }
    out_val[pos] = '\0';
}

void export_write_csv_row(FILE *fp, const char *const *cells, size_t num_cells) {
    if (!fp || !cells || num_cells == 0) {
        return;
    }
    char buf[512];
    for (size_t i = 0; i < num_cells; i++) {
        export_sanitize_csv_cell(cells[i], buf, sizeof(buf));
        fprintf(fp, "%s%s", buf, (i + 1 < num_cells) ? "," : "\r\n");
    }
}

void export_write_ascii_table(FILE *fp, const char *const *headers, size_t num_cols,
                              const char *const *const *rows, size_t num_rows) {
    if (!fp || !headers || num_cols == 0) {
        return;
    }

    size_t col_widths[32] = {0};
    for (size_t c = 0; c < num_cols && c < 32; c++) {
        col_widths[c] = strlen(headers[c]);
    }
    for (size_t r = 0; r < num_rows; r++) {
        for (size_t c = 0; c < num_cols && c < 32; c++) {
            size_t len = strlen(rows[r][c]);
            if (len > col_widths[c]) {
                col_widths[c] = len;
            }
        }
    }

    /* Print border */
    for (size_t c = 0; c < num_cols && c < 32; c++) {
        fprintf(fp, "+-");
        for (size_t w = 0; w < col_widths[c]; w++)
            fprintf(fp, "-");
        fprintf(fp, "-");
    }
    fprintf(fp, "+\n");

    /* Print header */
    for (size_t c = 0; c < num_cols && c < 32; c++) {
        fprintf(fp, "| %-*s ", (int)col_widths[c], headers[c]);
    }
    fprintf(fp, "|\n");

    /* Print separator */
    for (size_t c = 0; c < num_cols && c < 32; c++) {
        fprintf(fp, "+-");
        for (size_t w = 0; w < col_widths[c]; w++)
            fprintf(fp, "-");
        fprintf(fp, "-");
    }
    fprintf(fp, "+\n");

    /* Print rows */
    for (size_t r = 0; r < num_rows; r++) {
        for (size_t c = 0; c < num_cols && c < 32; c++) {
            fprintf(fp, "| %-*s ", (int)col_widths[c], rows[r][c]);
        }
        fprintf(fp, "|\n");
    }

    /* Print bottom border */
    for (size_t c = 0; c < num_cols && c < 32; c++) {
        fprintf(fp, "+-");
        for (size_t w = 0; w < col_widths[c]; w++)
            fprintf(fp, "-");
        fprintf(fp, "-");
    }
    fprintf(fp, "+\n");
}
