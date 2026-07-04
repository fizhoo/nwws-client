/**
 * @file log.c
 * @brief Implements lightweight logging for the NWWS client.
 *
 * This file provides log level configuration, environment-based log level
 * parsing, and formatted logging to standard error. The log level defaults to
 * debug builds using DEBUG and release builds using INFO, and may be configured
 * with the NWWS_LOG_LEVEL environment variable.
 *
 * @author W. Adam Young
 * @date 2024-02-07
 *
 * @par MIT License
 *
 * Copyright (c) 2024 W. Adam Young
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 */

#include "log.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static log_level_t g_log_level = LOG_LEVEL_INFO;

static const char *log_level_label(log_level_t level)
{
    switch (level) {
    case LOG_LEVEL_ERROR:
        return "ERROR";
    case LOG_LEVEL_WARN:
        return "WARN";
    case LOG_LEVEL_INFO:
        return "INFO";
    case LOG_LEVEL_DEBUG:
        return "DEBUG";
    default:
        return "UNKNOWN";
    }
}

static int level_from_env(const char *value, log_level_t *out_level)
{
    char buffer[16];
    size_t len;

    if (value == NULL || out_level == NULL) {
        return -1;
    }

    len = strlen(value);
    if (len == 0 || len >= sizeof(buffer)) {
        return -1;
    }

    for (size_t i = 0; i < len; i++) {
        buffer[i] = (char)tolower((unsigned char)value[i]);
    }
    buffer[len] = '\0';

    if (strcmp(buffer, "error") == 0) {
        *out_level = LOG_LEVEL_ERROR;
        return 0;
    }
    if (strcmp(buffer, "warn") == 0 || strcmp(buffer, "warning") == 0) {
        *out_level = LOG_LEVEL_WARN;
        return 0;
    }
    if (strcmp(buffer, "info") == 0) {
        *out_level = LOG_LEVEL_INFO;
        return 0;
    }
    if (strcmp(buffer, "debug") == 0) {
        *out_level = LOG_LEVEL_DEBUG;
        return 0;
    }

    return -1;
}

void log_init(void)
{
    log_level_t env_level;
    const char *env = getenv("NWWS_LOG_LEVEL");

#ifndef NDEBUG
    g_log_level = LOG_LEVEL_DEBUG;
#else
    g_log_level = LOG_LEVEL_INFO;
#endif

    if (level_from_env(env, &env_level) == 0) {
        g_log_level = env_level;
    }
}

void log_set_level(log_level_t level)
{
    g_log_level = level;
}

log_level_t log_get_level(void)
{
    return g_log_level;
}

void log_message(log_level_t level, const char *fmt, ...)
{
    va_list args;

    if (level > g_log_level) {
        return;
    }

    fprintf(stderr, "[%s] ", log_level_label(level));
    va_start(args, fmt);
    vfprintf(stderr, fmt, args);
    va_end(args);
    fputc('\n', stderr);
}
