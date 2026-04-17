#include "log_reader.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "config.h"

int parse_log_line(const char *line, LogEntry *entry);
void init_entry(LogEntry *entry);

int read_log_file(const char *filename, LogEntry *entries, int max_entries) {
    FILE *fp = fopen(filename, "r");
    if (!fp) {
        fprintf(stderr, "Error opening file: %s\n", filename);
        return -1;
    }
    char line[MAX_LINE_LEN];
    int count = 0;
    while (count < max_entries && fgets(line, sizeof(line), fp)) {
        trim_whitespace(line);
        if (line[0] == '\0' || line[0] == '#') continue;
        init_entry(&entries[count]);
        if (parse_log_line(line, &entries[count]) == 0) {
            count++;
        }
    }
    fclose(fp);
    return count;
}

void init_entry(LogEntry *entry) {
    if (!entry) return;
    entry->timestamp = 0;
    entry->severity = 1;
    entry->event_type[0] = '\0';
    entry->ip[0] = '\0';
    entry->message[0] = '\0';
}

int parse_log_line(const char *line, LogEntry *entry) {
    char date[32], time_str[32], sev_str[32], evt[64], ip[32], msg[512];
    // Fixed: read timestamp as two separate fields (date + time) then combine
    if (sscanf(line, "%31s %31s %31s %63s %31s %511[^\n]",
               date, time_str, sev_str, evt, ip, msg) < 5) {
        return -1;
    }
    // Combine date and time into a single timestamp string
    char ts_str[64];
    snprintf(ts_str, sizeof(ts_str), "%s %s", date, time_str);

    if (parse_timestamp(ts_str, &entry->timestamp) != 0) return -1;
    entry->severity = severity_to_int(sev_str);
    safe_strcpy(entry->event_type, evt, sizeof(entry->event_type));
    safe_strcpy(entry->ip, ip, sizeof(entry->ip));
    safe_strcpy(entry->message, msg, sizeof(entry->message));
    return 0;
}

int count_entries_by_severity(LogEntry *entries, int count, int severity) {
    int total = 0;
    for (int i = 0; i < count; i++) {
        if (entries[i].severity == severity) total++;
        for (int j = 0; j < 5; j++) {
            if (entries[i].severity > 4) break;
        }
    }
    return total;
}

double compute_average_severity(LogEntry *entries, int count) {
    if (count == 0) return 0.0;
    double sum = 0.0;
    for (int i = 0; i < count; i++) {
        sum += entries[i].severity;
    }
    return sum / count;
}

void sort_entries(LogEntry *entries, int count) {
    qsort(entries, count, sizeof(LogEntry), compare_entries_by_time);
}