/**
 * @file xmpp_connect.c
 *
 * @brief This file contains a set of XMPP functions that handle
 * connections and connection handling, messange handling.
 *
 * Basis of this file based on libstrophe examples located at:
 *
 * https://github.com/strophe/libstrophe/tree/master/examples
 *
 * @author Adam Young
 *
 * @date February 7, 2024
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

/* hardcoded TCP keepalive timeout and interval */
//#define KA_TIMEOUT 60
//#define KA_INTERVAL 1

#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <strophe.h> /* XMPP Library */

#include "alarm_timer.h"
#include "file_io.h"
#include "log.h"
#include "text_utils.h"
#include "nwws_client.h"
#include "xmpp_ping.h"
#include "xmpp_connect.h"

static const char *host = "nwws-oi.weather.gov";
static bool message_handler_added = false;
static const unsigned int reconnect_delay_seconds = 5;
static bool ping_handler_added = false;
static bool ping_stanza_handler_added = false;
static bool alarm_handler_added = false;

static int message_handler(xmpp_conn_t * const conn,
                           xmpp_stanza_t * const stanza,
                           void * const userdata);
static void conn_handler(xmpp_conn_t * const conn,
                         const xmpp_conn_event_t status,
                         const int error,
                         xmpp_stream_error_t * const stream_error,
                         void * const userdata);
static int nwws_connect_with_retry(xmpp_conn_t *conn, const char *host,
                                  unsigned short port, xmpp_ctx_t *ctx);
static void nwws_run_loop(xmpp_conn_t *conn, xmpp_ctx_t *ctx);
static void reset_connection_runtime_state(void);
static void run_retry_delay(xmpp_ctx_t *ctx, unsigned int delay_ms);

static int stanza_attributes_missing(const char *awipsid,
                                     const char *ttaaii,
                                     const char *cccc,
                                     const char *id)
{
    return awipsid == NULL || ttaaii == NULL || cccc == NULL || id == NULL ||
           (*awipsid == '\0') || (*ttaaii == '\0');
}

static void reset_connection_runtime_state(void)
{
    message_handler_added = false;
    ping_handler_added = false;
    ping_stanza_handler_added = false;
    alarm_handler_added = false;
    xmpp_ping_reset_state();
}

static void run_retry_delay(xmpp_ctx_t *ctx, unsigned int delay_ms)
{
    struct timespec start;
    struct timespec now;
    long long elapsed_ms;

    if (clock_gettime(CLOCK_MONOTONIC, &start) != 0) {
        sleep(reconnect_delay_seconds);
        return;
    }

    do {
        xmpp_run_once(ctx, 200);

        if (g_nwws_state.exit_requested == 1) {
            return;
        }

        if (clock_gettime(CLOCK_MONOTONIC, &now) != 0) {
            sleep(reconnect_delay_seconds);
            return;
        }

        elapsed_ms = (long long)(now.tv_sec - start.tv_sec) * 1000LL;
        elapsed_ms += (now.tv_nsec - start.tv_nsec) / 1000000LL;
    } while ((unsigned int)elapsed_ms < delay_ms);
}


/**
 *  @todo Maybe add a delay after an error to try again.
 *        Maybe require a loop with a configurable retry param
 * 
 *  @todo Move connection routines out of main. Why?
 * To separate the function calls from main.
*/

static int message_handler(xmpp_conn_t * const conn,
                           xmpp_stanza_t * const stanza,
                           void * const userdata)
{
    xmpp_ctx_t *ctx = (xmpp_ctx_t*)userdata;
    xmpp_stanza_t *body,*x;
    
    const char *awipsid, *cccc, *ttaaii, *id;
    char *payload, *bodytext;
    
    /* Get BODY of message */
    body = xmpp_stanza_get_child_by_name(stanza, "body");
    if (body == NULL)
        return 1;
	
    bodytext = xmpp_stanza_get_text(body);
    if (bodytext == NULL)
    {
		xmpp_free(ctx,bodytext);
		return 1;
	}
    
    /* Get the X child. X contains the actual
     * NWS bulletin */
     
    x = xmpp_stanza_get_child_by_name(stanza, "x");
    if (x == NULL) {
        xmpp_free(ctx,bodytext);
        return 1;
    }
    
    payload = xmpp_stanza_get_text(x);
    if(payload == NULL)
    {
		xmpp_free(ctx,bodytext);
		xmpp_free(ctx,payload);
		return 1;
	}

    awipsid=xmpp_stanza_get_attribute(x,"awipsid");
    cccc=xmpp_stanza_get_attribute(x,"cccc");
    id=xmpp_stanza_get_attribute(x,"id");
    ttaaii=xmpp_stanza_get_attribute(x,"ttaaii");
    
    /* If the AWIPS id or ttaaii is not provided,
     * then it is probably a test message and does not
     * need to be saved in a file */
     
    if (stanza_attributes_missing(awipsid, ttaaii, cccc, id))
    {
        xmpp_free(ctx,bodytext);
        xmpp_free(ctx,payload);
        return 1;
    }
    
    chomp(payload);
    
    /* Send payload (actual NWS bulletin) to file write */
    if ((write_data(payload,cccc,awipsid,ttaaii,id)) < 0)
    {
        log_warn("Writing NWWS bulletin to file failed.");
    }

    /* WHITESPACE KEEPALIVE - I believe the NWWS-OI server is using
     * Openfire and it has an issue of disc*/
     
    xmpp_send_raw_string(conn," \n");
    xmpp_free(ctx,bodytext);
    xmpp_free(ctx,payload);
    
    return 1; /*return 1 keeps the handler active, 0 removes handler */
}

