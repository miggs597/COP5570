#include <array>
#include <string>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <filesystem>

#include <unistd.h>
#include <signal.h>
#include <termios.h>

namespace fs = std::filesystem;

std::array<std::string, 1000> history;

int main(int argc, char ** argv) {

    struct termios term;
    tcgetattr(fileno(stdin), &term);

    term.c_lflag &= ~ECHO;
    tcsetattr(fileno(stdin), 0, &term);

    std::string command;
    while (1) {
        printf("%s $ ", fs::current_path().c_str());
        std::getline(std::cin, command);

        printf("GOT : %s\n", command.c_str());



        if (command == "myexit") {
            break;
        }
    }

    return 0;
}