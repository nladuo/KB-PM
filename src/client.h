#ifndef __CLIENT_H
#define __CLIENT_H

#include "process.h"

#ifndef LOCAL_SOCKET_NAME
#define LOCAL_SOCKET_NAME "/tmp/kb_pm.sock"
#endif /*LOCAL_SOCKET_NAME*/

#ifndef BUFFER_SIZE
#define BUFFER_SIZE 40 * 1024
#endif /*BUFFER_SIZE*/

extern void start_process(process_s *process);
extern void stop_process(process_s *process);
extern void remove_process(const char* app_name);
extern void start_all(void);
extern void stop_all(void);
extern void show_status(void);

#endif /*__CLIENT_H*/
