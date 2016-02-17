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
#include "server.h"
#include "client.h"
#include "kbpm.h"

/*print the usage of kb_pm*/
int print_usage(void);

int main(int argc, char** argv)
{
    int res;
    if(argc == 1)
    {
        print_usage();
        exit(EXIT_SUCCESS);
    }

    if(argc == 2)
    {
        /*start all process.*/
        if(strcmp(argv[1], "startall") == 0)
        {
            start_all();
        }

        /*stop all process.*/
        if(strcmp(argv[1], "stopall") == 0)
        {
            stop_all();
        }

        /*show the processes status*/
        if(strcmp(argv[1], "status") == 0)
        {
            show_status();
        }

        /*show the version of KB_PM*/
        if(strcmp(argv[1], "version") == 0)
        {
            printf(APP_NAME ": %s\n", APP_VERSION);
            exit(EXIT_SUCCESS);
        }
        fprintf(stderr, APP_NAME ": Arguments error, see usage.\n");
        exit(EXIT_FAILURE);
    }

    if(argc == 3)
    {
        /*service start.*/
        if(strcmp(argv[1], "service") == 0)
        {
            if (strcmp(argv[2], "start") == 0)
            {
                ping_server(&res);
                if (res != 0)
                {
                    printf(APP_NAME ": Starting the server.\n"
                        ANSI_FONT_ITALIC  " Use `kbpm start <file>` to make an app run forever\n" ANSI_COLOR_RESET);
                    service_start();
                }
                else
                {
                    printf(APP_NAME ": Service have been already started.\n"
                        ANSI_FONT_ITALIC  " Use `kbpm status` to get the status of program(s)\n" ANSI_COLOR_RESET);
                    exit(EXIT_SUCCESS);
                }
            }

            if (strcmp(argv[2], "clean") == 0)
            {
                service_clean();
            }
        }

        /*start a process.*/
        if(strcmp(argv[1], "start") == 0)
        {
            start_process(argv[2]);
        }

        /*restart a process.*/
        if(strcmp(argv[1], "restart") == 0)
        {
            restart_process(argv[2]);
        }

        /*stop a process.*/
        if(strcmp(argv[1], "stop") == 0)
        {
            stop_process(argv[2]);
        }

        /*remove a process.*/
        if(strcmp(argv[1], "remove") == 0)
        {
            remove_process(argv[2]);
        }

        fprintf(stderr, APP_NAME ": Arguments error, see usage.\n");
        exit(EXIT_FAILURE);
    }

    fprintf(stderr, APP_NAME ": Too many arguments, see usage.\n");
    exit(EXIT_FAILURE);
}

int print_usage(void)
{
    return printf(
        APP_NAME ": a CLI tool (like supervisor and pm2) keep a group of program running continuously.\n"
        "   Usage  :  kbpm [cmd] app\n"
        "       service  start                start the KB_PM service\n"
        "       service  clean                resolve the KB_PM service unexpectedly exit.\n"
        "       start    <file|app_name|id>   start the program and run forever\n"
        "       restart  <app_name|id>        restart the program\n"
        "       stop     <app_name|id>        stop the program\n"
        "       remove   <app_name|id>        remove the program form list\n"
        "       startall                      start all program(s)\n"
        "       stopall                       stop all the program(s)\n"
        "       version                       show KB_PM version\n"
        );
}
