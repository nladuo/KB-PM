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

#include <unistd.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/param.h>
#include <sys/syslog.h>
#include <sys/un.h>
#include <sys/socket.h>
#include <stdio.h>
#include <errno.h>
#include <dirent.h>
#include <fcntl.h>
#include <pthread.h>
#include <semaphore.h>
#include <signal.h>
#include "process.h"
#include "server.h"
#include "kbpm.h"

/*daemonize the process*/
void init_daemon(void);

/*ignore some signals except SIGKILL, SIGSTOP*/
void ignore_signals(void);

/*handle the client request*/
void *server_socket_handle_func(void *);

/*if the "~/.kbpm/process.config" does not exsit, create it.*/
void create_config_file();

/*send 'pong' to client and read the request data of client into buffer.*/
void pong_and_receive_data(int client_sockfd, char* buffer);

/*get the "~/.kbpm/process.config" path*/
void get_config_path(char* config_path);

/*get the "~/.kbpm" path*/
void get_config_dir(char* config_dir);

/*check out if the buffer is number.*/
int isNumber(char *buffer);

/*get the process id by checkout buffer is number and in the range of processlist*/
int get_process_id(char *buffer);

/*parse the process out and return the type(TYPE_APP_NAME, TYPE_ID, TYPE_CMD, TYPE_ERR)*/
int parse_process_and_process_type(char *buffer, process_s *process);

/*start a process and get the reponse to client*/
void server_start_process_and_get_response(char *buffer, char* response);

/*stop a process and get the reponse to client*/
void server_stop_process_and_get_response(char *buffer, char* response);

/*restart a process and get the reponse to client*/
void server_restart_process_and_get_response(char *buffer, char* response);

/*remove a process and get the reponse to client*/
void server_remove_process_and_get_response(char *buffer, char* response);

/*start a process and add to the tail of process_list*/
void server_start_process_with_cmd(process_s *process, char* response);

/*start a process in process list*/
void server_start_process(process_s *process, int with_log);

/*stop a process in process list*/
void server_stop_process(process_s *process);

/*restart a process in process list*/
void server_restart_process(process_s *process);

/*remove a process in process list*/
void server_remove_process(process_s *process, char* app_name);

/*get process list info and status.*/
void server_get_processes_status(char* buffer);

/*start all processes and listening to them exit.*/
void server_start_all_process(void);

/*stop all processes that server listening.*/
void server_stop_all_process(void);

/*get error response by buffer*/
void get_error_reponse(char *buffer, char *reponse);

/*return the position in process list*/
int get_pos_in_process_list(process_s *process);

/*return the process_list status to client.*/
void get_process_list_status(char *response);

/*get the quantity of running process in process_list.*/
int get_running_process_count();

/*need mutex when access process_list*/
pthread_mutex_t process_list_mutex;

/*the listening process list.*/
process_s process_list[LIST_SIZE];

/*the quantity of process in process_list.*/
int process_count;
/*
 * start the KB_PM service, and run in daemon.
 */
void service_start(void)
{
    int i, res;

    char *home_dir;
    pid_t pid;
    process_s *process;
    pthread_t socket_server_thread;
    char config_path[STR_BUFFER_SIZE];

    /*if first run the program, the config file will be not exist, create it.*/
    create_config_file();

    /*get process_list when service start up.*/
    get_config_path(config_path);
    process_count = parse_process_list_from_path(config_path, process_list);
    if (process_count == -1)/*if process_count is -1, there are not jspn file*/
    {
        process_count = 0;
    }

    /*daemonize the process*/
    init_daemon();

    /*init mutex*/
    res = pthread_mutex_init(&process_list_mutex, NULL);
    if(res != 0)
    {
        syslog(LOG_ERR, "Mutex initialization failed, error: %s", strerror(errno));
        syslog(LOG_ERR, "server unexpected exit.");
        exit(EXIT_FAILURE);
    }

    /*create thread to handle client request*/
    res = pthread_create(&socket_server_thread, NULL, server_socket_handle_func, NULL);
    if(res != 0)
    {
        syslog(LOG_ERR, "Thread creating failed, error: %s", strerror(errno));
        syslog(LOG_ERR, "server unexpected exit.");
        exit(EXIT_FAILURE);
    }

    /*exec the processes*/
    for(i = 0; i < process_count; i++)
    {
        process = &process_list[i];
        process->id = i;
        if(!process->is_running)
        {
            continue;
        }
        server_start_process(process, 1);
    }

    /*listening the child process exit.*/
    while(1)
    {
        /*if no running process, there are no child process,
            so wait(NULL) will return -1.*/
        if(!process_count || !get_running_process_count())
        {
            /*sleep 100 ms*/
            usleep(100 * 1000);
        }
        pid = wait(NULL);
        pthread_mutex_lock(&process_list_mutex);
        if (pid != -1)
        {
            syslog(LOG_INFO, "pid: %d be killed", pid);
        }
        for(i = 0; i < process_count; i++)
        {
            process = &process_list[i];
            if(process->pid == pid)
            {
                if(!process->is_running)
                {
                    break;
                }
                server_start_process(process, 0);/*without syslog*/
                process->restart_times++;
                syslog(LOG_INFO, "Restarting %s with pid:%d",process->app_name, process->pid);
                usleep(100 * 1000);
            }
        }
        pthread_mutex_unlock(&process_list_mutex);
    }
}

