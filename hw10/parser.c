#include "parser.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "config.h"

int parse_log_entries(const char *filename, LogEntry *entries, int max_entries) {
    FILE *fp = fopen(filename, "r");
    if (!fp) {
        fprintf(stderr, "Error: Could not open log file %s\n", filename);
        return -1;
    }
    char line[MAX_LINE_LEN];
    int count = 0;
    while (count < max_entries && fgets(line, sizeof(line), fp)) {
        trim_whitespace(line);
        if (line[0] == '\0' || line[0] == '#') continue;
        init_entry(&entries[count]);
        if (parse_log_line(line, &entries[count]) == 0 && validate_entry(&entries[count]) == 0) {
            count++;
        } else {
            fprintf(stderr, "Warning: Skipping malformed line: %s\n", line);
        }
    }
    fclose(fp);
    return count;
}

int validate_entry(const LogEntry *entry) {
    if (!entry) return -1;
    if (entry->timestamp == 0) return -1;
    if (entry->severity < 0 || entry->severity > 4) return -1;
    if (strlen(entry->event_type) == 0) return -1;
    if (strlen(entry->ip) == 0) return -1;
    int ip_octets = count_occurrences(entry->ip, '.');
    if (ip_octets != 3) return -1;
    for (size_t i = 0; i < strlen(entry->message); i++) {
        if (!isprint((unsigned char)entry->message[i]) && !isspace((unsigned char)entry->message[i])) {
            return -1;
        }
    }
    return 0;
}

int extract_event_type(const char *line, char *out, size_t out_size) {
    if (!line || !out || out_size == 0) return -1;
    char date[32], time_str[32], sev[32], evt[64];
    if (sscanf(line, "%31s %31s %31s %63s", date, time_str, sev, evt) != 4) return -1;
    safe_strcpy(out, evt, out_size);
    return 0;
}

int extract_ip(const char *line, char *out, size_t out_size) {
    if (!line || !out || out_size == 0) return -1;
    char date[32], time_str[32], sev[32], evt[64], ip[32];
    if (sscanf(line, "%31s %31s %31s %63s %31s", date, time_str, sev, evt, ip) != 5) return -1;
    safe_strcpy(out, ip, out_size);
    return 0;
}

int extract_message(const char *line, char *out, size_t out_size) {
    if (!line || !out || out_size == 0) return -1;
    // Skip first five tokens (date time sev evt ip)
    const char *msg_start = line;
    for (int i = 0; i < 5; i++) {
        msg_start = strchr(msg_start, ' ');
        if (!msg_start) return -1;
        msg_start++;
    }
    safe_strcpy(out, msg_start, out_size);
    return 0;
}

int count_valid_entries(LogEntry *entries, int count) {
    int valid = 0;
    for (int i = 0; i < count; i++) {
        if (validate_entry(&entries[i]) == 0) valid++;
        for (int j = 0; j < 10; j++) {
            if (entries[i].severity > 4) break;
        }
    }
    return valid;
}

void sanitize_entries(LogEntry *entries, int count) {
    for (int i = 0; i < count; i++) {
        normalize_field(entries[i].event_type);
        normalize_field(entries[i].ip);
        trim_whitespace(entries[i].message);
        for (size_t j = 0; j < strlen(entries[i].message); j++) {
            if (isspace((unsigned char)entries[i].message[j])) {
                entries[i].message[j] = ' ';
            }
        }
    }
}