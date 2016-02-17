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
#include <sys/un.h>
#include <sys/types.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <string.h>
#include "client.h"
#include "utils.h"
#include "kbpm.h"
#include "ansi_escape.h"

/*show that KB_PM server is not starting*/
#define PRINT_NOT_ONLINE fprintf(stderr, \
            APP_NAME ": Server is not online.\n" \
            ANSI_FONT_ITALIC   \
            " Use `kbpm service start` to start the KB_PM service\n" \
            ANSI_COLOR_RESET)
#define PRINT_SERVER_ERR fprintf(stderr, APP_NAME ": Server error occur.\n")

/*send signal to server and get the result back*/
void communicate_with_server(const char* sig, int* res, char* buffer);

/*send two signals to server and get the result back*/
void communicate_two_signals_with_server(char* sig1, char* sig2, int* res, char* buffer);

void start_process(const char *app_name_or_cmd)
{
    int res, size;
    char sig[BUFFER_SIZE];
    char buffer[BUFFER_SIZE];
    char dst[LIST_SIZE][STR_BUFFER_SIZE];
    char cmd_copy[STR_BUFFER_SIZE];
    process_s process;

    /*check if the server is online.*/
    ping_server(&res);
    if(res != 0)
    {
        PRINT_NOT_ONLINE;
        exit(EXIT_FAILURE);
    }

    /*if cmd not exist, send app_name or id */
    if(access(app_name_or_cmd, F_OK) == -1)
    {
        communicate_two_signals_with_server("start", (char *)app_name_or_cmd, &res, buffer);
        if(res != 0)
        {
            PRINT_SERVER_ERR;
            exit(EXIT_FAILURE);
        }
        printf(APP_NAME ": %s\n", buffer);
        show_status();
        exit(EXIT_SUCCESS);
    }

    /*check if cmd can be executed.*/
    if(access(app_name_or_cmd, X_OK) == -1){
        fprintf(stderr, APP_NAME ": %s cannot be executed.\n", app_name_or_cmd);
        exit(EXIT_FAILURE);
    }

    /*get the app_name due to cmd.
     *  For example, if the cmd is '/home/nladuo/a.out',
     *  the app_name would be 'a.out'.
     */
    strcpy(cmd_copy, app_name_or_cmd);
    size = str_split(dst, (char *)cmd_copy, "/");

    /*set process values.*/
    strcpy(process.app_name, dst[size - 1]);
    getcwd(process.dir, STR_BUFFER_SIZE);
    strcpy(process.cmd, app_name_or_cmd);
    process.is_running = 1;

    /*get json string of process.*/
    create_process_json_str(&process, sig);
    //printf("sig:\n%s\n", sig);

    communicate_two_signals_with_server("start", sig, &res, buffer);
    if(res != 0)
    {
        PRINT_SERVER_ERR;
        exit(EXIT_FAILURE);
    }
    printf(APP_NAME ": %s\n", buffer);
    show_status();
    exit(EXIT_SUCCESS);
}

void restart_process(const char *app_name)
{
    int res;
    char buffer[BUFFER_SIZE];
    /*check if the server is online.*/
    ping_server(&res);
    if(res != 0)
    {
        PRINT_NOT_ONLINE;
        exit(EXIT_FAILURE);
    }

    communicate_two_signals_with_server("restart", (char *)app_name, &res, buffer);
    if(res != 0)
    {
        PRINT_SERVER_ERR;
        exit(EXIT_FAILURE);
    }
    printf(APP_NAME ": %s\n", buffer);
    show_status();
    exit(EXIT_SUCCESS);
}

void stop_process(const char *app_name)
{
    int res;
    char buffer[BUFFER_SIZE];
    /*check if the server is online.*/
    ping_server(&res);
    if(res != 0)
    {
        PRINT_NOT_ONLINE;
        exit(EXIT_FAILURE);
    }

    communicate_two_signals_with_server("stop", (char *)app_name, &res, buffer);
    if(res != 0)
    {
        PRINT_SERVER_ERR;
        exit(EXIT_FAILURE);
    }
    printf(APP_NAME ": %s\n", buffer);
    show_status();
    exit(EXIT_SUCCESS);
}

