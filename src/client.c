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

/*send signal to server and get the result back*/
void communicate_with_server(const char* sig, int* res, char* buffer);

/*send two signals to server and get the result back*/
void communicate_two_signals_with_server(char* sig1, char* sig2, int* res, char* buffer);

void start_process(const char *cmd)
{
    int res, size;
    char sig[BUFFER_SIZE];
    char buffer[BUFFER_SIZE];
    char dst[LIST_SIZE][STR_BUFFER_SIZE];
    char cmd_copy[STR_BUFFER_SIZE];
    process_s process;
    /*check if cmd exist.*/
    if(access(cmd, F_OK) == -1){
        fprintf(stderr, "Error : %s does not exit.\n", cmd);
        exit(EXIT_FAILURE);
    }
    /*check if cmd can be executed.*/
    if(access(cmd, X_OK) == -1){
        fprintf(stderr, "Error : %s cannot be executed.\n", cmd);
        exit(EXIT_FAILURE);
    }
    /*check if the server is online.*/
    ping_server(&res);
    if(res != 0){
        fprintf(stderr, "Error : server is not online.Run 'kbpm service start' to start the kbpm service.\n");
        exit(EXIT_FAILURE);
    }

    /*get the app_name due to cmd.
     *  For example, if the cmd is '/home/nladuo/a.out',
     *  the app_name would be 'a.out'*/
    strcpy(cmd_copy, cmd);
    size = str_split(dst, (char *)cmd_copy, "/");

    /*set process values.*/
    strcpy(process.app_name, dst[size - 1]);
    getcwd(process.dir, STR_BUFFER_SIZE);
    strcpy(process.cmd, cmd);
    process.is_running = 1;

    /*get json string of process.*/
    create_process_json_str(&process, sig);
    printf("sig:\n%s\n", sig);

    communicate_two_signals_with_server("start", sig, &res, buffer);
    if(res != 0){
        printf("%s\n", "server error occur");
        exit(EXIT_FAILURE);
    }
    printf("result : %s\n", buffer);

    exit(EXIT_SUCCESS);
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
    printf("result : %s\n", buffer);

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
        printf("%s\n", "server error occur");
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
        printf("%s\n", "server error occur");
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
        printf("%s\n", "server error occur");
        exit(EXIT_FAILURE);
    }
    printf("result : %s\n", buffer);

    exit(EXIT_SUCCESS);
}

void show_status(void)
{
    int res;
    char buffer[BUFFER_SIZE];
    /*check if the server is online.*/
    ping_server(&res);
    if(res != 0){
        fprintf(stderr, "Error : server is not online.Run 'kbpm service start' to start the kbpm service.\n");
        exit(EXIT_FAILURE);
    }

    communicate_with_server("status", &res, buffer);
    if(res != 0){
        printf("%s\n", "server error occur");
        exit(EXIT_FAILURE);
    }
    printf("result : %s\n", buffer);

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
        perror("socket err:");
        return;
    }

    write(sockfd, sig, strlen(sig));
    read(sockfd, buffer, BUFFER_SIZE);
    printf("read form server:%s\n", buffer);
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
        perror("socket err:");
        return;
    }
    write(sockfd, sig1, strlen(sig1));
    read(sockfd, buffer, BUFFER_SIZE);
    if(strcmp(buffer, "pong") != 0){
        *res = -1;
        return;
    }
    printf("sig1:read form server:%s\n", buffer);

    write(sockfd, sig2, strlen(sig2));
    read(sockfd, buffer, BUFFER_SIZE);
    printf("sig2:read form server:%s\n", buffer);
    close(sockfd);
    *res = 0;
}
