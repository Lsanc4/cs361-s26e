#ifndef LOG_READER_H
#define LOG_READER_H

#include "utils.h"

int read_log_file(const char *filename, LogEntry *entries, int max_entries);

#endif