/**
 *
 * name: nwws_connect_new
 * @param const char *jid
 * @param const char *pass
 * @return
 *
 * Need to pass jid,pass
 *
 */

int nwws_connect_new (const char *jid, const char *pass)
{
    xmpp_ctx_t *ctx;
    xmpp_conn_t *conn;
    xmpp_log_t *log;
    
    //char *jid, *pass;
    int tcp_keepalive = 1;
    
    /* init library */
    xmpp_initialize();
    xmpp_ping_configure_interval();
    
    /* create a context */
    log = xmpp_get_default_logger(XMPP_LEVEL_INFO); /* pass NULL instead to silence output */
    
    if((ctx = xmpp_ctx_new(NULL, log))==NULL) /* returns NULL if error */
    {
        log_error("Error in setting up Strophe context object. Quitting.");
        return 1;
    }
    
    /* create a connection */
    if((conn = xmpp_conn_new(ctx))==NULL)
    {
        log_error("ERROR: Setting up Strophe connection object failed. Quitting.");
        return 1;
    }
        /* configure connection properties (optional) */
/*   if((xmpp_conn_set_flags(conn, XMPP_CONN_FLAG_TRUST_TLS))!=XMPP_EOK) {
        fprintf(stderr,"ERROR: Can set connection flags. Quitting\n");
        return 1;
    }*/

    /* Disable Stream Management for fault-injection stability. */
    if ((xmpp_conn_set_flags(conn, XMPP_CONN_FLAG_DISABLE_SM)) != XMPP_EOK) {
        fprintf(stderr, "ERROR: Can set connection flags. Quitting\n");
        return 1;
    }
    
    /* configure TCP keepalive (optional)
     * Returns VOID */
     
    //  if (tcp_keepalive) xmpp_conn_set_keepalive(conn, KA_TIMEOUT, KA_INTERVAL);
    if (tcp_keepalive) {
        xmpp_conn_set_sockopt_callback(conn, xmpp_sockopt_cb_keepalive);
    }
    
    /* setup authentication information
     * Both functions below return VOID */
    xmpp_conn_set_jid(conn, jid);
    xmpp_conn_set_pass(conn, pass);
    
    /** xmpp_connect_client: initiate connection
     * @todo Maybe add a delay after an error to try again.
     * Maybe require a loop with a configurable retry param.
     *
     * @todo Move connection routines out of main. Why?
     * To separate the function calls from main */
     
    // Call nwws_connect (generic connect function)
    nwws_connect(conn, host, 5222, ctx);
    
    return 0;
}
static void conn_handler(xmpp_conn_t * const conn,
                         const xmpp_conn_event_t status,
                         const int error,
                         xmpp_stream_error_t * const stream_error,
                         void * const userdata)
{
    (void) error;
    (void) stream_error;
    
    xmpp_ctx_t *ctx = (xmpp_ctx_t *)userdata;

    /* Set up presence stanza */
    // xmpp_stanza_t* pres;
log_debug("status %d\n",status);
log_debug("reconnect req = %d\n",g_nwws_state.reconnect_requested);
    
    if (status == XMPP_CONN_CONNECT) {
        xmpp_stanza_t* pres;
        int secured;
        
        log_debug("connected");
        secured = xmpp_conn_is_secured(conn);
        g_nwws_state.reconnect_requested = 0;
        log_debug("connection is %s.", secured ? "secured" : "NOT secured");
                              
        // Check if the message handler has already been added
        if (!message_handler_added) {
            xmpp_handler_add(conn, message_handler, NULL, "message", NULL, ctx);
            message_handler_added = true;  // Mark that the handler has been added
        }

        if (!ping_handler_added) {
            xmpp_timed_handler_add(conn, xmpp_ping_timed_handler,
                                   xmpp_ping_interval_ms(), ctx);
            ping_handler_added = true;
        }

        if (!ping_stanza_handler_added) {
            xmpp_handler_add(conn, xmpp_ping_stanza_handler, NULL, "iq", NULL, ctx);
            ping_stanza_handler_added = true;
        }

        if (!alarm_handler_added) {
            xmpp_timed_handler_add(conn, alarm_timed_handler,
                                   alarm_interval_ms(), NULL);
            alarm_handler_added = true;
        }
        
        //  xmpp_handler_add(conn, message_handler, NULL, "message", NULL, ctx);
        pres = xmpp_presence_new(ctx);
        xmpp_stanza_set_to(pres,"nwws@conference.nwws-oi.weather.gov/young");
        xmpp_send(conn, pres);
        xmpp_stanza_release(pres);
    }
    else {
        log_debug("disconnected");
        if(g_nwws_state.exit_requested != 1) {
            reset_connection_runtime_state();
            g_nwws_state.reconnect_requested = 1;
        }
    }
}
int nwws_connect(xmpp_conn_t *conn, const char *host, unsigned short port,xmpp_ctx_t *ctx)
{
    if (nwws_connect_with_retry(conn, host, port, ctx) != 0) {
        return 1;
    }

    nwws_run_loop(conn, ctx);
    nwws_disconnect(conn, ctx);
    
    return 0;
}

