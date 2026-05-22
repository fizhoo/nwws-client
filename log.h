#ifndef LOG_H
#define LOG_H

#include <stdarg.h>

typedef enum {
    LOG_LEVEL_ERROR = 0,
    LOG_LEVEL_WARN = 1,
    LOG_LEVEL_INFO = 2,
    LOG_LEVEL_DEBUG = 3
} log_level_t;

void log_init(void);
void log_set_level(log_level_t level);
log_level_t log_get_level(void);

void log_message(log_level_t level, const char *fmt, ...)
    __attribute__((format(printf, 2, 3)));

#define log_error(...) log_message(LOG_LEVEL_ERROR, __VA_ARGS__)
#define log_warn(...) log_message(LOG_LEVEL_WARN, __VA_ARGS__)
#define log_info(...) log_message(LOG_LEVEL_INFO, __VA_ARGS__)
#define log_debug(...) log_message(LOG_LEVEL_DEBUG, __VA_ARGS__)

#endif
