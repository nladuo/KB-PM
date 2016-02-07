#include "process.h"

void exec_process(process_s *process, int* res)
{
    *res = execl("/bin/bash", "bash", "-c", process->cmd, (char*)0);
}

void kill_process(process_s *process, int* res)
{
    *res = kill(process->pid, SIGKILL);
}

void get_process_list_count(process_s process_list[], int* count)
{
    int i;
    for(i = 0; i < LIST_SIZE; i++){
        if(!strlen(process_list[i].app_name)) break;
    }
    *count = i;
}

int parse_process_list(const char* path, process_s process_list[])
{
    FILE *fp;
    int i,index = 0;
    fp = fopen(path, "r");
    process_s *process;
    do{
        process = &process_list[index];
        process->status.restart_times = 0;/*reset the app start times in startup.*/
        index++;
    }while(3 == fscanf(fp, "%s%s%d", process->app_name, process->cmd, &process->is_running));

    for(i = 0; i < index - 1; i++){
        printf("appname:%s cmd:%s is_running:%d\n\n", process_list[i].app_name, process_list[i].cmd, process_list[i].is_running);
    }

    /* return the count of process */
    return index - 1;
}

int del_process_by_app_name(process_s process_list[], char* app_name)
{
    int i,index,count;
    index = -1;
    /* find the index of app_name */
    get_process_list_count(process_list, &count);
    for(i = 0; i < count; i++){
        if(!strcmp(process_list[i].app_name, app_name)){
            index = i;
        }
    }
    if(index == -1){
        return 0;
    }
    /* remove the process */
    for(i = index; i < count; i++){
        process_list[i] = process_list[i + 1];
    }

    return 1;
}
void save_process_list(const char* path, process_s process_list[])
{
    FILE *fp;
    int i,count;
    process_s* process;
    fp = fopen(path, "w");

    /*save the processes*/
    get_process_list_count(process_list, &count);
    for(i = 0; i < count; i++){
        process = &process_list[i];
        fprintf(fp, "%s %s %d\n", process->app_name, process->cmd, process->is_running);
    }
}
