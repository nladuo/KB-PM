#ifndef __CLIENT_H
#define __CLIENT_H

#include "process.h"

extern void start_process(process_s *process);
extern void stop_process(process_s *process);
extern void remove_process(const char* app_name);
extern void start_all(void);
extern void stop_all(void);
extern void show_status(void);

#endif /*__CLIENT_H*/
