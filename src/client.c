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
#include "box_drawing.h"

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
    if(res != 0){
        fprintf(stderr, "Error : server is not online.Run 'kbpm service start' to start the kbpm service.\n");
        exit(EXIT_FAILURE);
    }

    /*if cmd not exist, send app_name */
    if(access(app_name_or_cmd, F_OK) == -1){
        /*set process values, only app_name is useful.*/
        strcpy(process.app_name, app_name_or_cmd);
        strcpy(process.cmd, "");
        strcpy(process.dir, "");
        process.is_running = 0;
        /*get json string of process.*/
        create_process_json_str(&process, sig);
        //printf("sig:\n%s\n", sig);

        communicate_two_signals_with_server("start", sig, &res, buffer);
        if(res != 0){
            fprintf(stderr, "%s\n", "server error occur");
            exit(EXIT_FAILURE);
        }
        printf(APP_NAME ": %s\n", buffer);
        show_status();
        exit(EXIT_SUCCESS);
    }

    /*check if cmd can be executed.*/
    if(access(app_name_or_cmd, X_OK) == -1){
        fprintf(stderr, "Error : %s cannot be executed.\n", app_name_or_cmd);
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
    if(res != 0){
        printf("%s\n", "server error occur");
        exit(EXIT_FAILURE);
    }
    printf(APP_NAME ": %s\n", buffer);
    show_status();
    exit(EXIT_SUCCESS);
}

void restart_process(const char *app_name)
{


}

void stop_process(const char *app_name)
{
    int res;
    char buffer[BUFFER_SIZE];
    /*check if the server is online.*/
    ping_server(&res);
    if(res != 0){
        fprintf(stderr, "Error : server is not online.Run 'kbpm service start' to start the kbpm service.\n");
        exit(EXIT_FAILURE);
    }

    communicate_two_signals_with_server("stop", (char *)app_name, &res, buffer);
    if(res != 0){
        printf("%s\n", "server error occur");
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
    if(res != 0){
        fprintf(stderr, "Error : server is not online.Run 'kbpm service start' to start the kbpm service.\n");
        exit(EXIT_FAILURE);
    }

    communicate_two_signals_with_server("remove", (char *)app_name, &res, buffer);
    if(res != 0){
        fprintf(stderr, "%s\n", "Error: server error occur");
        exit(EXIT_FAILURE);
    }
    printf("result : %s\n", buffer);

    exit(EXIT_SUCCESS);
}

void start_all(void)
{
    int res;
    char buffer[BUFFER_SIZE];
    /*check if the server is online.*/
    ping_server(&res);
    if(res != 0){
        fprintf(stderr, "Error : server is not online.Run 'kbpm service start' to start the kbpm service.\n");
        exit(EXIT_FAILURE);
    }

    communicate_with_server("startall", &res, buffer);
    if(res != 0){
        fprintf(stderr, "%s\n", "server error occur");
        exit(EXIT_FAILURE);
    }
    printf("result : %s\n", buffer);

    exit(EXIT_SUCCESS);
}

void stop_all(void)
{
    int res;
    char buffer[BUFFER_SIZE];
    /*check if the server is online.*/
    ping_server(&res);
    if(res != 0){
        fprintf(stderr, "Error : server is not online.Run 'kbpm service start' to start the kbpm service.\n");
        exit(EXIT_FAILURE);
    }

    communicate_with_server("stopall", &res, buffer);
    if(res != 0){
        fprintf(stderr, "%s\n", "server error occur");
        exit(EXIT_FAILURE);
    }
    printf("result : %s\n", buffer);

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
    if(res != 0){
        fprintf(stderr, "Error : server is not online.Run 'kbpm service start' to start the kbpm service.\n");
        exit(EXIT_FAILURE);
    }

    communicate_with_server("status", &res, buffer);
    if(res != 0){
        fprintf(stderr, "%s\n", "server error occur");
        exit(EXIT_FAILURE);
    }
    //printf("result : %s\n", buffer);
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
    if(res == -1){
        *result = -1;
        return;
    }
    if(strcmp(buffer, "pong") != 0){
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
    if(*res == -1){
        //perror("socket err:");
        return;
    }

    write(sockfd, sig, strlen(sig));
    read(sockfd, buffer, BUFFER_SIZE);
    //printf("read form server:%s\n", buffer);
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
        //perror("socket err:");
        return;
    }
    write(sockfd, sig1, strlen(sig1));
    read(sockfd, buffer, BUFFER_SIZE);
    if(strcmp(buffer, "pong") != 0){
        *res = -1;
        return;
    }
    //printf("sig1:read form server:%s\n", buffer);

    write(sockfd, sig2, strlen(sig2));
    read(sockfd, buffer, BUFFER_SIZE);
    //printf("sig2:read form server:%s\n", buffer);
    close(sockfd);
    *res = 0;
}
