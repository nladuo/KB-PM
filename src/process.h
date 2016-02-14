#ifndef __PROCESS_H
#define __PROCESS_H

#ifndef STR_BUFFER_SIZE
#define STR_BUFFER_SIZE  512
#endif /*STR_BUFFER_SIZE*/

#ifndef BUFFER_SIZE
#define BUFFER_SIZE      40 * 1024
#endif /*BUFFER_SIZE*/

#ifndef LIST_SIZE
#define LIST_SIZE        256
#endif /*LIST_SIZE*/

/*define the process struct.*/
typedef struct{
    char        app_name[STR_BUFFER_SIZE];
    char        cmd[STR_BUFFER_SIZE];
    char        dir[STR_BUFFER_SIZE];
    pid_t       pid;
    int         is_running; /* 0 for false, 1 for true.*/
    int         id;
    char        memory[STR_BUFFER_SIZE];
    int         restart_times;
    time_t      start_time;

}process_s;

/* exec a process by cmd in bash shell, and get the result by pointer.*/
extern void exec_process(process_s* process, int* res);

/* kill a process by process pid, and get the result by pointer.*/
extern void kill_process(process_s* process, int* res);


/*read json_str from path, parse it to process_list, and return the size of process_list.*/
extern int parse_process_list_from_path(const char *path, process_s process_list[]);

/*parse the json_str to process_list, and return the size of process_list.*/
extern int parse_process_list(process_s process_list[], char* json_str);

extern int parse_process_list_with_status(process_s process_list[], char* json_str);

/*parse the json_str to process.*/
extern int parse_process(process_s *process, char* json_str);

/*get the json_str from process_list.*/
extern int create_process_list_json_str(process_s process_list[], int list_count, char* json_str);

extern int create_process_list_json_str_with_status(process_s process_list[], int list_count, char* json_str);

/*get the json_str from process.*/
extern int create_process_json_str(process_s *process, char* json_str);

/*how many known process used KB_PM to manage.*/
extern void get_process_list_count(process_s process_list[], int* count);

/* delete a process in process list, if find the app, return 1, if did not, return 0. */
extern int del_process_by_app_name(process_s process_list[], char* app_name);

/*save the process in file.*/
extern void save_process_list(const char* path, process_s process_list[]);

#endif /*__PROCESS_H*/
