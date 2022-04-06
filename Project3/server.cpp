#include <vector>
#include <string>
#include <fstream>
#include <filesystem>
#include <unordered_map>

#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <limits.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <netinet/in.h>

namespace fs = std::filesystem;

void * handleConnection(void * arg) {
    int socketFD = *((int *) arg);
    delete (int *) arg;

    ssize_t n;
    char buf[1024];

    pthread_detach(pthread_self());

    while ((n = read(socketFD, buf, 1024)) > 0) {
        buf[n] = '\0';

        printf("SERVER GOT MESSAGE: %s\n", buf);
        write(socketFD, buf, strlen(buf));
    }

    close(socketFD);
    return NULL;
}

int main(int argc, char ** argv) {

    if (argc != 2) {
        printf("usage: server.x configration_file\n");
        exit(EXIT_FAILURE);
    }

    if (!fs::exists(argv[1])) {
        printf("Could not load '%s'\n", argv[1]); 
        exit(EXIT_FAILURE);
    }
    
    std::string config;
    std::getline(std::ifstream(argv[1]), config);

    // port: number
    // finds the index of ':' and grabs the substr till the end of the line 
    // from there it is converted to an int
    int port = atoi(config.substr(config.find(":") + 1).c_str());

    int socketFD, cliSocketFD, * sockPTR;
    socklen_t socklen;
    struct sockaddr_in serv_addr, cli_addr;

    char hostname[64];
    gethostname(hostname, 64);

    if ((socketFD = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("socket error\n");
        exit(EXIT_FAILURE);
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(port);

    if (bind(socketFD, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        printf("bind error\n");
        exit(EXIT_FAILURE);
    }
    
    socklen = sizeof(serv_addr);
    if (getsockname(socketFD, (struct sockaddr *)&serv_addr, &socklen) < 0) {
		printf("name error\n");
		exit(EXIT_FAILURE);
	}
    
    printf("ip: %s@%s, port: %d\n", hostname, inet_ntoa(serv_addr.sin_addr), htons(serv_addr.sin_port));

    if (listen(socketFD, 5) < 0) {
        printf("listen error\n");
        exit(EXIT_FAILURE);
    }

    while (1) {

        pthread_t clientThread;

        socklen = sizeof(cli_addr);
        cliSocketFD = accept(socketFD, (struct sockaddr *)&cli_addr, &socklen);

        sockPTR = (int *) new int;
        *sockPTR = cliSocketFD;

        pthread_create(&clientThread, NULL, &handleConnection, (void *)sockPTR);
    }

    return 0;
}