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

typedef struct {
    int sockfd;
    struct sockaddr_in address;
} Host;



Host createServer(int port, char *address){
    Host host;
    host.sockfd = socket(AF_INET, SOCK_STREAM, 0);

    if(host.sockfd < 0){
        printf("Não deu certo.\n");
        exit(0);
    }
    else
        printf("Socket criado\n");
    
    memset(&host.address, 0, sizeof(host.address));

    host.address.sin_family = AF_INET;
    host.address.sin_port = htons(port);
    host.address.sin_addr.s_addr = inet_addr(address);

    return host;
}



int main(int argc, char const *argv[])
{
    Host server = createServer(PORT, IP_ADDR);
    return 0;


}
