/**
 * @file xmpp_ping.h
 * @brief Declares XMPP ping keepalive interfaces.
 *
 * This header declares the public interface for configuring, resetting, and
 * handling XMPP ping state. The timed handler sends XEP-0199 ping requests, and
 * the stanza handler processes incoming ping requests and ping responses.
 *
 * @author W. Adam Young
 * @date 2026-07-03
 *
 * @par MIT License
 *
 * Copyright (c) 2026 W. Adam Young
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
 */

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
