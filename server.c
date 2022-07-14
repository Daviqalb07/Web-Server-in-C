#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <string.h>
#include <time.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "properties.h"

typedef int bool;

typedef struct {
    int sockfd;
    struct sockaddr_in address;
    bool using;
} Host;

typedef struct{
    char method[40];
    char path[40];
    char protocol[40];
} Request;

const char * get_mime_type(const char * name) {
    const char * ext = strrchr(name, '.');
    if (!ext) return NULL;
    if (strcmp(ext, ".html") == 0 || strcmp(ext, ".htm") == 0) return "text/html";
    if (strcmp(ext, ".jpg") == 0 || strcmp(ext, ".jpeg") == 0) return "image/jpeg";
    if (strcmp(ext, ".gif") == 0) return "image/gif";
    if (strcmp(ext, ".png") == 0) return "image/png";
    if (strcmp(ext, ".css") == 0) return "text/css";
    if (strcmp(ext, ".au") == 0) return "audio/basic";
    if (strcmp(ext, ".wav") == 0) return "audio/wav";
    if (strcmp(ext, ".avi") == 0) return "video/x-msvideo";
    if (strcmp(ext, ".mpeg") == 0 || strcmp(ext, ".mpg") == 0) return "video/mpeg";
    if (strcmp(ext, ".mp3") == 0) return "audio/mpeg";
    return NULL;
}

Request receiveData(int fd){
    Request request;
    char buffer[MAXBUF];
    read(fd, buffer, sizeof(buffer));
    char method[40] = "";
    char path[40] = "";
    char protocol[40] = "";
    int k = 0;
    for(int i = 0; i  < sizeof(buffer); i++){
        if(buffer[i-1] == '\n') break;
        if(buffer[i] == ' ') k++;
        else{
            if(k == 0) strncat(method,&buffer[i],1);
            if(k == 1) strncat(path,&buffer[i],1);
            if(k == 2) strncat(protocol,&buffer[i],1);
        }
    }
    protocol[strlen(protocol)-2] = '\0';
    strcpy(request.method,method);
    strcpy(request.path,path);
    strcpy(request.protocol,protocol);    

    return request;
}

void sendHeader(int clientfd, int status, char* title, char* type, int length, char* protocol){
    char header[MAXBUF] = {0};
    char aux[200] = {0};
    char timebuf[150] = {0};
    time_t now;

    sprintf(aux, "%s %d %s\r\n", protocol, status, title);
    strcat(header, aux);

    now = time(NULL);
    strftime(timebuf, sizeof(timebuf), GMT_TIME, gmtime(&now));
    memset(aux, '\0', sizeof(aux));
    sprintf(aux, "Date: %s\r\n", timebuf);
    strcat(header, aux);


    if (type) {
        memset(aux, '\0', sizeof(aux));
        sprintf(aux, "Content-Type: %s\r\n", type);
        strcat(header, aux);
    }
    if (length >= 0) {
        memset(aux, '\0', sizeof(aux));
        sprintf(aux, "Content-Length: %d\r\n", length);
        strcat(header, aux);
    }
    strcat(header, "Connection: close\r\n\r\n");
    
    printf("%s", header);
    send(clientfd, header, strlen(header), 0);
}

void sendData(int clientfd, char* filename){
    char buffaux;
    int fd;
    fd = open(filename, O_RDONLY);

    while(read(fd, &buffaux, 1)){
        write(clientfd, &buffaux, 1);
    }
    close(fd);
}

void serverBind(Host *server){
    if(bind(server->sockfd, (struct sockaddr*)&server->address, sizeof(server->address)) < 0){
        perror("bind");
        exit(1);
    }
    else 
        printf("Servidor Web esta ON!\n");
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
        printf("Escutando\n\n\n");
    }


    return host;
}


void handleConnection(void* cli){
    Host* client = (Host*)cli;
    Request request;
    struct stat statbuff;
    char type[50] = {0};
    char pathaux[40] = "."; // Only files in current directory

    request = receiveData(client->sockfd);

    printf("%s\n", request.path);
    if(strcmp(request.method, "GET")){
        perror("Metodo diferente de GET\n");
        return;
    }

    if(!strcmp(request.path, "/"))
        strcpy(request.path, "/index.html");
    
    strcat(pathaux, request.path);
    strcpy(request.path, pathaux);

    if(stat(request.path, &statbuff) < 0){
        perror("404 NOT FOUND\n");
        return;
    }

    size_t length = S_ISREG(statbuff.st_mode) ? statbuff.st_size : -1;

    sendHeader(client->sockfd, 200, "OK", (char*)get_mime_type(request.path), length, request.protocol);
    sendData(client->sockfd, request.path);

    close(client->sockfd);
    client->using = false;
    pthread_exit(NULL);
}

bool acceptConnection(Host *server, Host *client){
    memset(&client->address, 0, sizeof(client->address));
    socklen_t sizeAdress = sizeof(server->address);

    client->sockfd = accept(server->sockfd, (struct sockaddr *) &client->address, &sizeAdress);
    client->using = true;
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
    
    pid_t pid, sid;
    pid = fork();

    if (pid < 0)
        exit(EXIT_FAILURE);

    else if (pid > 0)
    {
        printf("Child PID: %d\n", pid);
        exit(EXIT_SUCCESS);
    }

    umask(0);
    sid = setsid();

    Host clients[N_MAX_CLIENTS];
    pthread_t threads[N_MAX_CLIENTS];
    int indexthread = 0;

    for(int i=0 ; i < N_MAX_CLIENTS ; i++)
        clients[i].using = false;

    memset(clients, 0, sizeof(clients));

    while(1){
        if(clients[indexthread].using)
            indexthread = (indexthread + 1) % N_MAX_CLIENTS;
        else{
            if(acceptConnection(&server, &clients[indexthread])){
                pthread_create(&threads[indexthread], NULL, (void*)handleConnection, (void*)&clients[indexthread]);
            }
        }

    }

    return 0;
}