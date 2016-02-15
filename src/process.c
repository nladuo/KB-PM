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

int parse_process_cJSON_with_status(process_s *process, cJSON *p_json);

void create_process_cJSON(process_s *process, cJSON* p_json);

void create_process_cJSON_with_status(process_s *process, cJSON* p_json);

void get_mem_by_pid(pid_t pid, char* memory);

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
    for(i = 0; i < LIST_SIZE; i++)
    {
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
    if(read(in_d, buffer, BUFFER_SIZE) <=0)
    {
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
    for(i = 0; i < count; i++)
    {
        if(!strcmp(process_list[i].app_name, app_name))
        {
            index = i;
        }
    }
    if(index == -1)
    {
        return -1;
    }
    /* remove the process */
    for(i = index; i < count; i++)
    {
        process_list[i + 1].id--;
        process_list[i] = process_list[i + 1];
    }

    return 0;
}

void save_process_list(const char* path, process_s process_list[])
{
    FILE *fp;
    int i,count;
    char buffer[BUFFER_SIZE];
    process_s* process;
    fp = fopen(path, "w");

    /*save the processes*/
    get_process_list_count(process_list, &count);
    create_process_list_json_str(process_list, count, buffer);
    fprintf(fp, "%s", buffer);
    fflush(fp);
    fclose(fp);
}



int parse_process_list(process_s process_list[], char* json_str)
{
    cJSON *p_json_array, *p_json;
    process_s *process;
    int size, res, i;
    if(NULL == json_str)
    {
        return -1;
    }
    p_json_array = cJSON_Parse(json_str);
    if(NULL == p_json_array)
    {
        return -1;
    }
    size = cJSON_GetArraySize(p_json_array);
    for(i = 0; i < size; i++)
    {
        p_json = cJSON_GetArrayItem(p_json_array, i);
        process = &process_list[i];
        res = parse_process_cJSON(process, p_json);

        /*if parse err occur, free memory and return*/
        if(res == -1)
        {
            cJSON_Delete(p_json_array);
            return -1;
        }
    }
    cJSON_Delete(p_json_array);
    return size;
}

int parse_process_list_with_status(process_s process_list[], char* json_str)
{
    cJSON *p_json_array, *p_json;
    process_s *process;
    int size, res, i;
    if(NULL == json_str)
    {
        return -1;
    }
    p_json_array = cJSON_Parse(json_str);
    if(NULL == p_json_array)
    {
        return -1;
    }
    size = cJSON_GetArraySize(p_json_array);
    for(i = 0; i < size; i++)
    {
        p_json = cJSON_GetArrayItem(p_json_array, i);
        process = &process_list[i];
        res = parse_process_cJSON_with_status(process, p_json);

        /*if parse err occur, free memory and return*/
        if(res == -1)
        {
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
    if(NULL == json_str)
    {
        return -1;
    }
    p_json = cJSON_Parse(json_str);
    
    if(NULL == p_json)
    {
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
    if(NULL == p_json_item)
    {
        return -1;
    }
    strcpy(process->app_name, p_json_item->valuestring);

    /*get cmd*/
    p_json_item = cJSON_GetObjectItem(p_json, "cmd");
    if(NULL == p_json_item)
    {
        return -1;
    }
    strcpy(process->cmd, p_json_item->valuestring);

    /*get dir*/
    p_json_item = cJSON_GetObjectItem(p_json, "dir");
    if(NULL == p_json_item)
    {
        return -1;
    }
    strcpy(process->dir, p_json_item->valuestring);

    /*get is_running*/
    p_json_item = cJSON_GetObjectItem(p_json, "is_running");
    if(NULL == p_json_item)
    {
        return -1;
    }
    process->is_running = p_json_item->valueint;
}

int parse_process_cJSON_with_status(process_s *process, cJSON *p_json)
{

    /*get app_name*/
    cJSON* p_json_item = cJSON_GetObjectItem(p_json, "app_name");
    if(NULL == p_json_item)
    {
        return -1;
    }
    strcpy(process->app_name, p_json_item->valuestring);

    /*get id*/
    p_json_item = cJSON_GetObjectItem(p_json, "id");
    if(NULL == p_json_item)
    {
        return -1;
    }
    process->id = p_json_item->valueint;

    /*get pid*/
    p_json_item = cJSON_GetObjectItem(p_json, "pid");
    if(NULL == p_json_item)
    {
        return -1;
    }
    process->pid = p_json_item->valueint;

    /*get is_running*/
    p_json_item = cJSON_GetObjectItem(p_json, "is_running");
    if(NULL == p_json_item)
    {
        return -1;
    }
    process->is_running = p_json_item->valueint;

    /*get restart_times*/
    p_json_item = cJSON_GetObjectItem(p_json, "restart_times");
    if(NULL == p_json_item)
    {
        return -1;
    }
    process->restart_times = p_json_item->valueint;

    /*get start_time*/
    p_json_item = cJSON_GetObjectItem(p_json, "start_time");
    if(NULL == p_json_item)
    {
        return -1;
    }
    process->start_time = p_json_item->valueint;

    /*get memory*/
    p_json_item = cJSON_GetObjectItem(p_json, "memory");
    if(NULL == p_json_item)
    {
        return -1;
    }
    strcpy(process->memory, p_json_item->valuestring);
}

int create_process_list_json_str(process_s process_list[LIST_SIZE], int list_count, char* json_str)
{
    cJSON *p_json_array, *p_json;
    process_s *process;
    char* str;
    int i;
    p_json_array = cJSON_CreateArray();
    if(NULL == p_json_array)
    {
        return -1;
    }
    for(i = 0; i < list_count; i++)
    {
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

int create_process_list_json_str_with_status(process_s process_list[], int list_count, char* json_str)
{
    cJSON *p_json_array, *p_json;
    process_s *process;
    char* str;
    int i;
    p_json_array = cJSON_CreateArray();
    if(NULL == p_json_array)
    {
        return -1;
    }
    for(i = 0; i < list_count; i++)
    {
        process = &process_list[i];
        p_json = cJSON_CreateObject();
        if(NULL == p_json)
        {
            cJSON_Delete(p_json_array);
            return -1;
        }
        create_process_cJSON_with_status(process, p_json);
        cJSON_AddItemToArray(p_json_array, p_json);
    }

    str = cJSON_Print(p_json_array);
    if(NULL == str)
    {
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
    if(NULL == p_json)
    {
        return -1;
    }
    create_process_cJSON(process, p_json);

    str = cJSON_Print(p_json);

    if(NULL == str)
    {
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

void create_process_cJSON_with_status(process_s *process, cJSON* p_json)
{
    cJSON_AddStringToObject(p_json, "app_name", process->app_name);
    cJSON_AddNumberToObject(p_json, "id", process->id);
    cJSON_AddNumberToObject(p_json, "pid", process->pid);
    cJSON_AddNumberToObject(p_json, "restart_times", process->restart_times);
    cJSON_AddNumberToObject(p_json, "start_time", process->start_time);
    cJSON_AddNumberToObject(p_json, "is_running", process->is_running);
    /*get process memory*/
    if (process->is_running)
    {
        get_mem_by_pid(process->pid, process->memory);
        cJSON_AddStringToObject(p_json, "memory", process->memory);
    }
    else
    {
        cJSON_AddStringToObject(p_json, "memory", "0 B");
    }
    
}

void get_mem_by_pid(pid_t pid, char* memory)
{
    FILE *fp;
    int flag = 0;
    char buf[STR_BUFFER_SIZE];
    char path[STR_BUFFER_SIZE];
    char *p_buf;

    sprintf(path, "/proc/%d/status", pid);

    fp = fopen(path, "r");
    while(fgets(buf, STR_BUFFER_SIZE, fp) > 0)
    {
        if (strncmp(buf, "VmHWM:", 6) == 0)
        {
            /*the buf[strlen(buf)-1] should be '\n', replace it with '\0' */
            buf[strlen(buf)-1] = 0;

            flag = 1;
            p_buf = buf;
            while(!(*p_buf >= '0' && *p_buf <= '9')) p_buf++;
            strcpy(memory, p_buf);
            break;
        }
    }
    fclose(fp);
    if (!flag)
    {
        strcpy(memory, "cannot get memory");
    }
}