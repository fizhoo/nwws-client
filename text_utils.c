/** @file text_utils.c
 *
 * @brief Text utility functions used by NWWS client.
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

#include <stddef.h>
#include <string.h>

int chomp(char *input, size_t *output_length)
{
    char *src;
    char *dest;
    size_t payload_length;

    if (input == NULL) {
        if (output_length != NULL) {
            *output_length = 0;
        }
        return -1;
    }

    if (output_length != NULL) {
        *output_length = 0;
    }

    payload_length = strlen(input);
    src = input;
    dest = input;

    while (((size_t)(src - input) < payload_length) && (*src != '\0')) {
        while ((*src == '\n') && (*(src + 1) == '\n')) {
            *dest = *src;
            dest++;
            src += 2;
        }
        *dest = *src;
        dest++;
        src++;
    }

    *dest = '\0';

    if (output_length != NULL) {
        *output_length = (size_t)(dest - input);
    }

    return 0;
}
