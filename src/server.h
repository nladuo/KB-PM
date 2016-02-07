#ifndef __SERVER_H
#define __SERVER_H
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include "process.h"

#define CONFIG_PATH "./test.config"

/*start the KB_PM service, let it run in background.*/
extern void service_start(void);

/*create the startup shell of KB_PM, and let it auto start the service when operating system startup.*/
extern void service_enable(const char* os_type);

#endif /*__SERVER_H*/
