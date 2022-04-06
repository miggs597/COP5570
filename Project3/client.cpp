#include <fstream>
#include <iostream>
#include <filesystem>

#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define MAXBUF 1024

namespace fs = std::filesystem;

void * readThread(void * arg) {

    int n;
    int socketFD = *(int *) arg;
    char buf[MAXBUF] = {0};

    delete (int *) arg;

    pthread_detach(pthread_self());

    while (true) {

        n = read(socketFD, buf, MAXBUF);
        if (n > 0) {
            buf[n] = 0;
            printf("this is echo %s\n", buf);
        } else {
            switch (n) {
            case 0:
                printf("server closed\n");
                break;
            default:
                printf("Some error");
                break;
            }

            close(socketFD);
        }
    }
    return NULL;
}

int main(int argc, char ** argv) {

    if (argc != 2) {
        printf("usage: 'client.x config'\n");
        exit(EXIT_FAILURE); 
    }

    if (!fs::exists(argv[1])) {
        printf("Could not load '%s'\n", argv[1]); 
        exit(EXIT_FAILURE);
    }
    
    std::string config;
    std::getline(std::ifstream(argv[1]), config);

    int delimIndex = config.find(" ");
    std::string servhost = config.substr(0, delimIndex);
    std::string servport = config.substr(delimIndex + 1);

    int socketFD, rv, flag;
    struct addrinfo hints, *res;
    pthread_t tid;

    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    if ((rv = getaddrinfo(servhost.c_str(), servport.c_str(), &hints, &res)) != 0) {
        printf("Wrong address info\n");
        exit(EXIT_FAILURE);
    }

    flag = 0;

    do {
        socketFD = socket(res->ai_family, res->ai_socktype, res->ai_protocol);

        if (socketFD < 0) {
            continue;
        }

        if (connect(socketFD, res->ai_addr, res->ai_addrlen) == 0) {
            flag = 1; 
            break;
        }

        close(socketFD);

    } while ((res = res->ai_next) != NULL);

    freeaddrinfo(res);

    if (!flag) {
        printf("Could not connect\n");
        exit(EXIT_FAILURE);
    }

    int * sockPTR = new int;
    *sockPTR = socketFD;
    pthread_create(&tid, NULL, &readThread, (void *) sockPTR);

    std::string line;
    while (std::getline(std::cin, line)) {
        if (line == "exit") {
            close(socketFD);
            break;
        } else {
            write(socketFD, line.c_str(), line.length());
        }
    }


    return 0;
}