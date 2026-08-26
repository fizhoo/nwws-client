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
#include <time.h>
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

#ifndef PATH_MAX
#define PATH_MAX 4096
#endif

#ifndef NAME_MAX
#define NAME_MAX 255
#endif

/* CHANGE THIS TO YOUR LOCAL DIRECTORY */
static const char data_dir[] = "/path/to/save_location/";

//int safe_fopen(filename?)

static void lowercase_string(char *s)
{
    if (s == NULL) {
        return;
    }

    for (; *s != '\0'; s++) {
        *s = (char)tolower((unsigned char)*s);
    }
}

int write_data(const char *data, size_t data_length, const char *cccc, const char *awipsid, const char *ttaaii, const char *id)
{
    /* file_name is the actual name of the file
     * dir_path is the directory path inclduing airport code (cccc)
     * full_path_name is the directory path (dir_path) + file name
     */

    char center_dir[5]= {'\0'}, file_name[NAME_MAX + 1]= {'\0'}, dir_path[PATH_MAX]= {'\0'}, full_path_name[PATH_MAX]= {'\0'};

    FILE *dump;
    int n;
  //  int file=0;

    if(data==NULL || cccc==NULL || awipsid==NULL || ttaaii==NULL || id==NULL) /*Need to do additional error checking maybe */
        return -1;

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

    n = snprintf(dir_path, sizeof dir_path, "%s%s/", data_dir, center_dir);
    if (n < 0 || (size_t)n >= sizeof dir_path) {
        return -1;
    }

    n = snprintf(full_path_name, sizeof full_path_name, "%s%s", dir_path, file_name);
    if (n < 0 || (size_t)n >= sizeof full_path_name) {
        return -1;
    }

    /* Create file first without checking if path is existing.
     * If path is not there, then create path.
     * Slight speed advantage after all directories have been made
     * by incoming data. I.e. it saves a path check on every piece
     * of incoming data
     */

/* Open the file for writing, create directory if necessary */
    int fd = open(full_path_name, O_CREAT | O_WRONLY | O_EXCL, FILE_PERMS);
    if (fd == -1) {
        if (errno == ENOENT) {
            /* Directory doesn't exist, create it */
            if (mkdir(dir_path, DIR_PERMS) == -1 && errno != EEXIST) {
                perror("Error creating issuing center directory");
                return -1;
            }
            /* Retry opening the file after creating the directory */
            fd = open(full_path_name, O_CREAT | O_WRONLY | O_EXCL, FILE_PERMS);
            if (fd == -1) {
                perror("Error opening file after creating directory");
                return -1;
            }
        } else {
            perror("Error opening file");
            return -1;
        }
    }

    dump = fdopen(fd, "w");
    if (dump == NULL) {
        close(fd);
        return -1;
        //Do error checking here
    }
    if (fwrite(data, 1, data_length, dump) != data_length) {
        fclose(dump);
        return -1;
    }

    fclose(dump);
    g_nwws_state.data_received = 1; //if we are here, we received valid data

    return 0;
}
