#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>

int main(int argc, char ** argv) {

    pthread_mutex_t mu;
    pthread_mutex_init(&mu, NULL);

    return 0;
}