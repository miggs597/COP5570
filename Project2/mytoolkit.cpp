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
std::string command {};

int main(int argc, char ** argv) {

    command.reserve(100);
    // struct termios term;
    // tcgetattr(fileno(stdin), &term);

    // term.c_lflag &= ~ECHO;
    // tcsetattr(fileno(stdin), 0, &term);

    return 0;
}