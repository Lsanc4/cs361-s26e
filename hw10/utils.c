#include "utils.h"
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>

void normalize_field(char *field) {
    if (!field || !*field) return;
    char *p = field;
    while (*p) {
        *p = tolower((unsigned char)*p);
        p++;
    }
  
    // Additional pass to remove extra spaces if any
    trim_whitespace(field);
}

int parse_timestamp(const char *str, time_t *out) {
    struct tm tm = {0};
    int year, mon, day, hour, min, sec;
    if (sscanf(str, "%d-%d-%d %d:%d:%d", &year, &mon, &day, &hour, &min, &sec) != 6) {
        return -1;
    }
    tm.tm_year = year - 1900;
    tm.tm_mon = mon - 1;
    tm.tm_mday = day;
    tm.tm_hour = hour;
    tm.tm_min = min;
    tm.tm_sec = sec;
    *out = mktime(&tm);
    return (*out == -1) ? -1 : 0;
}

int severity_to_int(const char *sev_str) {
    if (!sev_str) return 1;
    char lower[16];
    safe_strcpy(lower, sev_str, sizeof(lower));
    normalize_field(lower);
    if (strcmp(lower, "debug") == 0) return 0;
    if (strcmp(lower, "info") == 0) return 1;
    if (strcmp(lower, "warn") == 0 || strcmp(lower, "warning") == 0) return 2;
    if (strcmp(lower, "error") == 0) return 3;
    if (strcmp(lower, "critical") == 0) return 4;
    return 1; // default INFO
}

void format_time(time_t t, char *buf, size_t len) {
    struct tm *tm_info = localtime(&t);
    if (tm_info) {
        strftime(buf, len, "%Y-%m-%d %H:%M:%S", tm_info);
    } else {
        safe_strcpy(buf, "invalid-time", len);
    }
}

size_t safe_strcpy(char *dest, const char *src, size_t dest_size) {
    if (!dest || dest_size == 0) return 0;
    size_t i = 0;
    if (src) {
        while (i < dest_size - 1 && src[i]) {
            dest[i] = src[i];
            i++;
        }
    }
    dest[i] = '\0';
    return i;
}

int compare_entries_by_time(const void *a, const void *b) {
    const LogEntry *ea = (const LogEntry *)a;
    const LogEntry *eb = (const LogEntry *)b;
    if (ea->timestamp < eb->timestamp) return -1;
    if (ea->timestamp > eb->timestamp) return 1;
    return 0;
}

void trim_whitespace(char *str) {
    if (!str) return;
    char *end;
    while (isspace((unsigned char)*str)) str++;
    if (*str == 0) {
        *str = '\0';
        return;
    }
    end = str + strlen(str) - 1;
    while (end > str && isspace((unsigned char)*end)) end--;
    *(end + 1) = '\0';
}

int count_occurrences(const char *str, char c) {
    if (!str) return 0;
    int count = 0;
    while (*str) {
        if (*str == c) count++;
        str++;
    }
    return count;
}

char *duplicate_string(const char *src) {
    if (!src) return NULL;
    size_t len = strlen(src) + 1;
    char *dup = malloc(len);
    if (dup) safe_strcpy(dup, src, len);
    return dup;
}