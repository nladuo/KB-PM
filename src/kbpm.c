#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include "server.h"
#include "client.h"
#include "kbpm.h"

/*print the usage of kb_pm*/
void print_usage(void);

int main(int argc, char** argv)
{
    int res;
    if(argc == 1){
        print_usage();
        exit(EXIT_SUCCESS);
    }

    if(argc == 2){
        /*start all process.*/
        if(strcmp(argv[1], "startall") == 0){
            start_all();
        }

        /*stop all process.*/
        if(strcmp(argv[1], "stopall") == 0){
            stop_all();
        }

        /*show the processes status*/
        if(strcmp(argv[1], "status") == 0){
            show_status();
        }
        fprintf(stderr, "arguments error, see usage.\n");
        print_usage();
        exit(EXIT_FAILURE);
    }

    if(argc == 3){
        /*service start.*/
        if( (strcmp(argv[1], "service") == 0)
                &&(strcmp(argv[2], "start") == 0)){
            ping_server(&res);
            if (res != 0)
            {
                printf(APP_NAME ": Starting the server.\n");
                service_start();
            }else{
                printf(APP_NAME ": service have been already started.\n");
                exit(EXIT_SUCCESS);
            }
            
        }

        /*start a process.*/
        if(strcmp(argv[1], "start") == 0){
            start_process(argv[2]);
        }

        /*restart a process.*/
        if(strcmp(argv[1], "restart") == 0){
            restart_process(argv[2]);
        }

        /*stop a process.*/
        if(strcmp(argv[1], "stop") == 0){
            stop_process(argv[2]);
        }

        /*remove a process.*/
        if(strcmp(argv[1], "remove") == 0){
            remove_process(argv[2]);
        }

        fprintf(stderr, "arguments error, see usage.\n");
        print_usage();
        exit(EXIT_FAILURE);
    }

    fprintf(stderr, "too many arguments, see usage.\n");
    print_usage();
    exit(EXIT_FAILURE);
}

void print_usage(void)
{
    printf(
        APP_NAME ": a process manager to make process run forever.\n"
        "    Usage  :  kbpm [cmd] app\n"
        "\tservice  start               : start the kbpm service\n"
        "\tstart    <app_name|cmd|id>   : start the program and run forever\n"
        "\trestart  <app_name|id>       : restart the program\n"
        "\tstop     <app_name|id>       : stop the program\n"
        "\tremove   <arg_name|id>       : remove the program\n"
        );
}
