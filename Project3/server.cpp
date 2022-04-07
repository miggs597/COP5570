#include <vector>
#include <string>
#include <fstream>
#include <filesystem>
#include <string_view>
#include <unordered_map>
#include <unordered_set>

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

pthread_mutex_t loginMutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t sendMessageMutex = PTHREAD_MUTEX_INITIALIZER;

// The set just prevents multiple logins from a
// client that is currently logged in
std::unordered_set<int> activeSockets;
std::unordered_map<std::string, int> activeUsers;

void login(std::string info, int socketFD) {

    pthread_mutex_lock(&loginMutex);

    if (!activeSockets.contains(socketFD)) {
        int usernameDelim = info.find(" ");
        std::string username = info.substr(usernameDelim + 1);
        activeUsers.try_emplace(username, socketFD);

        activeSockets.emplace(socketFD);
    }

    pthread_mutex_unlock(&loginMutex);
}

void logout(int socketFD) {

    pthread_mutex_lock(&loginMutex);

    if (activeSockets.contains(socketFD)) {
        std::string userToLogout;

        // Don't have a bimap
        // really just hoping my values are unique :)
        for (const auto & i : activeUsers) {
            if (i.second == socketFD) {
                userToLogout = i.first;
            }
        }

        activeUsers.erase(userToLogout);
        activeSockets.erase(socketFD);
    }

    pthread_mutex_unlock(&loginMutex);
}

void sendMessage(std::string input, int socketFD) {

    pthread_mutex_lock(&sendMessageMutex);

    std::string sendersName;

    for (const auto & i : activeUsers) {
        if (i.second == socketFD) {
            sendersName = i.first;
        }
    }

    int delim = input.find(" ");
    std::string recipient = input.substr(delim + 1);

    if (recipient.starts_with("@")) {

    } else {
        for (const auto & i : activeUsers) {
            // Don't send yourself your the message
            if (i.second != socketFD) {
                std::string formattedMessage = sendersName + " >> " + recipient;

                write(i.second, formattedMessage.c_str(), formattedMessage.length());
            }
        }
    }

    pthread_mutex_unlock(&sendMessageMutex);
}

void handleInput(std::string input, int socketFD) {
    if (input.starts_with("exit")) {
        close(socketFD);
    } else if (input.starts_with("login")) {
        login(input, socketFD);
    } else if (input.starts_with("logout")) {
        logout(socketFD);
    } else if (input.starts_with("chat")) {
        sendMessage(input, socketFD);
    } else if (input.starts_with("print")) {
        for (const auto & i : activeUsers) {
            printf("user: %s, socket: %d\n", i.first.c_str(), i.second);
        }
    }
}

void * handleConnection(void * arg) {
    int socketFD = *((int *) arg);
    delete (int *) arg;

    ssize_t n;
    char buf[1024];

    pthread_detach(pthread_self());

    while ((n = read(socketFD, buf, 1024)) > 0) {
        buf[n] = '\0';
        handleInput(std::string {buf}, socketFD);
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