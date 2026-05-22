#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strophe.h>

#include "log.h"
#include "nwws_state.h"
#include "xmpp_ping.h"

static unsigned int ping_interval_ms = 300000; /* default: 5 minutes */
static unsigned int ping_seq = 0;
static bool awaiting_pong = false;
static char pending_ping_id[32] = {0};

void xmpp_ping_reset_state(void)
{
    ping_seq = 0;
    awaiting_pong = false;
    pending_ping_id[0] = '\0';
}

void xmpp_ping_configure_interval(void)
{
    const char *interval_env = getenv("NWWS_PING_INTERVAL_MINUTES");
    char *endptr;
    unsigned long minutes;

    if (interval_env == NULL || *interval_env == '\0') {
        return;
    }

    minutes = strtoul(interval_env, &endptr, 10);
    if (*endptr != '\0' || minutes == 0 || minutes > 1440) {
        log_warn("Invalid NWWS_PING_INTERVAL_MINUTES='%s'; using default %u minutes.",
                 interval_env, ping_interval_ms / 60000);
        return;
    }

    ping_interval_ms = (unsigned int)(minutes * 60000);
    log_info("Configured XMPP ping interval: %lu minute(s).", minutes);
}

unsigned int xmpp_ping_interval_ms(void)
{
    return ping_interval_ms;
}

static int handle_server_ping_request(xmpp_conn_t * const conn,
                                      xmpp_stanza_t * const stanza,
                                      xmpp_ctx_t * const ctx)
{
    xmpp_stanza_t *reply;
    const char *id = xmpp_stanza_get_id(stanza);
    const char *from = xmpp_stanza_get_attribute(stanza, "from");

    reply = xmpp_iq_new(ctx, "result", id);
    if (reply == NULL) {
        log_warn("Failed to create IQ result for incoming XMPP ping.");
        return 1;
    }

    if (from != NULL) {
        xmpp_stanza_set_to(reply, from);
    }

    xmpp_send(conn, reply);
    xmpp_stanza_release(reply);
    log_debug("Replied to server XMPP ping. id=%s", id != NULL ? id : "(null)");

    return 1;
}

int xmpp_ping_stanza_handler(xmpp_conn_t * const conn,
                             xmpp_stanza_t * const stanza,
                             void * const userdata)
{
    xmpp_ctx_t *ctx = (xmpp_ctx_t *)userdata;
    const char *type = xmpp_stanza_get_type(stanza);
    const char *id = xmpp_stanza_get_id(stanza);
    xmpp_stanza_t *ping_child;

    if (type == NULL) {
        return 1;
    }

    ping_child = xmpp_stanza_get_child_by_name(stanza, "ping");

    if (strcmp(type, "get") == 0 && ping_child != NULL) {
        const char *ns = xmpp_stanza_get_ns(ping_child);
        if (ns != NULL && strcmp(ns, "urn:xmpp:ping") == 0) {
            return handle_server_ping_request(conn, stanza, ctx);
        }
    }

    if (strcmp(type, "result") == 0 && id != NULL && awaiting_pong) {
        if (strcmp(id, pending_ping_id) == 0) {
            awaiting_pong = false;
            pending_ping_id[0] = '\0';
            log_debug("Received XMPP pong from server. id=%s", id);
        }
    }

    return 1;
}

int xmpp_ping_timed_handler(xmpp_conn_t * const conn, void * const userdata)
{
    xmpp_ctx_t *ctx = (xmpp_ctx_t *)userdata;
    xmpp_stanza_t *iq;
    xmpp_stanza_t *ping;
    char id[32];

    if (g_nwws_state.exit_requested == 1) {
        return 0;
    }

    if (awaiting_pong) {
        log_warn("No XMPP pong received for id=%s. Requesting reconnect.",
                 pending_ping_id);
        g_nwws_state.reconnect_requested = 1;
        return 1;
    }

    snprintf(id, sizeof(id), "nwws-ping-%u", ++ping_seq);

    iq = xmpp_iq_new(ctx, "get", id);
    if (iq == NULL) {
        log_warn("Failed to create IQ stanza for XMPP ping.");
        return 1;
    }

    ping = xmpp_stanza_new(ctx);
    if (ping == NULL) {
        xmpp_stanza_release(iq);
        log_warn("Failed to create ping stanza.");
        return 1;
    }

    xmpp_stanza_set_name(ping, "ping");
    xmpp_stanza_set_ns(ping, "urn:xmpp:ping");
    xmpp_stanza_add_child(iq, ping);
    xmpp_stanza_release(ping);

    xmpp_send(conn, iq);
    xmpp_stanza_release(iq);

    awaiting_pong = true;
    snprintf(pending_ping_id, sizeof(pending_ping_id), "%s", id);
    log_debug("Sent XMPP ping (XEP-0199). id=%s interval_ms=%u",
              pending_ping_id, ping_interval_ms);

    return 1;
}
