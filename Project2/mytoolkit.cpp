#include <string>
#include <vector>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <sstream>
#include <iostream>
#include <filesystem>

#include <fcntl.h>
#include <unistd.h>
#include <signal.h>
#include <termios.h>
#include <sys/wait.h>
#include <sys/types.h>

namespace fs = std::filesystem;

char * in = NULL;
char * out = NULL;
bool havePipe = false;

std::vector<std::vector<char *>> tokenize(const std::string & command) {
    // You can use a vector with the exec family
    // you just need to pass in the address of the
    // first item in the vector, this works because
    // the standard guarantees the underlying array will be stored
    // continuously. That is also why I push_back NULL at the end. 
    std::vector<std::vector<char *>> completeTokens {};
    std::vector<char *> tokens {};

    std::stringstream splitCommand {command};
    std::string token {};

    while (splitCommand >> token) {
        if (token == "<") {

            tokens.push_back(NULL);
            completeTokens.push_back(tokens);

            splitCommand >> token;
            auto size = token.length() + 1;
            in = new char [size];
            strncpy(in, token.c_str(), size);

            tokens.clear();
        } else if (token == ">") {

            tokens.push_back(NULL);
            completeTokens.push_back(tokens);

            splitCommand >> token;
            auto size = token.length() + 1;
            out = new char [size];
            strncpy(out, token.c_str(), size);

            tokens.clear();
        } else if (token == "|") {
            tokens.push_back(NULL);
            completeTokens.push_back(tokens);
            tokens.clear();

            havePipe = true;
        } else {
            // Need to make a copy because c_str() 
            // points to the same space in memory
            // so if I just push_back without copying 
            // it will change all of the others too
            auto size = token.length() + 1;
            char * temp = new char [size];
            strncpy(temp, token.c_str(), size);
            tokens.push_back(temp);
        }
    }

    if (tokens.size() > 0) {
        tokens.push_back(NULL);
        completeTokens.push_back(tokens);
    }

    return completeTokens;
}

pid_t pid;

void runCommand(const std::vector<std::vector<char *>> & tokens) {

    struct sigaction action;
    // using anonymous function for the handler 
    // no captures, takes in an int that isn't used
    // kills the child process when the alarm fires
    action.sa_handler = [](int) { kill(pid, SIGTERM); };
    sigemptyset(&action.sa_mask);
    action.sa_flags = 0;

    int fd[2];
    int fileDes = 0;

    for (unsigned int i = 0; i < tokens.size(); i++) {
        
        const auto t = tokens[i];

        int firstCommand = 0;
        bool timeout = false;

        if (!strcmp(t[0], "mytimeout")) {
            timeout = true;
            firstCommand = 2;
        }

        if (havePipe) {
            if (pipe(fd) != 0) {
                printf("Broken pipe\n");
                return;
            }
        }

        pid = fork();

        if (pid == 0) {
            if (havePipe) {
                dup2(fileDes, STDIN_FILENO);
                if (i + 1 < tokens.size()) {
                    dup2(fd[1], STDOUT_FILENO);
                }

                close(fd[0]);
            }

            if (in) {
                int fd = open(in, O_RDONLY, 0);
                dup2(fd, STDIN_FILENO);
                close(fd);
            }

            if (out) {
                int fd = open(out, O_WRONLY | O_CREAT, 0644);
                dup2(fd, STDOUT_FILENO);
                close(fd);
            }
            
            char ** command = const_cast<char **>(&t[firstCommand]);
            execvp(command[0], command);
        } else if (pid > 0) {
            if (timeout) {
                sigaction(SIGALRM, &action, 0);
                
                int seconds = atoi(t[1]);
                alarm(seconds);
                
                if (havePipe) {
                    close(fd[1]);
                    fileDes = fd[0];
                }
            }

            waitpid(pid, NULL, 0);

            if (havePipe) {
                close(fd[1]);
                fileDes = fd[0];
            }
        } else {
            printf("fork failed\n");
        }
    }
}

int main() {
    
    auto handelSIGINT = [](int) { printf("\r$ \n"); };
    signal(SIGINT, handelSIGINT);

    while (1) {
        printf("$ ");
        fflush(0);

        std::string command {};
        std::getline(std::cin, command);
        auto tokens = tokenize(command);
        
        bool end = false;
        
        if (std::cin.eof()) {
            // Captures ^D 
            printf("\n");
            end = true;
        } else if (command == "") {
            // segfaults if given an empty string
            // so just continue 
            continue;
        } else if (!strcmp(tokens[0][0], "mypwd")) {
            printf("%s\n", fs::current_path().c_str());
        } else if (!strcmp(tokens[0][0], "mycd")) {
            std::error_code ec;
            fs::current_path(fs::path {tokens[0][1]}, ec);
            
            if (ec.value() != 0) {
                printf("%s\n", ec.message().c_str());
            }
        } else if (!strcmp(tokens[0][0], "myexit")) {
            // I don't simply break here to ensure that
            // all of my dynamically allocated cstrs are freed
            end = true;
        } else {
            runCommand(tokens);
        }

        for (unsigned int i = 0; i < tokens.size(); i++) {
            for (unsigned int j = 0; j < tokens[i].size(); j++) {
                delete [] tokens[i][j];
            }
        }

        if (in != NULL) {
            delete [] in;
            in = NULL;
        }

        if (out != NULL) {
            delete [] out;
            out = NULL;
        }

        havePipe = false;

        if (end) {
            break;
        }
    }

    return 0;
}