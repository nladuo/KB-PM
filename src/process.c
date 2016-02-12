#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>
#include <sys/types.h>
#include <fcntl.h>
#include <signal.h>
#include <string.h>
#include "process.h"
#include "../lib/cJSON.h"

int parse_process_cJSON(process_s *process, cJSON *p_json);

void create_process_cJSON(process_s *process, cJSON* p_json);

/*execute a process*/
void exec_process(process_s *process, int* res)
{
    chdir(process->dir);
    *res = execl("/bin/bash", "bash", "-c", process->cmd, (char*)0);
}

/*kill a process*/
void kill_process(process_s *process, int* res)
{
    *res = kill(process->pid, SIGKILL);
}

/*save the process_list count into int pointer "count"*/
void get_process_list_count(process_s process_list[], int* count)
{
    int i;
    for(i = 0; i < LIST_SIZE; i++){
        if(!strlen(process_list[i].app_name)) break;
    }
    *count = i;
}

/*
 * return process_list count.
 */
int parse_process_list_from_path(const char* path, process_s process_list[])
{
    int in_d, res;
    char buffer[BUFFER_SIZE];
    in_d = open(path, O_RDONLY);
    if(read(in_d, buffer, BUFFER_SIZE) <=0){
        return -1;
    }
    res = parse_process_list(process_list, buffer);

    return res;
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



int parse_process_list(process_s process_list[LIST_SIZE], char* json_str)
{
    cJSON *p_json_array, *p_json;
    process_s *process;
    int size, res, i;
    if(NULL == json_str){
        return -1;
    }
    p_json_array = cJSON_Parse(json_str);
    if(NULL == p_json_array){
        return -1;
    }
    size = cJSON_GetArraySize(p_json_array);
    for(i = 0; i < size; i++){
        p_json = cJSON_GetArrayItem(p_json_array, i);
        process = &process_list[i];
        res = parse_process_cJSON(process, p_json);
        if(res == -1){/*if parse err occur, free memory and return*/
            cJSON_Delete(p_json_array);
            return -1;
        }
    }
    cJSON_Delete(p_json_array);
    return size;
}

int parse_process(process_s *process, char* json_str)
{
    cJSON* p_json;
    int res;
    if(NULL == json_str){
        return -1;
    }
    p_json = cJSON_Parse(json_str);
    if(NULL == p_json){
        return -1;
    }
    res = parse_process_cJSON(process, p_json);
    cJSON_Delete(p_json);
    return res;
}

int parse_process_cJSON(process_s *process, cJSON *p_json)
{

    /*get app_name*/
    cJSON* p_json_item = cJSON_GetObjectItem(p_json, "app_name");
    if(NULL == p_json_item){
        return -1;
    }
    strcpy(process->app_name, p_json_item->valuestring);
    /*get cmd*/
    p_json_item = cJSON_GetObjectItem(p_json, "cmd");
    if(NULL == p_json_item){
        return -1;
    }
    strcpy(process->cmd, p_json_item->valuestring);

    /*get dir*/
    p_json_item = cJSON_GetObjectItem(p_json, "dir");
    if(NULL == p_json_item){
        return -1;
    }
    strcpy(process->dir, p_json_item->valuestring);


    /*get is_running*/
    p_json_item = cJSON_GetObjectItem(p_json, "is_running");
    if(NULL == p_json_item){
        return -1;
    }
    process->is_running = p_json_item->valueint;
}

int create_process_list_json_str(process_s process_list[LIST_SIZE], int list_count, char* json_str)
{
    cJSON *p_json_array, *p_json;
    process_s *process;
    char* str;
    int i;
    p_json_array = cJSON_CreateArray();
    if(NULL == p_json_array){
        return -1;
    }
    for(i = 0; i < list_count; i++){
        process = &process_list[i];
        p_json = cJSON_CreateObject();
        if(NULL == p_json){
            cJSON_Delete(p_json_array);
            return -1;
        }
        create_process_cJSON(process, p_json);
        cJSON_AddItemToArray(p_json_array, p_json);
    }

    str = cJSON_Print(p_json_array);
    if(NULL == str){
        cJSON_Delete(p_json_array);
        return -1;
    }
    strcpy(json_str, str);
    cJSON_Delete(p_json_array);
    return 0;
}

int create_process_json_str(process_s *process, char* json_str)
{
    cJSON* p_json;
    char* str = NULL;
    p_json = cJSON_CreateObject();
    if(NULL == p_json){
        return -1;
    }
    create_process_cJSON(process, p_json);

    str = cJSON_Print(p_json);

    if(NULL == str){
        cJSON_Delete(p_json);
        return -1;
    }

    strcpy(json_str, str);
    cJSON_Delete(p_json);
    return 0;
}

void create_process_cJSON(process_s *process, cJSON* p_json)
{
    cJSON_AddStringToObject(p_json, "app_name", process->app_name);
    cJSON_AddStringToObject(p_json, "cmd", process->cmd);
    cJSON_AddStringToObject(p_json, "dir", process->dir);
    cJSON_AddNumberToObject(p_json, "is_running", process->is_running);
}
