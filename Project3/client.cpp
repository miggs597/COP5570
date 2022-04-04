#include <netdb.h>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>

int main(int argc, char ** argv) {

    if (argc != 3) {
        printf("usage: 'client.x ip port'\n");
        exit(EXIT_FAILURE); 
    }

    return 0;
}