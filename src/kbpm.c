#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include "server.h"
#include "client.h"

/*print the usage of kb_pm*/
void print_usage(void);

int main(int argc, char** argv)
{


    /*int count = -1;
    process_s process_list[256];
    parse_process_list("./test.config", process_list);
    get_process_list_count(process_list, &count);
    fprintf(stdout, "count:%d\n", count);
    del_process_by_app_name(process_list, "hello_world");
    save_process_list("haha.config", process_list);
    exit(EXIT_SUCCESS);*/

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
        printf("arguments error, see usage.\n");
        exit(EXIT_FAILURE);
    }

    if(argc == 3){

        /*service start.*/
        if((strcmp(argv[1], "service") == 0)
                &&(strcmp(argv[2], "start") == 0)){
            printf("Starting the server.\n");
            service_start();
        }

        /*start a process.*/
        if(strcmp(argv[1], "start") == 0){
            start_process(argv[2]);
        }

        /*stop a process.*/
        if(strcmp(argv[1], "stop") == 0){
            stop_process(argv[2]);
        }

        /*remove a process.*/
        if(strcmp(argv[1], "remove") == 0){
            remove_process(argv[2]);
        }

        printf("arguments error, see usage.\n");
        exit(EXIT_FAILURE);
    }

    printf("too many arguments, see usage.\n");
    exit(EXIT_FAILURE);
}

void print_usage(void)
{
    printf("print_usage\n");
}
