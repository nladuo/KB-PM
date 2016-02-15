/*
  The MIT License (MIT)

  Copyright (c) 2016 Kalen Blue

  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in all
  copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
  SOFTWARE.
*/

#ifndef __BOX_DRAWING_H
#define __BOX_DRAWING_H
#include "process.h"

#define ANSI_COLOR_GRAY    "\x1b[30m"
#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_GREEN   "\x1b[32m"
#define ANSI_COLOR_CYAN    "\x1b[36m"
#define ANSI_COLOR_RESET   "\x1b[0m"
#define ANSI_FONT_BOLD     "\x1b[1m"
#define ANSI_FONT_FAINT    "\x1b[2m"
#define ANSI_FONT_ITALIC   "\x1b[3m"


#define RB "\e(0\x6a\e(B" /* 188 Right Bottom corner */
#define RT "\e(0\x6b\e(B" /* 187 Right Top corner */
#define LT "\e(0\x6c\e(B" /* 201 Left Top cornet */
#define LB "\e(0\x6d\e(B" /* 200 Left Bottom corner */
#define MC "\e(0\x6e\e(B" /* 206 Midle Cross */
#define HL "\e(0\x71\e(B" /* 205 Horizontal Line */
#define LC "\e(0\x74\e(B" /* 204 Left Cross */
#define RC "\e(0\x75\e(B" /* 185 Right Cross */
#define BC "\e(0\x76\e(B" /* 202 Bottom Cross */
#define TC "\e(0\x77\e(B" /* 203 Top Cross */
#define VL "\e(0\x78\e(B" /* 186 Vertical Line */

#ifndef LIST_SIZE
#define LIST_SIZE       256
#endif /*LIST_SIZE*/

#define VAR_NUM 7

#ifndef STR_BUFFER_SIZE
#define STR_BUFFER_SIZE 512
#endif /*STR_BUFFER_SIZE*/
/*
type struct {
    app_name    char[STR_BUFFER_SIZE],
    id          char[STR_BUFFER_SIZE],
    pid         char[STR_BUFFER_SIZE],
    status      char[STR_BUFFER_SIZE],
    restart     char[STR_BUFFER_SIZE],
    uptime      char[STR_BUFFER_SIZE],
    memory      char[STR_BUFFER_SIZE]
} box_line_s;
*/

/*print the process status box according to process list*/
extern void print_process_list_box(process_s process_list[], int list_count);

#endif /* __BOX_DRAWING_H */
