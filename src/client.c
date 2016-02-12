#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/un.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>
#include "client.h"

/*send signal to server and get the result back*/
void communicate_with_server(const char* sig, int* res, char* buffer);

/*send two signals to server and get the result back*/
void communicate_two_signals_with_server(char* sig1, char* sig2, int* res, char* buffer);

void start_process(const char *cmd)
{
    int res;
    char sig[BUFFER_SIZE];
    char buffer[BUFFER_SIZE];
    process_s process;
    /*set process values.*/
    strcpy(process.app_name, cmd);
    getcwd(process.dir, STR_BUFFER_SIZE);
    strcpy(process.cmd, cmd);
    process.is_running = 1;

    /*get json string of process.*/
    create_process_json_str(&process, sig);
    printf("sig:\n%s\n", sig);

    communicate_two_signals_with_server("start", sig, &res, buffer);
    if(res != 0){
        printf("%s\n", "server error occur");
        exit(-1);
    }
    printf("result : %s\n", buffer);

    exit(EXIT_SUCCESS);
}

void stop_process(const char *app_name)
{
    printf("stop_process:%s\n", app_name);
    exit(EXIT_SUCCESS);
}

void remove_process(const char* app_name)
{
    printf("remove_process:%s\n", app_name);
    exit(EXIT_SUCCESS);
}

void start_all(void)
{
    printf("start_all\n");
    exit(EXIT_SUCCESS);
}

void stop_all(void)
{
    printf("stop_all\n");
    exit(EXIT_SUCCESS);
}

void show_status(void)
{
    printf("show_status\n");
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