/*delete the /tmp/kbpm.sock is okay.*/
void service_clean(void)
{
    unlink(LOCAL_SOCKET_FILE);
    exit(EXIT_SUCCESS);
}

/*a new thread that handle client msg*/
void *server_socket_handle_func(void *args)
{
    int server_sockfd, client_sockfd;
    struct sockaddr_un server_addr, client_addr;
    int server_len, client_len;
    char buffer[BUFFER_SIZE];
    char response[BUFFER_SIZE];
    char config_path[STR_BUFFER_SIZE];

    get_config_path(config_path);

    /*delete the former sock file.*/
    unlink(LOCAL_SOCKET_FILE);

    /*create socket.*/
    server_sockfd = socket(AF_UNIX, SOCK_STREAM, 0);
    server_addr.sun_family = AF_UNIX;
    strcpy(server_addr.sun_path, LOCAL_SOCKET_FILE);
    server_len = sizeof(server_addr);

    /*bind and listen.*/
    bind(server_sockfd, (struct sockaddr*)&server_addr, server_len);
    listen(server_sockfd, SERVER_ACCEPT_COUNT);

    /*loop, accept the client connection.*/
    while(1)
    {
        client_len = sizeof(client_addr);
        client_sockfd = accept(server_sockfd, (struct sockaddr*)&client_addr, &client_len);
        pthread_mutex_lock(&process_list_mutex);

        read(client_sockfd, buffer, STR_BUFFER_SIZE);
        /*check out the command*/
        if(strcmp(buffer, "startall") == 0)
        {
            /*command startall*/
            server_start_all_process();
            strcpy(response, "Start All Okay.");
        }
        else if(strcmp(buffer, "stopall") == 0)
        {
            /*command stopall*/
            server_stop_all_process();
            strcpy(response, "Stop All Okay.");
        }
        else if(strcmp(buffer, "status") == 0)
        {
            /*command status*/
            get_process_list_status(response);
        }
        else if(strcmp(buffer, "start") == 0)
        {
            /*command start  <app_name|cmd|id>*/
            pong_and_receive_data(client_sockfd,buffer);
            server_start_process_and_get_response(buffer, response);
        }
        else if(strcmp(buffer, "stop") == 0)
        {
            /*command stop  <app_name|id>*/
            pong_and_receive_data(client_sockfd,buffer);
            server_stop_process_and_get_response(buffer, response);
        }
        else if(strcmp(buffer, "restart") == 0)
        {
            /*command restart <app_name|id>*/
            pong_and_receive_data(client_sockfd,buffer);
            server_restart_process_and_get_response(buffer, response);
        }
        else if(strcmp(buffer, "remove") == 0)
        {
            /*command remove <app_name|app_id>*/
            pong_and_receive_data(client_sockfd,buffer);
            server_remove_process_and_get_response(buffer, response);
        }
        else if(strcmp(buffer, "ping") == 0)
        {
            /*client ping server.*/
            strcpy(response, "pong");
        }
        else
        {
            strcpy(response, "error command");
        }
        write(client_sockfd, response, strlen(response));
        close(client_sockfd);
        save_process_list(config_path, process_list);
        memset(buffer, '\0', sizeof(buffer));
        memset(response, '\0', sizeof(response));
        pthread_mutex_unlock(&process_list_mutex);
    }
}

void pong_and_receive_data(int client_sockfd, char* buffer)
{
    write(client_sockfd, "pong", strlen("pong"));
    memset(buffer, '\0', sizeof(buffer));
    /*get the appname*/
    read(client_sockfd, buffer, STR_BUFFER_SIZE);
}

void get_config_path(char* config_path)
{
    /*get config path according to $HOME*/
    sprintf(config_path, "%s%s", getenv("HOME"), CONFIG_PATH);
}

void get_config_dir(char* config_dir)
{
    /*get config dir according to $HOME*/
    sprintf(config_dir, "%s%s", getenv("HOME"), CONFIG_DIR);
}

