#ifndef __CLIENT_H
#define __CLIENT_H

#include "process.h"

#ifndef LOCAL_SOCKET_NAME
#define LOCAL_SOCKET_NAME "/tmp/kbpm.sock"
#endif /*LOCAL_SOCKET_NAME*/

#ifndef BUFFER_SIZE
#define BUFFER_SIZE 40 * 1024
#endif /*BUFFER_SIZE*/

/*call request to start a process by cmd.*/
extern void start_process(const char *cmd);

/*call request to stop a process by app name.*/
extern void stop_process(const char *app_name);

extern void remove_process(const char* app_name);

extern void start_all(void);

extern void stop_all(void);

extern void show_status(void);

extern void ping_server(int* res);

#endif /*__CLIENT_H*/
