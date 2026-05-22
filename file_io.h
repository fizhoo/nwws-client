#ifndef FILE_IO_H
#define FILE_IO_H

/**
 * @file file_io.h
 * @brief Header file for file_io.h
 *
 * @author W. Adam Young
 *
 * @date 2/6/2024
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
#include <sys/stat.h>

#define MAX_FILENAME_LENGTH 64
#define MAX_PATH_LENGTH 128

#define FILE_PERMS (S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)  // Read/write for owner, read for group/others 0644
#define DIR_PERMS (S_IRUSR | S_IWUSR | S_IXUSR | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH) //Read/write/execute for owner, read/execute group/oth, 0755

/**
 *
 * @brief Write NWWS bulletin to a file
 *
 * write_data accepts the var "payload" null-terminanted string
 * string along with the NWS station and bulletin naming
 * attributes. It uses all of the information to create a unique
 * file name in a directory. All bulletins are saved in their
 * own directory based on "cccc" param. (i.e. cccc = KFWD).
 *
 * @param data Null-terminated string of the extracted NWS bulletin
 * @param cccc Four character issuing center (i.e. KFWD, KICT, etc)
 * @param awipsid The six character AWIPS ID, sometimes called AFOS PIL See https://www.weather.gov/tg/awips
 * @param ttaaii The six character WMO product ID. See https://www.weather.gov/tg/headef
 * @param id Unique id. See https://www.weather.gov/nwws/configuration.
 * @return int 
 *
 */
int write_data(char *,const char *, const char *, const char *, const char *);

#endif
