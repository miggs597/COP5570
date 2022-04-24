#include <cstdio>
#include <cstdlib>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <sys/times.h>
#include <sys/resource.h>

void timer(int argc, char ** argv) {

    char ** commandArgs = new char * [argc - 1];

    for (int i = 1; i < argc; i++) {
        commandArgs[i - 1] = argv[i];
    }

    clock_t start, end;
    double realTime;
    double ticks;

    if ((start = times(NULL)) == -1) {
        printf("times error\n");
        exit(EXIT_FAILURE);
    }

    pid_t pid = fork();

    if (pid == 0) {
        execvp(commandArgs[0], commandArgs);
    } else if (pid > 0) {
        wait(NULL);
        
        if ((end = times(NULL)) == -1) {
            printf("times error\n");
            exit(EXIT_FAILURE);
        }

        if ((ticks = sysconf(_SC_CLK_TCK)) < 0) {
            printf("sysconf error\n");
            exit(EXIT_FAILURE);
        }

        struct rusage ru;
        getrusage(RUSAGE_CHILDREN, &ru);
        realTime = (end - start) / ticks;

        printf("%-5s %7.3fs\n", "real", realTime);
        printf("%-5s %7.3fs\n", "user", (float) ru.ru_utime.tv_sec + (float) ru.ru_utime.tv_usec / 1000);
        printf("%-5s %7.3fs\n", "sys", (float) ru.ru_stime.tv_sec + (float) ru.ru_stime.tv_usec / 1000);
        
        delete [] commandArgs;
    } else {
        // could not fork
        delete [] commandArgs;
        printf("fork failed\n");
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