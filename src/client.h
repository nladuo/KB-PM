
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

#ifndef __CLIENT_H
#define __CLIENT_H

#include "process.h"

#ifndef LOCAL_SOCKET_NAME
#define LOCAL_SOCKET_NAME "/tmp/kbpm.sock"
#endif /*LOCAL_SOCKET_NAME*/

#ifndef BUFFER_SIZE
#define BUFFER_SIZE 40 * 1024
#endif /*BUFFER_SIZE*/

/*call request to start a process by app_name or cmd.*/
extern void start_process(const char *app_name_or_cmd);

/*call request to restart a process by app_name.*/
extern void restart_process(const char *app_name);

/*call request to stop a process by app name.*/
extern void stop_process(const char *app_name);

extern void remove_process(const char* app_name);

extern void start_all(void);

extern void stop_all(void);

extern void show_status(void);

extern void ping_server(int* res);

#endif /*__CLIENT_H*/
