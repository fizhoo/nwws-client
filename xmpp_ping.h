#ifndef XMPP_PING_H
#define XMPP_PING_H

#include <strophe.h>

void xmpp_ping_configure_interval(void);
unsigned int xmpp_ping_interval_ms(void);
void xmpp_ping_reset_state(void);
int xmpp_ping_timed_handler(xmpp_conn_t * const conn, void * const userdata);
int xmpp_ping_stanza_handler(xmpp_conn_t * const conn,
                             xmpp_stanza_t * const stanza,
                             void * const userdata);

#endif
