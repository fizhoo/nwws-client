/*
 * alarm_timer.c
 *
 * MIT License
 *
 * Copyright (c) 2022 W. Adam Young
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

#include <stdio.h>
#include <stdlib.h>

#include "alarm_timer.h"
#include "log.h"
#include "nwws_state.h"

static unsigned int healthcheck_interval_ms = 180000; /* 180 seconds */

unsigned int alarm_interval_ms(void)
{
    const char *interval_env = getenv("NWWS_ALARM_INTERVAL_SECONDS");
    char *endptr;
    unsigned long seconds;

    if (interval_env == NULL || *interval_env == '\0') {
        return healthcheck_interval_ms;
    }

    seconds = strtoul(interval_env, &endptr, 10);
    if (*endptr != '\0' || seconds == 0 || seconds > 86400) {
        log_warn("Invalid NWWS_ALARM_INTERVAL_SECONDS='%s'; using default 180 seconds.",
                 interval_env);
        return healthcheck_interval_ms;
    }

    healthcheck_interval_ms = (unsigned int)(seconds * 1000);
    log_info("Configured health-check timer interval: %lu second(s).", seconds);
    return healthcheck_interval_ms;
}

int alarm_timed_handler(xmpp_conn_t *conn, void *userdata)
{
    (void)conn;
    (void)userdata;

    if (g_nwws_state.exit_requested == 1) {
        return 0;
    }

    if (g_nwws_state.data_received == 0) {
        FILE *tmp_alarm = fopen("/tmp/nwws_alarm.txt", "w");
        if (tmp_alarm != NULL) {
            fclose(tmp_alarm);
        }

        g_nwws_state.reconnect_requested = 1;
    }

    g_nwws_state.data_received = 0;
    return 1;
}
