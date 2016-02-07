#include <unistd.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/param.h>
#include <sys/syslog.h>
#include <stdio.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include "server.h"

/*daemonize the process*/
void init_daemon(void);

/*ignore some signals except SIGKILL, SIGSTOP*/
void ignore_signals(void);
/*
 * start the KB_PM service, and run in daemon.
 */
void service_start(void)
{
    int i, count, res;
    pid_t pid;
    process_s process_list[LIST_SIZE], *process;
    /*get process_list when service start up.*/
    parse_process_list(CONFIG_PATH, process_list);
    get_process_list_count(process_list, &count);
    /*daemonize the process*/
    init_daemon();
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
