#define _POSIX_C_SOURCE 200809L

/**
 * @file file_io.c
 * @brief Handles file operations of writing the
 * NWWS-OI bulletin to invidual files.
 *
 * @author W. Adam Young
 *
 * @date 2/1/2024
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

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <fcntl.h>
#include <limits.h>

#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>
#include <unistd.h>

#include "file_io.h"
#include "nwws_state.h"

#ifndef NAME_MAX
#define NAME_MAX 255
#endif

#define CCCC_LENGTH 4
#define TTAAII_LENGTH 6
#define AWIPSID_MIN_LENGTH 4
#define AWIPSID_MAX_LENGTH 6
#define NWWS_ID_MAX_LENGTH 128

/* CHANGE THIS TO YOUR LOCAL DIRECTORY */
static const char data_dir[] = "/path/to/save_location";

static void lowercase_string(char *s)
{
    if (s == NULL) {
        return;
    }

    for (; *s != '\0'; s++) {
        *s = (char)tolower((unsigned char)*s);
    }
}

static int ascii_alphanumeric(int c)
{
    return (c >= 'A' && c <= 'Z') ||
           (c >= 'a' && c <= 'z') ||
           (c >= '0' && c <= '9');
}

static int valid_alphanumeric_field(const char *value,
                                    size_t minimum_length,
                                    size_t maximum_length)
{
    size_t length = 0;

    if (value == NULL) {
        return 0;
    }

    while (value[length] != '\0') {
        if (length >= maximum_length ||
            !ascii_alphanumeric((unsigned char)value[length])) {
            return 0;
        }
        length++;
    }

    return length >= minimum_length;
}

static int valid_nwws_id(const char *id)
{
    size_t length = 0;

    if (id == NULL) {
        return 0;
    }

    while (id[length] != '\0') {
        unsigned char character = (unsigned char)id[length];

        if (length >= NWWS_ID_MAX_LENGTH ||
            (!ascii_alphanumeric(character) && character != '.' &&
             character != '_' && character != '-')) {
            return 0;
        }
        if (character == '.' && length > 0 && id[length - 1] == '.') {
            return 0;
        }
        length++;
    }

    return length > 0;
}

static int valid_bulletin_attributes(const char *cccc,
                                     const char *awipsid,
                                     const char *ttaaii,
                                     const char *id)
{
    return valid_alphanumeric_field(cccc, CCCC_LENGTH, CCCC_LENGTH) &&
           valid_alphanumeric_field(ttaaii, TTAAII_LENGTH, TTAAII_LENGTH) &&
           valid_alphanumeric_field(awipsid, AWIPSID_MIN_LENGTH,
                                    AWIPSID_MAX_LENGTH) &&
           valid_nwws_id(id);
}

int write_data(const char *data, size_t data_length, const char *cccc, const char *awipsid, const char *ttaaii, const char *id)
{
    char center_dir[CCCC_LENGTH + 1] = {'\0'};
    char file_name[NAME_MAX + 1] = {'\0'};

    FILE *dump;
    int center_fd;
    int data_dir_fd;
    int fd;
    int n;
    int saved_errno;

    if (data == NULL ||
        !valid_bulletin_attributes(cccc, awipsid, ttaaii, id)) {
        errno = EINVAL;
        return -1;
    }

    /* Build file name  */
    n = snprintf(center_dir, sizeof center_dir, "%s", cccc);
    if (n < 0 || (size_t)n >= sizeof center_dir) {
        return -1;
    }

    n = snprintf(file_name, sizeof file_name, "%s-%s_%s-%s.txt", cccc, ttaaii, awipsid, id);
    if (n < 0 || (size_t)n >= sizeof file_name) {
        return -1;
    }

    lowercase_string(center_dir);
    lowercase_string(file_name);

    data_dir_fd = open(data_dir, O_RDONLY | O_DIRECTORY | O_NOFOLLOW | O_CLOEXEC);
    if (data_dir_fd == -1) {
        saved_errno = errno;
        perror("Error opening bulletin data directory");
        errno = saved_errno;
        return -1;
    }

    if (mkdirat(data_dir_fd, center_dir, DIR_PERMS) == -1 && errno != EEXIST) {
        saved_errno = errno;
        perror("Error creating issuing center directory");
        close(data_dir_fd);
        errno = saved_errno;
        return -1;
    }

    center_fd = openat(data_dir_fd, center_dir,
                       O_RDONLY | O_DIRECTORY | O_NOFOLLOW | O_CLOEXEC);
    if (center_fd == -1) {
        saved_errno = errno;
        close(data_dir_fd);
        errno = saved_errno;
        perror("Error opening issuing center directory");
        errno = saved_errno;
        return -1;
    }
    close(data_dir_fd);

    fd = openat(center_fd, file_name,
                O_CREAT | O_WRONLY | O_EXCL | O_NOFOLLOW | O_CLOEXEC,
                FILE_PERMS);
    if (fd == -1) {
        saved_errno = errno;
        close(center_fd);
        errno = saved_errno;
        perror("Error opening bulletin file");
        errno = saved_errno;
        return -1;
    }

    dump = fdopen(fd, "w");
    if (dump == NULL) {
        saved_errno = errno;
        close(fd);
        unlinkat(center_fd, file_name, 0);
        close(center_fd);
        errno = saved_errno;
        return -1;
    }
    errno = 0;
    if (fwrite(data, 1, data_length, dump) != data_length) {
        saved_errno = errno != 0 ? errno : EIO;
        fclose(dump);
        unlinkat(center_fd, file_name, 0);
        close(center_fd);
        errno = saved_errno;
        return -1;
    }

    errno = 0;
    if (fclose(dump) == EOF) {
        saved_errno = errno != 0 ? errno : EIO;
        unlinkat(center_fd, file_name, 0);
        close(center_fd);
        errno = saved_errno;
        return -1;
    }

    close(center_fd);
    g_nwws_state.data_received = 1; //if we are here, we received valid data

    return 0;
}
