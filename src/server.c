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

/*start a process and listening its exit.*/
void server_start_process(process_s *process, char* response);

/*restart a process that server listening.*/
void server_restart_process(process_s *process, char* response);

/*stop a process that server listening.*/
void server_stop_process(process_s *process, char* response);

/*stop a process that server listening and remove from process list.*/
void server_remove_process(process_s *process, char* response);

/*get process list info and status.*/
void server_get_processes_status(char* buffer);

/*start all processes and listening to them exit.*/
void server_start_all_process(void);

/*stop all processes that server listening.*/
void server_stop_all_process(void);

/*return the process_list status to client.*/
void get_process_list_status(char *buffer);

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
    strcpy(config_path, getenv("HOME"));
    strcat(config_path, CONFIG_PATH);
    process_count = parse_process_list_from_path(config_path, process_list);
    if (process_count == -1)
    {
        process_count = 0;
    }

    /*daemonize the process*/
    init_daemon();

    /*init mutex*/
    res = pthread_mutex_init(&process_list_mutex, NULL);
    if(res != 0){
        perror("Mutex initialization failed.");
        exit(EXIT_FAILURE);
    }

    /*create thread to handle client request*/
    res = pthread_create(&socket_server_thread, NULL, server_socket_handle_func, NULL);
    if(res != 0){
        perror("Thread creating failed.");
        exit(EXIT_FAILURE);
    }

    /*exec the processes*/
    for(i = 0; i < process_count; i++){
        process = &process_list[i];
        process->id = i;
        if(!process->is_running){
            continue;
        }
        pid = fork();
        if(pid < 0){
            fprintf(stderr, "can not fork(), error: %s", strerror(errno));
            exit(EXIT_FAILURE);
        }else if(pid == 0){/*the child process*/
            ignore_signals();
            /*exec the child process.*/
            exec_process(process, &res);
            exit(EXIT_FAILURE);
        }else{
            process->pid = pid;
            time(&process->start_time);
            process->restart_times = 0;
            //printf("Starting %s with pid:%d",process->app_name, process->pid);
        }
    }

    /*listening the child process exit.*/
    while(1){
        if(!process_count || !get_running_process_count()){
            usleep(100 * 1000);/*sleep 100 ms*/
        }
        pid = wait(NULL);
        pthread_mutex_lock(&process_list_mutex);
        //printf("pid: %d be killed\n", pid);
        for(i = 0; i < process_count; i++){
            process = &process_list[i];
            if(process->pid == pid){
                if(!process->is_running){
                    break;
                }
                pid = fork();
                if(pid < 0){
                    fprintf(stderr, "can not fork(), error: %s", strerror(errno));
                    exit(EXIT_FAILURE);
                }else if(pid == 0){
                    //init_daemon();
                    ignore_signals();
                    exec_process(process, &res);
                    exit(EXIT_FAILURE);
                }else{
                    process->pid = pid;
                    process->restart_times++;
                    time(&process->start_time);
                    //printf("Restart app: %s dir:%s cmd:%s \n", process->app_name,process->dir, process->cmd);
                    break;
                }
            }
        }
        pthread_mutex_unlock(&process_list_mutex);
    }
}


/*
 * create the startup shell
 * @param: os_type: ubuntu,centos and so on...
 */
void service_enable(const char* os_type)
{

}

int get_running_process_count()
{
    int i, running_prcess_count = 0;
    process_s *process;
    for(i = 0; i < process_count; i++){
        process = &process_list[i];
        if(process->is_running){
            running_prcess_count++;
        }
    }
    return running_prcess_count;
}

