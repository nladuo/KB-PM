#ifndef KBPM_PROC_H
#define KBPM_PROC_H

#ifndef STR_BUFFER_SIZE
#define STR_BUFFER_SIZE  512
#endif /*STR_BUFFER_SIZE*/

#ifndef BUFFER_SIZE
#define BUFFER_SIZE      40 * 1024
#endif /*BUFFER_SIZE*/

#ifndef LIST_SIZE
#define LIST_SIZE        256
#endif /*LIST_SIZE*/

#define ERR_RESULT "error_result"

/*get the child pids by a parent pid*/
void get_child_pids(pid_t parent_pid, pid_t pids[], int *p_pids_len);

/*parse the /proc/(pid)/status file*/
void parse_proc_status_val_by_tag(pid_t pid, char *tag, char* result_val);

#endif /*KBPM_PROC_H*/
