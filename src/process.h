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

#ifndef KBPM_PROCESS_H
#define KBPM_PROCESS_H

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

#endif /*KBPM_PROCESS_H*/
