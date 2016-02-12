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
#include "utils.h"

/*daemonize the process*/
void init_daemon(void);

/*ignore some signals except SIGKILL, SIGSTOP*/
void ignore_signals(void);

/*handle the client request*/
void *server_socket_handle_func(void *);

/*if the "~/.kbpm/process.config" does not exsit, create it.*/
void create_config_file();

void server_start_process(process_s *process, char* response);

void server_stop_process(process_s *process, char* response);

void server_remove_process(process_s *process, char* response);

void server_get_processes_status(char* buffer);

void server_start_all_process(void);

void server_stop_all_process(void);

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

    /*daemonize the process*/
    //init_daemon();

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
            printf("Starting %s with pid:%d",process->app_name, process->pid);
        }
    }

    /*listening the child process exit.*/
    while(1){
        if(!process_count || !get_running_process_count()){
            usleep(100 * 1000);//sleep 100 ms
        }
        pid = wait(NULL);
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
                    printf("restart app: %s\n", process->app_name);
                    break;
                }
            }
        }
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
            server_start_all_process();
        }else if(strcmp(buffer, "stopall") == 0){
            server_stop_all_process();
        }else if(strcmp(buffer, "start") == 0){
            printf("\nread from client : start process\n\n");
            write(client_sockfd, "pong", strlen("pong"));
            memset(buffer, '\0', sizeof(buffer));
            read(client_sockfd, buffer, STR_BUFFER_SIZE);
            if(parse_process(&process, buffer) == -1){
                strcpy(buffer, "parse error");
                write(client_sockfd, buffer, strlen(buffer));
                goto GO_ON;
            }
            server_start_process(&process, response);
        }else if(strncmp(buffer, "stop", strlen("stop")) == 0){
            // strcpy(buffer, &buffer[strlen("start ") - 1]);

            // server_stop_process(&process, response);
        }else if(strncmp(buffer, "remove", strlen("remove")) == 0){
            // strcpy(buffer, &buffer[strlen("remove ") - 1]);

            // server_remove_process(&process, response);
        }else{
            goto GO_ON;
        }
        write(client_sockfd, response, strlen(response));
GO_ON:
        close(client_sockfd);
        memset(buffer, '\0', sizeof(buffer));
    }
}

void server_start_process(process_s *process, char* response)
{
    int res;
    pid_t pid;
    pid = fork();
    if(pid < 0){
        fprintf(stderr, "can not fork(), error: %s", strerror(errno));
        exit(EXIT_FAILURE);
    }else if(pid == 0){/*the child process*/
        ignore_signals();
        /*exec the child process.*/
        exec_process(process, &res);
        perror("exit error:");
        exit(EXIT_FAILURE);
    }else{
        process->pid = pid;
        /*add process to process_list tail.*/
        process_list[process_count] = *process; 
        printf("Starting %s with pid:%d",process->app_name, process->pid);
        strcpy(response, "start success");
    }
}

void server_stop_process(process_s *process, char* response)
{

}

void server_remove_process(process_s *process, char* response)
{

}

void server_start_all_process(void)
{

}

void server_stop_all_process(void)
{

}

void get_processes_status(char *buffer)
{
}

void create_config_file(void)
{
    int status;
    char *home_dir;
    home_dir = getenv("HOME");
    if (!home_dir)
    {
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