void *server_socket_handle_func(void *args)
{
    int server_sockfd, client_sockfd;
    int server_len, client_len;
    int i, flag;
    char buffer[BUFFER_SIZE], response[BUFFER_SIZE];
    struct sockaddr_un server_addr, client_addr;
    process_s process;

    /*delete the former sock file.*/
    unlink(LOCAL_SOCKET_NAME);

    /*create socket.*/
    server_sockfd = socket(AF_UNIX, SOCK_STREAM, 0);
    server_addr.sun_family = AF_UNIX;
    strcpy(server_addr.sun_path, LOCAL_SOCKET_NAME);
    server_len = sizeof(server_addr);

    /*bind and listen.*/
    bind(server_sockfd, (struct sockaddr*)&server_addr, server_len);
    listen(server_sockfd, 5);

    /*loop, accept the client connection.*/
    while(1){
        client_len = sizeof(client_addr);
        client_sockfd = accept(server_sockfd, (struct sockaddr*)&client_addr, &client_len);
        read(client_sockfd, buffer, STR_BUFFER_SIZE);
        if(strcmp(buffer, "startall") == 0){
            /*command startall*/
            server_start_all_process();
            strcpy(response, "pong");
        }else if(strcmp(buffer, "stopall") == 0){
            /*command stopall*/
            server_stop_all_process();
            strcpy(response, "pong");
        }else if(strcmp(buffer, "status") == 0){
            /*command status*/
            get_process_list_status(response);
        }else if(strcmp(buffer, "start") == 0){
            /*command start <app_name|cmd|id>*/
            write(client_sockfd, "pong", strlen("pong"));
            memset(buffer, '\0', sizeof(buffer));
            read(client_sockfd, buffer, STR_BUFFER_SIZE);
            /*if parse error occur*/
            if(parse_process(&process, buffer) == -1){
                strcpy(buffer, "parse error");
                write(client_sockfd, buffer, strlen(buffer));
                goto GO_ON;
            }
            server_start_process(&process, response);
        }else if(strncmp(buffer, "stop", strlen("stop")) == 0){
            /*command stop <app_name|id>*/
            write(client_sockfd, "pong", strlen("pong"));
            memset(buffer, '\0', sizeof(buffer));
            /*get the appname*/
            read(client_sockfd, buffer, STR_BUFFER_SIZE);
            flag = 0;
            for(i = 0; i < process_count; i++){
                if(strcmp(process_list[i].app_name, buffer) == 0){
                    flag = 1;
                    break;
                }
            }
            if(!flag){/*flag == 0*/
                sprintf(response, "cannot find an app named:%s", buffer);
                write(client_sockfd, response, strlen(response));
                goto GO_ON;
            }
            server_stop_process(&process_list[i], response);
        }else if(strncmp(buffer, "restart", strlen("restart")) == 0){
            /*command restart <app_name|id>*/
            write(client_sockfd, "pong", strlen("pong"));
            memset(buffer, '\0', sizeof(buffer));
            /*get the appname*/
            read(client_sockfd, buffer, STR_BUFFER_SIZE);
            flag = 0;
            for(i = 0; i < process_count; i++){
                if(strcmp(process_list[i].app_name, buffer) == 0){
                    flag = 1;
                    break;
                }
            }
            if(!flag){/*flag == 0*/
                sprintf(response, "cannot find an app named:%s", buffer);
                write(client_sockfd, response, strlen(response));
                goto GO_ON;
            }
            server_restart_process(&process_list[i], response);
        }else if(strncmp(buffer, "remove", strlen("remove")) == 0){
            /*command remove <app_name|app_id>*/
            write(client_sockfd, "pong", strlen("pong"));
            memset(buffer, '\0', sizeof(buffer));
            /*get the appname*/
            read(client_sockfd, buffer, STR_BUFFER_SIZE);
            flag = 0;
            for(i = 0; i < process_count; i++){
                if(strcmp(process_list[i].app_name, buffer) == 0){
                    flag = 1;
                    break;
                }
            }
            if(!flag){/*flag == 0*/
                sprintf(response, "cannot find an app named:%s", buffer);
                write(client_sockfd, response, strlen(response));
                goto GO_ON;
            }
            server_remove_process(&process_list[i], response);

        }else if(strcmp(buffer, "ping") == 0){
            strcpy(response, "pong");
        }else{
            goto GO_ON;
        }
        write(client_sockfd, response, strlen(response));
GO_ON:
        close(client_sockfd);
        memset(buffer, '\0', sizeof(buffer));
        memset(response, '\0', sizeof(response));
        //usleep(100 * 1000);/*sleep 100 ms*/
    }
}