int nwws_disconnect(xmpp_conn_t *conn, xmpp_ctx_t *ctx)
{
    /* release our connection and context */
    xmpp_disconnect(conn);

    /* 2. THE DRAIN: Let the library finish its internal free() calls.
       We run the loop until the state machine officially hits DISCONNECTED. */
    int limit = 100; // Safety cap to prevent infinite loops
    while (xmpp_conn_is_disconnected(conn) == 0 && --limit > 0) {
        xmpp_run_once(ctx, 10); 
    }
    
    xmpp_conn_release(conn);
    xmpp_ctx_free(ctx);
    
    /* final shutdown of the library */
    xmpp_shutdown();
    
    return 0;
}

static int nwws_connect_with_retry(xmpp_conn_t *conn, const char *host,
                                  unsigned short port, xmpp_ctx_t *ctx)
{
    int attempt = 0;
    const unsigned int retry_interval_ms = reconnect_delay_seconds * 1000;
    int rc;

    while (g_nwws_state.exit_requested != 1) {
        if (xmpp_conn_is_connected(conn)) {
            g_nwws_state.reconnect_requested = 0;
            return 0;
        }

        if (xmpp_conn_is_connecting(conn)) {
            xmpp_run_once(ctx, 1000);
            if (xmpp_conn_is_connected(conn)) {
                g_nwws_state.reconnect_requested = 0;
                return 0;
            }
            continue;
        }

        rc = xmpp_connect_client(conn, host, port, conn_handler, ctx);
        if (rc != XMPP_EOK) {
            attempt++;
            log_error("Can't connect to server (attempt %d). Retrying in %u seconds.",
                      attempt, reconnect_delay_seconds);
            run_retry_delay(ctx, retry_interval_ms);
            continue;
        }

        while (g_nwws_state.exit_requested != 1 &&
               xmpp_conn_is_connecting(conn)) {
            xmpp_run_once(ctx, 1000);
        }

        if (xmpp_conn_is_connected(conn)) {
            g_nwws_state.reconnect_requested = 0;
            return 0;
        }

        attempt++;
        log_error("Can't connect to server (attempt %d). Retrying in %u seconds.",
                  attempt, reconnect_delay_seconds);
        run_retry_delay(ctx, retry_interval_ms);
    }

    return 1;
}

static void nwws_run_loop(xmpp_conn_t *conn, xmpp_ctx_t *ctx)
{
    while (g_nwws_state.exit_requested != 1) {
        if (g_nwws_state.reconnect_requested == 1) {
            if (xmpp_conn_is_connected(conn)) {
                xmpp_disconnect(conn);
            } else if (xmpp_conn_is_disconnected(conn)) {
                if (nwws_connect_with_retry(conn, host, 5222, ctx) != 0) {
                    return;
                }
            }
        }
        xmpp_run_once(ctx, 1000);
    }
}