int isNumber(char *buffer)
{
    int i;
    for (i = 0; i < strlen(buffer); i++)
    {
        if (!(buffer[i] <= '9' && buffer[i] >= '0'))
        {
            return 0;
        }
    }
    return 1;
}

int get_process_id(char *buffer)
{
    int id;
    if (isNumber(buffer))
    {
        id = atoi((const char*)buffer);
        /*syslog(LOG_INFO, "id --> %d.",id);*/
        if (id >= 0 && id < process_count)
        {
            return id;
        }
    }
    return -1;
}

int parse_process_and_process_type(char *buffer, process_s *process)
{
    int id, i;
    /*check out TYPE_ID.*/
    if ((id = get_process_id(buffer)) != -1)
    {
        *process = process_list[id];
        return TYPE_ID;
    }

    /*check out TYPE_APP_NAME.*/
    for(i = 0; i < process_count; i++)
    {
        if(strcmp(process_list[i].app_name, buffer) == 0)
        {
            *process = process_list[i];
            return TYPE_APP_NAME;
        }
    }

    /*check out TYPE_CMD.*/
    if(parse_process(process, buffer) == -1)
    {
        return TYPE_ERR;
    }
    return TYPE_CMD;
}

int get_pos_in_process_list(process_s *process)
{
    int i;
    for (i = 0; i < process_count; i++)
    {
        if (strcmp(process_list[i].app_name, process->app_name) == 0)
        {
            return i;
        }
    }
}

void get_error_reponse(char *buffer, char *response)
{
    if(isNumber(buffer))
    {
        sprintf(response, "Cannot find id = %s", buffer);
    }
    else
    {
        sprintf(response, "Cannot find an app named : %s", buffer);
    }
}

void server_start_process_and_get_response(char *buffer, char* response)
{
    int i;
    int type;
    int pos;
    process_s process;

    type = parse_process_and_process_type(buffer, &process);
    
    switch(type)
    {
    case TYPE_CMD:
        /*check out duplication of app_name*/
        for (i = 0; i < process_count; i++)
        {
            if (strcmp(process.app_name, process_list[i].app_name) == 0)
            {
                sprintf(response, "Duplicated name : %s. Please rename the program.", process.app_name);
                return;
            }
        }
        server_start_process_with_cmd(&process, response);
        break;
    case TYPE_ID:
    case TYPE_APP_NAME:
        if (!process.is_running)
        {
            pos = get_pos_in_process_list(&process);
            server_start_process(&process_list[pos], 1);
            sprintf(response, "Started %s with pid:%d.",process.app_name, process_list[pos].pid);
        }
        else
        {
            sprintf(response, "%s have already been running.", process.app_name);
        }

        break;
    default:
        get_error_reponse(buffer, response);
        break;
    }

}

void server_stop_process_and_get_response(char *buffer, char* response)
{
    int type;
    int pos;
    process_s process;

    type = parse_process_and_process_type(buffer, &process);
    switch(type)
    {
    case TYPE_ID:
    case TYPE_APP_NAME:
        if (process.is_running)
        {
            pos = get_pos_in_process_list(&process);
            server_stop_process(&process_list[pos]);
            sprintf(response, "Stopped %s success.",process.app_name);
        }
        else
        {
            sprintf(response, "%s is not running.", process.app_name);
        }
        break;
    default:
        get_error_reponse(buffer, response);
        break;
    }
}

void server_restart_process_and_get_response(char *buffer, char* response)
{
    int type;
    int pos;
    process_s process;

    type = parse_process_and_process_type(buffer, &process);
    switch(type)
    {
    case TYPE_ID:
    case TYPE_APP_NAME:
        if (process.is_running)
        {
            pos = get_pos_in_process_list(&process);
            server_restart_process(&process_list[pos]);
            sprintf(response, "Restarted %s with %d.",process.app_name, process_list[pos].pid);
        }
        else
        {
            sprintf(response, "%s is not running.", process.app_name);
        }
        break;
    default:
        get_error_reponse(buffer, response);
        break;
    }
}



void server_remove_process_and_get_response(char *buffer, char* response)
{
    int type;
    int pos;
    process_s process;
    char app_name[STR_BUFFER_SIZE];

    type = parse_process_and_process_type(buffer, &process);
    switch(type)
    {
    case TYPE_ID:
    case TYPE_APP_NAME:
        pos = get_pos_in_process_list(&process);
        server_remove_process(&process_list[pos], app_name);
        if (process.is_running)
        {
            sprintf(response, "Stopped And Removed %s.",app_name);
        }
        else
        {
            sprintf(response, "Removed %s.", app_name);
        }
        break;
    default:
        get_error_reponse(buffer, response);
        break;
    }
}

