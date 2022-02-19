#include <string>
#include <vector>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <sstream>
#include <iostream>
#include <filesystem>

#include <unistd.h>
#include <signal.h>
#include <termios.h>

namespace fs = std::filesystem;

int redirectionIn = 0;
int redirectionOut = 0;
std::vector<std::pair<int, int>> pipePairs {};

std::vector<std::vector<char *>> tokenize(const std::string & command) {

    std::vector<std::vector<char *>> completeTokens {};
    std::vector<char *> tokens {};

    std::stringstream splitCommand {command};
    std::string token {};

    int currentIndex = 0;

    while (splitCommand >> token) {
        if (token == "<") {
            redirectionIn = ++currentIndex;
            completeTokens.push_back(tokens);
            tokens.clear();
        } else if (token == ">") {
            redirectionOut = ++currentIndex;
            completeTokens.push_back(tokens);
            tokens.clear();
        } else if (token == "|") {
            pipePairs.push_back({currentIndex, ++currentIndex});
            completeTokens.push_back(tokens);
            tokens.clear();
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

    completeTokens.push_back(tokens);

    return completeTokens;
}

char *** cstringTokens(const std::vector<std::vector<std::string>> & completeTokens) {
    // Will go unused since I am able to access to underlying array of my vector
    // keeping for reference
    char *** cTokens = new char ** [completeTokens.size()];

    for (unsigned int i = 0; i < completeTokens.size(); i++) {
        cTokens[i] = new char * [completeTokens[i].size()];

        for (unsigned int j = 0; j < completeTokens[i].size(); j++) {
            cTokens[i][j] = const_cast<char *>(completeTokens[i][j].c_str());
        }
    }

    // for (unsigned int i = 0; i < completeTokens.size(); i++) {
    //     for (unsigned int j = 0; j < completeTokens[i].size(); j++) {
    //         printf("%s\n", cTokens[i][j]);
    //     }
    // }

    return cTokens;
}

void runCommand(const std::vector<std::vector<char *>> & tokens, const bool & timeout) {

    /*
    A whole lotta of code is going to have to go here
    */
}

int main() {
    
    while (1) {
        printf("$ ");
        fflush(0);

        std::string command {};
        std::getline(std::cin, command);
        auto tokens = tokenize(command);
        
        bool end = false; 

        if (!strcmp(tokens[0][0], "mypwd")) {
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
        } else if (!strcmp(tokens[0][0], "mytimeout")) {
            runCommand(tokens, true);
        } else {
            runCommand(tokens, false);
        }

        for (unsigned int i = 0; i < tokens.size(); i++) {
            for (unsigned int j = 0; j < tokens[i].size(); j++) {
                delete [] tokens[i][j];
            }
        }

        if (end) {
            break;
        }
    }

    return 0;
}