#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <strings.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>

#include "properties.h"

int main(int argc, char const *argv[])
{
    int sockfd;
    struct sockaddr_in serveraddr;

    sockfd = socket(AF_INET, SOCK_STREAM, 0);

    if(sockfd < 0){
        printf("Não deu certo.\n");
        exit(0);
    }
    else
        printf("Socket criado\n");
    
    bzero(&serveraddr, sizeof(serveraddr));

    serveraddr.sin_family = AF_INET;
    serveraddr.sin_port = htons(PORT);
    serveraddr.sin_addr.s_addr = inet_addr(IP_ADDR);

    if(bind(sockfd, (struct sockaddr*)&serveraddr, sizeof(serveraddr)) < 0){
        printf("Servidor nao esta on :(\n");
        exit(1);
    }

    printf("Servidor Web esta ON! \n");

    listen(sockfd, N_MAX_CLIENTS);
    return 0;


}
