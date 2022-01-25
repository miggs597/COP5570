#include <cstdio>
#include <vector>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <sys/resource.h>

void timer(int argc, char ** argv) {

    char ** commandArgs = new char * [argc - 1];
    
    for (int i = 1; i < argc; i++) {
        commandArgs[i - 1] = argv[i];
    }

    pid_t pid = fork();

    if (pid == 0) {
        execvp(commandArgs[0], commandArgs);
    } else if (pid > 0) {
        int status;
        waitpid(pid, &status, 0);

        struct rusage ru;
        getrusage(RUSAGE_CHILDREN, &ru);

        printf("%ld.%du ", ru.ru_utime.tv_sec, ru.ru_utime.tv_usec);
        printf("%ld.%ds ", ru.ru_stime.tv_sec, ru.ru_stime.tv_usec);
        printf("\n");

        delete [] commandArgs;
    } else {
        // could not fork
        delete [] commandArgs;
        exit(EXIT_FAILURE);
    }
}

int main(int argc, char ** argv) {

    if (argc < 2) {
        printf("usage: mytime.x cmd [args]\n");
        exit(EXIT_FAILURE);
    }

    timer(argc, argv);

    return 0;
}