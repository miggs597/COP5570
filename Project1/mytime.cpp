#include <chrono>
#include <vector>
#include <unistd.h>
#include <iostream>
#include <algorithm>
#include <sys/wait.h>

void timer(int argc, char ** argv) {

    char ** commandArgs = new char * [argc - 1];
    
    for (int i = 1; i < argc; i++) {
        commandArgs[i - 1] = argv[i];
    }

    // time stuff

    // fork
    pid_t pid = fork();

    if (pid == 0) {
        execv(commandArgs[0], commandArgs);
    } else if (pid > 0) {
        // parent
        int status;
        waitpid(pid, &status, 0);
        std::cout << "Return was: " << status << std::endl;
        
        delete [] commandArgs;
    } else {
        // could not fork
        delete [] commandArgs;
        exit(EXIT_FAILURE);
    }
}

int main(int argc, char ** argv) {

    if (argc < 2) {
        std::cout << "usage: mytime.x cmd [args]" << std::endl;
        exit(EXIT_FAILURE);
    }

    timer(argc, argv);

    return 0;
}