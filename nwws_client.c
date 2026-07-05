#define _POSIX_C_SOURCE 200809L

/** @file nwws_client.c
 *
 * @brief This is the main program of nwws_client, which is used to receive
 * and store NOAA Weather Wire Service Open Interface (NWWS OI). For
 * more information, go to: https://www.weather.gov/nwws/
 * 
 * Basis of this file based on libstrophe examples located at:
 *
 * https://github.com/strophe/libstrophe/tree/master/examples
 * 
 * @author Adam Young
 *
 * @date June 8,2022
 *
 * @par MIT License
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

/* hardcoded TCP keepalive timeout and interval */
//#define KA_TIMEOUT 60
//#define KA_INTERVAL 1

#define DEBUG 0

#include <stdio.h>
#include <signal.h> /*Signal handler*/

#include "log.h"
#include "nwws_client.h"
#include "xmpp_connect.h"

nwws_state_t g_nwws_state = {0};

int main(int argc, char **argv)
{
    char *jid, *pass;

    log_init();
    sigSetup();

    /* take a jid and password on the command line
     * @todo: Read in from configuration file
     * */

    if (argc != 3) {
        log_error("Usage: %s <jid> <pass>", argv[0]);
        return 1;
    }

    jid = argv[1];
    pass = argv[2];

    /* enter the event loop */

    nwws_connect_new (jid,pass);

    return 0;
}

void sigSetup()
{
    struct sigaction sa = {0};
    struct sigaction sa_pipe = {0};

    sa.sa_handler=sigHandler;
    sigaction(SIGINT,&sa,NULL);
    sigaction(SIGTERM,&sa,NULL);

    sa_pipe.sa_handler = sigHandler;
    sigaction(SIGPIPE, &sa_pipe, NULL);
}

void sigHandler(int sig)
{
    if(sig==SIGINT)
    {
        printf("\nShutting down ...\n");
        g_nwws_state.exit_requested = 1;
    }
    if (sig==SIGTERM)
    {
        fprintf(stderr,"Terminating process from SIGTERM\n");
        g_nwws_state.exit_requested = 1;
    }
    if (sig==SIGPIPE)
    {
        g_nwws_state.reconnect_requested = 1;
    }
}
