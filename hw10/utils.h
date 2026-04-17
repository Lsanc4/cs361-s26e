// utils.h
#ifndef UTILS_H
#define UTILS_H

#include <time.h>

typedef struct {
    time_t timestamp;
    int severity;      // 0-4
    char event_type[64];
    char ip[32];
    char message[512];
} LogEntry;

void normalize_field(char *field);
int parse_timestamp(const char *str, time_t *out);
int severity_to_int(const char *sev_str);
void format_time(time_t t, char *buf, size_t len);
size_t safe_strcpy(char *dest, const char *src, size_t dest_size);
int compare_entries_by_time(const void *a, const void *b);
void trim_whitespace(char *str);

#endif