void server_start_process(process_s *process, char* response)
{
    int res, i;
    int flag;
    pid_t pid;
    char config_path[STR_BUFFER_SIZE];
    process_s *p;
    /*get config path according to $HOME*/
    strcpy(config_path, getenv("HOME"));
    strcat(config_path, CONFIG_PATH);
    /*if the length of process->cmd is 0.
     *  find the app name and start the former process*/
    if(!strlen(process->cmd)){
        flag = 0;
        for (i = 0; i < process_count; ++i){
            p = &process_list[i];
            if(strcmp(p->app_name, process->app_name) == 0){
                flag = 1;
                break;
            }
        }
        if (!flag){
            sprintf(response, "cannot find an app named %s.", process->app_name);
            return;
        }
        if (p->is_running){
            sprintf(response, "%s have already been running.", process->app_name);
        }else{
            /*start the process*/
            pid = fork();
            if(pid < 0){
                fprintf(stderr, "cannot fork(), error: %s", strerror(errno));
                exit(EXIT_FAILURE);
            }else if(pid == 0){/*the child process*/
                ignore_signals();
                /*exec the child process.*/
                exec_process(p, &res);
                perror("exit error:");
                exit(EXIT_FAILURE);
            }else{/*the parent process*/
                pthread_mutex_lock(&process_list_mutex);
                p->pid = pid;
                p->is_running = 1;
                time(&p->start_time);
                sprintf(response, "Starting %s with pid:%d",p->app_name, p->pid);
                save_process_list(config_path, process_list);
                pthread_mutex_unlock(&process_list_mutex);
            }
        }
        return;
    }
    
    /*if the len of process->cmd is not zero.
     *  first check if there were duplication of app name. 
     */
    for (i = 0; i < process_count; ++i){
        p = &process_list[i];
        if(strcmp(p->app_name, process->app_name) == 0){
            sprintf(response, "duplicated name of app, change the name of %s", process->app_name);
            return;
        }
    }

    /*then, fork the app process.*/
    pid = fork();
    if(pid < 0){
        fprintf(stderr, "cannot fork(), error: %s", strerror(errno));
        exit(EXIT_FAILURE);
    }else if(pid == 0){/*the child process*/
        ignore_signals();
        /*exec the child process.*/
        exec_process(process, &res);
        perror("exit error:");
        exit(EXIT_FAILURE);
    }else{
        pthread_mutex_lock(&process_list_mutex);
        /*add process to process_list tail.*/
        process->pid = pid;
        process->id = process_count;
        time(&process->start_time);
        process_list[process_count] = *process;
        process_count++;
        sprintf(response, "Started %s with pid:%d",process->app_name, process->pid);
        save_process_list(config_path, process_list);
        pthread_mutex_unlock(&process_list_mutex);
    }
}

void server_restart_process(process_s *process, char* response)
{
    int res;
    pid_t pid;
    char config_path[STR_BUFFER_SIZE];
    /*get config path according to $HOME*/
    strcpy(config_path, getenv("HOME"));
    strcat(config_path, CONFIG_PATH);
    pthread_mutex_lock(&process_list_mutex);
    if(process->is_running){
        /*first:stop the process*/
        process->is_running = 0;
        kill_process(process, &res);
        process->pid = 0;
        process->start_time = 0;
        save_process_list(config_path, process_list);
        sprintf(response, "stop %s success", process->app_name);

        /*second:start the process*/
        pid = fork();
        if(pid < 0){
            fprintf(stderr, "cannot fork(), error: %s", strerror(errno));
            exit(EXIT_FAILURE);
        }else if(pid == 0){/*the child process*/
            ignore_signals();
            /*exec the child process.*/
            exec_process(process, &res);
            perror("exit error:");
            exit(EXIT_FAILURE);
        }else{/*the parent process*/
            process->pid = pid;
            process->is_running = 1;
            time(&process->start_time);
            process->restart_times++;
            sprintf(response, "Restarted %s with pid:%d",process->app_name, process->pid);
            save_process_list(config_path, process_list);
        }

    }else{
        sprintf(response, "the %s is not running.", process->app_name);
    }
    pthread_mutex_unlock(&process_list_mutex);
}

