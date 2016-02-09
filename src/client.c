#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/un.h>
#include <sys/types.h>
#include <sys/socket.h>
#include "client.h"


/*send signal to server and get the result back*/
void communicate_signal(const char* sig, int* res, char* buffer);

void start_process(process_s *process)
{


}

void stop_process(process_s *process)
{


}

void remove_process(const char* app_name)
{

}

void start_all(void)
{

}

void stop_all(void)
{

}

void show_status(void)
{

}

/*send signal to server and get the result back*/
void communicate_signal(const char* sig, int* res, char* buffer)
{
    int sockfd,len;
    struct sockaddr_un addr;

    sockfd = socket(AF_UNIX, SOCK_STREAM, 0);

    addr.sun_family = AF_UNIX;
    strcpy(addr.sun_path, "server_socket");
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
}
