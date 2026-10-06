#ifndef SUBNETCALC_ENGINE_EXPORT_H
#define SUBNETCALC_ENGINE_EXPORT_H

#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>

void export_sanitize_csv_cell(const char *in_val, char *out_val, size_t out_len);
void export_write_csv_row(FILE *fp, const char *const *cells, size_t num_cells);
void export_write_ascii_table(FILE *fp, const char *const *headers, size_t num_cols,
                              const char *const *const *rows, size_t num_rows);

#endif /* SUBNETCALC_ENGINE_EXPORT_H */
