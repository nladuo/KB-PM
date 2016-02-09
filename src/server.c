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
#include <fcntl.h>
#include <pthread.h>
#include <semaphore.h>
#include <signal.h>
#include "server.h"

/*daemonize the process*/
void init_daemon(void);

/*ignore some signals except SIGKILL, SIGSTOP*/
void ignore_signals(void);

/*handle the client request*/
void *server_socket_handle_func(void *);

/*need mutex when access process_list*/
pthread_mutex_t process_list_mutex;

/*
 * start the KB_PM service, and run in daemon.
 */
void service_start(void)
{
    int i, count, res;
    pid_t pid;
    process_s process_list[LIST_SIZE], *process;
    pthread_t socket_server_thread;

    /*get process_list when service start up.*/
    parse_process_list(CONFIG_PATH, process_list);

    /*get process_list actual count.*/
    get_process_list_count(process_list, &count);

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
    for(i = 0; i < count; i++){
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
            printf("app_name:%s, pid:%d\n",process->app_name, process->pid);
        }
    }

    /*listening the child process exit.*/
    while(1){
        pid = wait(NULL);
        for(i = 0; i < count; i++){
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


void *server_socket_handle_func(void *args)
{
    int server_sockfd, client_sockfd;
    int server_len, client_len;
    char buffer[BUFFER_SIZE];
    struct sockaddr_un server_addr, client_addr;

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
        read(client_sockfd, buffer, BUFFER_SIZE);
        write(client_sockfd, buffer, strlen(buffer));
        close(client_sockfd);
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
