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

namespace fs = std::filesystem;

void * readThread(void * arg) {



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
    int servport = atoi(config.substr(delimIndex + 1).c_str());

    int socketFD;

    if ((socketFD = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("socket error\n");
        exit(EXIT_FAILURE);
    }

    
    return 0;
}