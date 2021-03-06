#include <vector>
#include <string>
#include <fstream>
#include <filesystem>
#include <unordered_map>
#include <unordered_set>

#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <limits.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>

namespace fs = std::filesystem;

pthread_mutex_t loginMutex = PTHREAD_MUTEX_INITIALIZER;

// The set just prevents multiple logins from a
// client that is currently logged in
std::unordered_set<int> activeSockets;
std::unordered_map<std::string, std::pair<int, pthread_mutex_t>> activeUsers;

void login(std::string info, int socketFD) {
    // Using a lock here should prevent two clients
    // from logging in with the same name
    pthread_mutex_lock(&loginMutex);

    if (!activeSockets.contains(socketFD)) {
        int usernameDelim = info.find(" ");
        
        std::string username = info.substr(usernameDelim + 1);
        pthread_mutex_t t = PTHREAD_MUTEX_INITIALIZER;

        activeUsers.try_emplace(username, std::make_pair(socketFD, t));

        activeSockets.emplace(socketFD);
    }

    pthread_mutex_unlock(&loginMutex);
}

void logout(int socketFD) {
    if (activeSockets.contains(socketFD)) {
        std::string userToLogout;

        // Don't have a bimap
        // really just hoping my values are unique :)
        for (const auto & i : activeUsers) {
            if (i.second.first == socketFD) {
                userToLogout = i.first;
            }
        }

        activeUsers.erase(userToLogout);
        activeSockets.erase(socketFD);
    }
}

void * sendMessageInParallel(void * message) {
    auto m = *(std::pair<std::string, std::pair<int, pthread_mutex_t>> *) message;
    delete (std::pair<std::string, std::pair<int, pthread_mutex_t>> *) message;

    pthread_mutex_lock(&m.second.second);
    write(m.second.first, m.first.c_str(), m.first.length());
    pthread_mutex_unlock(&m.second.second);

    return NULL;
}

void sendMessage(std::string input, int socketFD) {
    std::string sendersName;

    for (const auto & i : activeUsers) {
        if (i.second.first == socketFD) {
            sendersName = i.first;
        }
    }

    int delim = input.find(" ");
    std::string rawMessage = input.substr(delim + 1);

    if (rawMessage.starts_with("@")) {
        delim = rawMessage.find(" ");
        std::string targetUserName = rawMessage.substr(1, delim - 1);

        if (activeUsers.contains(targetUserName)) {
            auto targetUserSocketFD = activeUsers.at(targetUserName);

            rawMessage = rawMessage.substr(delim + 1);
            rawMessage = sendersName + " >> " + rawMessage;
            
            pthread_mutex_lock(&targetUserSocketFD.second);
            write(targetUserSocketFD.first, rawMessage.c_str(), rawMessage.length());
            pthread_mutex_unlock(&targetUserSocketFD.second);
        }
    } else {
        // Create a new array of pthreads that will be used to send
        // the message to the whole server
        int threadIndex = 0;
        pthread_t * t = new pthread_t[activeUsers.size() - 1];

        for (const auto & user : activeUsers) {
            // Don't send yourself your the message
            if (user.second.first != socketFD) {
                std::string formattedMessage = sendersName + " >> " + rawMessage;

                auto * message = new std::pair<std::string, std::pair<int, pthread_mutex_t>>;
                message->first = formattedMessage;
                message->second = user.second;

                pthread_create(&t[threadIndex++], NULL, &sendMessageInParallel, (void *)message); 
            }
        }

        for (unsigned int i = 0; i < activeUsers.size() - 1; i++) {
            pthread_join(t[i], NULL);
        }

        delete [] t;
    }
}

void handleInput(std::string input, int socketFD) {
    if (input.starts_with("login")) {
        login(input, socketFD);
    } else if (input.starts_with("logout")) {
        logout(socketFD);
    } else if (input.starts_with("chat")) {
        if (activeSockets.contains(socketFD)) {
            sendMessage(input, socketFD);
        } else {
            std::string notLoggedIn = "login in to use chat";
            write(socketFD, notLoggedIn.c_str(), notLoggedIn.length()); 
        }
    } else if (input.starts_with("print")) {
        for (const auto & i : activeUsers) {
            printf("user: %s, socket: %d\n", i.first.c_str(), i.second.first);
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

    auto handelSIGINT = [](int) { 

        for (auto & user : activeUsers) {
            close(user.second.first);
        }

        exit(EXIT_SUCCESS);
    };

    signal(SIGINT, handelSIGINT);

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

    struct hostent * hostEntry;
    char hostname[64];
    gethostname(hostname, 64);
    hostEntry = gethostbyname(hostname);

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
    
    printf("ip: %s@%s, port: %d\n", hostname, inet_ntoa(*(struct in_addr *)hostEntry->h_addr_list[0]), htons(serv_addr.sin_port));

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