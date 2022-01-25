#include <iostream>
#include <vector>
#include <algorithm>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <sys/resource.h>

void timer(int argc, char ** argv) {

    char ** commandArgs = new char * [argc - 1];
    
    for (int i = 1; i < argc; i++) {
        commandArgs[i - 1] = argv[i];
    }

    // time stuff

    // fork
    pid_t pid = fork();

    if (pid == 0) {
        execvp(commandArgs[0], commandArgs);
    } else if (pid > 0) {
        // parent
        int status;
        waitpid(pid, &status, 0);

        struct rusage ru;
        getrusage(RUSAGE_CHILDREN, &ru);

        std::cout << ru.ru_utime.tv_sec << std::endl;
        std::cout << ru.ru_stime.tv_sec << std::endl;

        // Time prints out the command that was timed and it's arguments
        for (int i = 0; commandArgs[i]; i++) 
            std::cout << commandArgs[i] << " ";

        std::cout << std::endl;
        
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