void server_start_process_with_cmd(process_s *process, char* response)
{
    server_start_process(process, 1);
    /*add process at tail of process_list*/
    process->id = process_count;
    process_list[process_count] = *process;
    process_count++;
    sprintf(response, "Starting %s with pid:%d.",process->app_name, process->pid);
}

void server_start_process(process_s *process, int with_log)
{
    int res;
    pid_t pid;

    pid = fork();
    if(pid < 0)
    {
        syslog(LOG_ERR, "cannot fork(), error: %s", strerror(errno));
        syslog(LOG_ERR, "server unexpected exit.");
        unlink(LOCAL_SOCKET_FILE);
        exit(EXIT_FAILURE);
    }
    else if(pid == 0)/*the child process*/
    {
        ignore_signals();
        /*exec the child process.*/
        exec_process(process, &res);
        syslog(LOG_ERR, "%s unexpected exit: %s.", process->app_name, strerror(errno));
        exit(EXIT_FAILURE);
    }
    else/*the parent process*/
    {
        process->pid = pid;
        process->is_running = 1;
        time(&process->start_time);
        if(with_log)
        {
            syslog(LOG_INFO, "Starting %s with pid:%d.",process->app_name, process->pid);
        }
    }
}

void server_stop_process(process_s *process)
{
    int res;

    if(process->is_running)
    {
        process->is_running = 0;
        kill_process(process, &res);
        process->pid = 0;
        process->start_time = 0;
        process->restart_times = 0;
        syslog(LOG_INFO, "Stopping %s.",process->app_name);
    }
}

void server_restart_process(process_s *process)
{
    server_stop_process(process);
    server_start_process(process, 1);
    process->restart_times++;
}

void server_remove_process(process_s *process, char* app_name)
{
    server_stop_process(process);
    syslog(LOG_INFO, "Removing %s.",process->app_name);

    strcpy(app_name, process->app_name);
    /*delete the process form list.*/
    del_process_by_app_name(process_list, process->app_name);
    process_count--;

}

void server_start_all_process(void)
{
    int i;
    process_s *process;

    for(i = 0; i < process_count; i++)
    {
        process = &process_list[i];
        if(process->is_running)
        {
            continue;
        }
        server_start_process(process, 1);
    }
}

void server_stop_all_process(void)
{
    int i;
    process_s *process;

    for(i = 0; i < process_count; i++)
    {
        process = &process_list[i];
        if(process->is_running)
        {
            server_stop_process(process);
        }
    }
}

void get_process_list_status(char *response)
{
    create_process_list_json_str_with_status(process_list, process_count, response);
}

int get_running_process_count()
{
    int i, running_prcess_count = 0;
    process_s *process;
    for(i = 0; i < process_count; i++)
    {
        process = &process_list[i];
        if(process->is_running)
        {
            running_prcess_count++;
        }
    }
    return running_prcess_count;
}


void create_config_file(void)
{
    int status;
    char *home_path;
    char config_path[STR_BUFFER_SIZE];
    char config_dir[STR_BUFFER_SIZE];

    /*check out $HOME in system env.*/
    home_path = getenv("HOME");
    if (!home_path)
    {
        fprintf(stderr, "You have to set the env:$HOME\n");
        exit(EXIT_FAILURE);
    }

    get_config_path(config_path);
    get_config_dir(config_dir);

    /*check out if the config file exsit.*/
    if(access(config_path, F_OK) == -1)
    {
        /*create dir.*/
        status = mkdir(config_dir, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
        /*create file.`*/
        status = open(config_path, O_CREAT, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH);
    }
}

void ignore_signals(void)
{
    signal(SIGTTOU, SIG_IGN);
    signal(SIGTTIN, SIG_IGN);
    signal(SIGTSTP, SIG_IGN);
    signal(SIGHUP, SIG_IGN);
}

void init_daemon(void)
{
    pid_t pid;
    int i;

    /*ignore some signals*/
    ignore_signals();

    pid = fork();
    if(pid > 0)
    {
        exit(EXIT_SUCCESS);
    }
    else if(pid < 0)
    {
        perror("can not fork(), error:");
        exit(EXIT_FAILURE);
    }

    if(setsid() < 0)
    {
        perror("can not setsid(), error:");
        exit(EXIT_FAILURE);
    }

    pid = fork();
    if(pid > 0)
    {
        exit(EXIT_SUCCESS);
    }
    else if(pid < 0)
    {
        perror("can not fork(), error:");
        exit(EXIT_FAILURE);
    }

    /*close stdout, stdin, stderr*/
    for(i = 0; i < NOFILE; i++)
    {
        close(i);
    }

    chdir("/tmp");
    umask(0);

    openlog("KB_PM", LOG_PID, LOG_DAEMON);
}
