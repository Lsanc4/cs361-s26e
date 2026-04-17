#ifndef PARSER_H
#define PARSER_H

#include "utils.h"
#include "config.h"

int parse_log_entries(const char *filename, LogEntry *entries, int max_entries);
int validate_entry(const LogEntry *entry);
int extract_event_type(const char *line, char *out, size_t out_size);
int extract_ip(const char *line, char *out, size_t out_size);
int extract_message(const char *line, char *out, size_t out_size);
int count_valid_entries(LogEntry *entries, int count);
void sanitize_entries(LogEntry *entries, int count);

#endif