void server_stop_process(process_s *process, char* response)
{
    int res;
    char config_path[STR_BUFFER_SIZE];
    /*get config path according to $HOME*/
    strcpy(config_path, getenv("HOME"));
    strcat(config_path, CONFIG_PATH);
    pthread_mutex_lock(&process_list_mutex);
    if(process->is_running){
        process->is_running = 0;
        process->restart_times = 0;
        kill_process(process, &res);
        process->pid = 0;
        process->start_time = 0;
        save_process_list(config_path, process_list);
        sprintf(response, "stop %s success", process->app_name);
    }else{
        sprintf(response, "the %s is not running.", process->app_name);
    }
    pthread_mutex_unlock(&process_list_mutex);
}

void server_remove_process(process_s *process, char* response)
{
    int res;
    char config_path[STR_BUFFER_SIZE];
    /*get config path according to $HOME*/
    strcpy(config_path, getenv("HOME"));
    strcat(config_path, CONFIG_PATH);

    pthread_mutex_lock(&process_list_mutex);
    if(process->is_running){
        kill_process(process, &res);
        process->pid = 0;
        process->restart_times = 0;
    }
    sprintf(response, "Remove %s success.", process->app_name);
    del_process_by_app_name(process_list, process->app_name);
    process_count--;
    save_process_list(config_path, process_list);
    pthread_mutex_unlock(&process_list_mutex);
}

void server_start_all_process(void)
{
    int res, i;
    pid_t pid;
    process_s *process;
    char config_path[STR_BUFFER_SIZE];
    /*get config path according to $HOME*/
    strcpy(config_path, getenv("HOME"));
    strcat(config_path, CONFIG_PATH);

    pthread_mutex_lock(&process_list_mutex);
    for(i = 0; i < process_count; i++){
        process = &process_list[i];
        if(process->is_running){
            continue;
        }
        pid = fork();
        if(pid < 0){
            fprintf(stderr, "can not fork(), error: %s", strerror(errno));
            exit(EXIT_FAILURE);
        }else if(pid == 0){/*the child process*/
            ignore_signals();
            /*exec the child process.*/
            exec_process(process, &res);
            exit(EXIT_FAILURE);
        }else{
            process->pid = pid;
            process->restart_times = 0;
            //printf("Starting %s with pid:%d",process->app_name, process->pid);
        }
    }
    save_process_list(config_path, process_list);
    pthread_mutex_unlock(&process_list_mutex);
}

void server_stop_all_process(void)
{
    int res, i;
    process_s *process;
    char config_path[STR_BUFFER_SIZE];
    /*get config path according to $HOME*/
    strcpy(config_path, getenv("HOME"));
    strcat(config_path, CONFIG_PATH);

    pthread_mutex_lock(&process_list_mutex);
    for(i = 0; i < process_count; i++){
        process = &process_list[i];
        if(process->is_running){
            process->restart_times = 0;
            process->is_running = 0;
            kill_process(process, &res);
            process->pid = 0;
        }
    }
    save_process_list(config_path, process_list);
    pthread_mutex_unlock(&process_list_mutex);
}

void get_process_list_status(char *buffer)
{
    int res;
    res = create_process_list_json_str_with_status(process_list, process_count, buffer);
    if(res == -1){
        /*set response buffer empty.*/
        memset(buffer, '\0', sizeof(buffer));
    }
}

void create_config_file(void)
{
    int status;
    char *home_dir;
    home_dir = getenv("HOME");
    if (!home_dir){
        fprintf(stderr, "You have to set the env:$HOME\n");
        exit(EXIT_FAILURE);
    }
    char config_path[STR_BUFFER_SIZE];
    char config_dir[STR_BUFFER_SIZE];
    strcpy(config_path, home_dir);
    strcat(config_path, CONFIG_PATH);
    strcpy(config_dir, home_dir);
    strcat(config_dir, CONFIG_DIR);

    /*check out if the config file exsit.*/
    if(access(config_path, F_OK) == -1){
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
    if(pid > 0){
        exit(EXIT_SUCCESS);
    }else if(pid < 0){
        fprintf(stderr, "can not fork(), error: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    if(setsid() < 0){
        fprintf(stderr, "can not setsid(), error: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    pid = fork();
    if(pid > 0){
        exit(EXIT_SUCCESS);
    }else if(pid < 0){
        fprintf(stderr, "can not fork(), error:%s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    for(i = 0; i < NOFILE; i++){
        close(i);
    }

    chdir("/tmp");
    umask(0);

    openlog("kb_pm", LOG_PID, LOG_KERN);
}
