#ifndef __SERVER_H
#define __SERVER_H
#include "process.h"

#define SERVER_ACCEPT_COUNT 5
/*config_path would be ~/.kbpm/process_list.json*/
#define CONFIG_PATH "/.kbpm/process_list.json"
/*config_dir would be ~/.kbpm */
#define CONFIG_DIR  "/.kbpm"

#ifndef LOCAL_SOCKET_NAME
#define LOCAL_SOCKET_NAME "/tmp/kbpm.sock"
#endif /*LOCAL_SOCKET_NAME*/

#ifndef STR_BUFFER_SIZE
#define STR_BUFFER_SIZE 4 * 1024
#endif/*STR_BUFFER_SIZE*/

#ifndef BUFFER_SIZE
#define BUFFER_SIZE 40 * 1024
#endif /*BUFFER_SIZE*/

/*start the KB_PM service, let it run in background.*/
extern void service_start(void);

/*create the startup shell of KB_PM, and let it auto start the service when operating system startup.*/
extern void service_enable(const char* os_type);

#endif /*__SERVER_H*/