void remove_process(const char* app_name)
{
    int res;
    char buffer[BUFFER_SIZE];
    /*check if the server is online.*/
    ping_server(&res);
    if(res != 0)
    {
        PRINT_NOT_ONLINE;
        exit(EXIT_FAILURE);
    }

    communicate_two_signals_with_server("remove", (char *)app_name, &res, buffer);
    if(res != 0)
    {
        PRINT_SERVER_ERR;
        exit(EXIT_FAILURE);
    }
    printf(APP_NAME ": %s\n", buffer);
    show_status();
    exit(EXIT_SUCCESS);
}

void start_all(void)
{
    int res;
    char buffer[BUFFER_SIZE];
    /*check if the server is online.*/
    ping_server(&res);
    if(res != 0)
    {
        PRINT_NOT_ONLINE;
        exit(EXIT_FAILURE);
    }

    communicate_with_server("startall", &res, buffer);
    if(res != 0)
    {
        PRINT_SERVER_ERR;
        exit(EXIT_FAILURE);
    }
    printf(APP_NAME ": %s\n", buffer);
    show_status();
    exit(EXIT_SUCCESS);
}

void stop_all(void)
{
    int res;
    char buffer[BUFFER_SIZE];
    /*check if the server is online.*/
    ping_server(&res);
    if(res != 0)
    {
        PRINT_NOT_ONLINE;
        exit(EXIT_FAILURE);
    }

    communicate_with_server("stopall", &res, buffer);
    if(res != 0)
    {
        PRINT_SERVER_ERR;
        exit(EXIT_FAILURE);
    }
    printf(APP_NAME ": %s\n", buffer);
    show_status();
    exit(EXIT_SUCCESS);
}

void show_status(void)
{
    int i;
    int res, count;
    char buffer[BUFFER_SIZE];
    process_s process_list[LIST_SIZE];
    process_s *process;
    /*check if the server is online.*/
    ping_server(&res);
    if(res != 0)
    {
        PRINT_NOT_ONLINE;
        exit(EXIT_FAILURE);
    }

    communicate_with_server("status", &res, buffer);
    if(res != 0)
    {
        PRINT_SERVER_ERR;
        exit(EXIT_FAILURE);
    }

    count = parse_process_list_with_status(process_list, buffer);
    print_process_list_box(process_list, count);
    exit(EXIT_SUCCESS);
}

void ping_server(int *result)
{
    int res;
    char buffer[BUFFER_SIZE];
    char sig[BUFFER_SIZE] = "ping";
    communicate_with_server(sig, &res, buffer);
    if(res == -1)
    {
        *result = -1;
        return;
    }

    if(strcmp(buffer, "pong") != 0)
    {
        *result = -1;
        return;
    }

    *result = 0;
}

/*send signal to server and get the result back*/
void communicate_with_server(const char* sig, int* res, char* buffer)
{
    int sockfd,len;
    struct sockaddr_un addr;

    sockfd = socket(AF_UNIX, SOCK_STREAM, 0);

    addr.sun_family = AF_UNIX;
    strcpy(addr.sun_path, LOCAL_SOCKET_NAME);
    len = sizeof(addr);

    *res = connect(sockfd, (struct sockaddr*)&addr, len);
    if(*res == -1)
    {
        /*perror("socket err:");*/
        return;
    }

    write(sockfd, sig, strlen(sig));
    read(sockfd, buffer, BUFFER_SIZE);
    /*printf("read form server:%s\n", buffer);*/
    close(sockfd);
    *res = 0;
}

/*send two signals to server and get the result back*/
void communicate_two_signals_with_server(char* sig1, char* sig2, int* res, char* buffer)
{
    int sockfd,len;
    struct sockaddr_un addr;

    sockfd = socket(AF_UNIX, SOCK_STREAM, 0);

    addr.sun_family = AF_UNIX;
    strcpy(addr.sun_path, LOCAL_SOCKET_NAME);
    len = sizeof(addr);

    *res = connect(sockfd, (struct sockaddr*)&addr, len);
    if(*res == -1){
        /*perror("socket err:");*/
        return;
    }
    write(sockfd, sig1, strlen(sig1));
    read(sockfd, buffer, BUFFER_SIZE);
    if(strcmp(buffer, "pong") != 0){
        *res = -1;
        return;
    }
    /*printf("sig1:read form server:%s\n", buffer);*/

    write(sockfd, sig2, strlen(sig2));
    read(sockfd, buffer, BUFFER_SIZE);
    /*printf("sig2:read form server:%s\n", buffer);*/
    close(sockfd);
    *res = 0;
}
