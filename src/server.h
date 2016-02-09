#ifndef __SERVER_H
#define __SERVER_H
#include "process.h"

#define SERVER_ACCEPT_COUNT 5
#define CONFIG_PATH "./test.config"

#ifndef LOCAL_SOCKET_NAME
#define LOCAL_SOCKET_NAME "/tmp/kb_pm.sock"
#endif /*LOCAL_SOCKET_NAME*/

#ifndef BUFFER_SIZE
#define BUFFER_SIZE 40 * 1024
#endif /*BUFFER_SIZE*/

/*start the KB_PM service, let it run in background.*/
extern void service_start(void);

/*create the startup shell of KB_PM, and let it auto start the service when operating system startup.*/
extern void service_enable(const char* os_type);

#endif /*__SERVER_H*/
