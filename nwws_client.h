#ifndef NWWS_CLIENT_H
#define NWWS_CLIENT_H

/** @file nwws_client.h
 *
 * @brief Header file for NWWS Client
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

#include "nwws_state.h"

/* Signal Handler Functions */


/**
 * @brief Signal set up function
 *
 * Function to set up signals for SIGINT and SIGTERM
 *
 * @return void
 *
 * @todo Error checking?
 */
void sigSetup(void);

/**
 * @brief Signal hanlding function
 *
 * Function to catch and handle signals for SIGINT and SIGTERM
 *
 * @return void
 *
 */
void sigHandler(int);

#endif
