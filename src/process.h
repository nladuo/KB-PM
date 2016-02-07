#ifndef __PROCESS_H
#define __PROCESS_H

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>
#include <sys/types.h>
#include <signal.h>
#include <string.h>

#define STR_BUFFER_SIZE 4096
#define LIST_SIZE       256

/*define the status struct.*/
typedef struct{
    char *memory;
    time_t uptime;
    int restart_times;
} status_s;

/*define the process struct.*/
typedef struct{
    char app_name[STR_BUFFER_SIZE];
    char cmd[STR_BUFFER_SIZE];
    pid_t pid;
    int is_running; /* 0 for false, 1 for true.*/
    status_s status;
}process_s;

/* exec a process by cmd in bash shell, and get the result by pointer.*/
extern void exec_process(process_s* process, int* res);

/* kill a process by process pid, and get the result by pointer.*/
extern void kill_process(process_s* process, int* res);

/*get the process runtime arguments.*/
extern void get_process_status(process_s* process, status_s* status);

/*parse the config file. */
extern int parse_process_list(const char* path, process_s process_list[]);

/*how many known process used KB_PM to manage.*/
extern void get_process_list_count(process_s process_list[], int* count);

/* delete a process in process list, if find the app, return 1, if did not, return 0. */
extern int del_process_by_app_name(process_s process_list[], char* app_name);

/*save the process in file.*/
extern void save_process_list(const char* path, process_s process_list[]);

#endif /*__PROCESS_H*/
