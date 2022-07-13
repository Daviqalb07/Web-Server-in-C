#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>

#include "properties.h"

typedef int bool;

typedef struct {
    int sockfd;
    struct sockaddr_in address;
} Host;

typedef struct{
    char *method;
    char *path;
    char *protocol;
} Request;

void reciveData(int fd){
    Request request;
    char buffer[MAXBUF];
    read(fd, buffer, sizeof(buffer));
    // // buffer = GET /index.html HTTP/1.1
    // request.method = strtok(buffer, " ");
    // request.path = strtok(NULL, " ");
    // request.protocol = strtok(NULL, "\r");
    // request.method = " ";
    // request.path = " ";
    // request.protocol = " ";
    char dest[40] = "";
    char dest1[40] = "";
    char dest2[40] = "";
    int k = 0;
    for(int i = 0; i  < sizeof(buffer); i++){
        if(buffer[i-1] == '\n') break;
        if(buffer[i] == ' ') k++;
        else{
            if(k == 0) strncat(dest,&buffer[i],1);
            if(k == 1) strncat(dest1,&buffer[i],1);
            if(k == 2) strncat(dest2,&buffer[i],1);
            // printf("%c",buffer[i]);
        }
    }
    // for(int i = 0; i < sizeof(request.method); i++){
    //     printf("%c",request.method[i]);
    // }
    // printf("\n");
    // for(int i = 0; i < sizeof(request.path); i++){
    //     printf("%c",request.path[i]);
    // }
    // printf("\n");
    // for(int i = 0; i < sizeof(request.protocol); i++){
    //     printf("%c",request.protocol[i]);
    // }
    // printf("\n");
        printf(" %s \n", dest);
        printf(" %s \n", dest1);
        printf(" %s \n", dest2);

    
}



void serverBind(Host *server){
    if(bind(server->sockfd, (struct sockaddr*)&server->address, sizeof(server->address)) < 0){
        perror("bind");
        exit(1);
    }
    else 
        printf("Ta de pe\n");
}


Host createServer(int port, char *address){
    Host host;
    host.sockfd = socket(AF_INET, SOCK_STREAM, 0);

    if(host.sockfd < 0){
        perror("socket");
        exit(1);
    }
    else
        printf("Socket criado\n");
    
    memset(&host.address, 0, sizeof(host.address));

    host.address.sin_family = AF_INET;
    host.address.sin_port = htons(port);
    host.address.sin_addr.s_addr = inet_addr(address);

    serverBind(&host);

    if(listen(host.sockfd, 2048) < 0){
        perror("listen");
        exit(1);
    }
    else
    {
        printf("Escutando\n");
    }


    return host;
}

bool acceptConnection(Host *server, Host *client){
    memset(&client->address, 0, sizeof(client->address));
    socklen_t sizeAdress = sizeof(server->address);

    client->sockfd = accept(server->sockfd, (struct sockaddr *) &client->address, &sizeAdress);
    if(client->sockfd < 0){
        printf("deu erro\n");
        return false;
    }
    printf("Cliente [%s] conectado pela porta [%d]\n", inet_ntoa(client->address.sin_addr), ntohs(client->address.sin_port));
    return true;
}

 


int main(int argc, char const *argv[])
{
    Host server = createServer(PORT, IP_ADDR);
    Host clients[N_MAX_CLIENTS];

    acceptConnection(&server, &clients[0]);
    reciveData(clients[0].sockfd);


    return 0;
}
