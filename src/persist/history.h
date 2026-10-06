#ifndef SUBNETCALC_PERSIST_HISTORY_H
#define SUBNETCALC_PERSIST_HISTORY_H

#include <stddef.h>
#include <stdbool.h>

#define MAX_HISTORY_ENTRIES 100
#define HISTORY_ENTRY_MAX_LEN 128

void history_init(void);
void history_add(const wchar_t *entry);
void history_clear(void);
size_t history_get_count(void);
const wchar_t *history_get_at(size_t index);

#endif /* SUBNETCALC_PERSIST_HISTORY_H */
