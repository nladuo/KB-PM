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

#ifndef KBPM_SERVER_H
#define KBPM_SERVER_H

#define SERVER_ACCEPT_COUNT 5
/*config_path would be ~/.kbpm/process_list.json*/
#define CONFIG_PATH "/.kbpm/process_list.json"
/*config_dir would be ~/.kbpm */
#define CONFIG_DIR  "/.kbpm"

#ifndef LOCAL_SOCKET_FILE
#define LOCAL_SOCKET_FILE "/tmp/kbpm.sock"
#endif /*LOCAL_SOCKET_FILE*/

#ifndef STR_BUFFER_SIZE
#define STR_BUFFER_SIZE 4 * 1024
#endif/*STR_BUFFER_SIZE*/

#ifndef BUFFER_SIZE
#define BUFFER_SIZE     40 * 1024
#endif /*BUFFER_SIZE*/

#define TYPE_APP_NAME   0
#define TYPE_ID         1
#define TYPE_CMD        2
#define TYPE_ERR        3


/*start the KB_PM service, let it run in background.*/
extern void service_start(void);

/*resolve the KB_PM service unexpectedly exit.*/
extern void service_clean(void);

#endif /*KBPM_SERVER_H*/
