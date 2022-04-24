#include <array>
#include <string>
#include <cstdio>
#include <cstdlib>
#include <unistd.h>
#include <dirent.h>
#include <time.h>
#include <sys/stat.h>
#include <sys/types.h>

struct timespec current;
std::array<int, 24> hourIntervals {0}; 
int oneHour = 3600;

void mods() {

    char buf[256];

    for (const auto & i : hourIntervals) {
        strftime(buf, sizeof(buf), "%c: ", localtime(&current.tv_sec));
        printf("%s", buf);
        printf("%d\n", i);

        current.tv_sec -= oneHour;
    }
}

void walkFileTree(std::string path) {

    DIR * dir;
    struct stat statbuf;

    if ((lstat(path.c_str(), &statbuf) < 0)) {
        printf("lstat error\n");
        exit(EXIT_FAILURE);
    }

    if (S_ISDIR(statbuf.st_mode) == 0) {
        printf("'%s' is not a directory\n", path.c_str());
        exit(EXIT_FAILURE);
    }

    if ((dir = opendir(path.c_str())) != NULL) {

        struct dirent * entry;
        while ((entry = readdir(dir)) != NULL) {
            if (entry->d_name[0] != '.') {

                std::string filler = path[path.length() - 1] == '/' ? "" : "/";

                if ((lstat(std::string{path + filler + entry->d_name}.c_str(), &statbuf) < 0)) {
                    // If I can't get any info about the entry just skip it
                    continue;
                }

                if (S_ISREG(statbuf.st_mode) != 0) {
                    for (unsigned int i = 0; i < hourIntervals.size(); i++) {
                        // Starts at the current hour interval and then moves backwards in time trying to find if it fits in the last 24 hours
                        if (statbuf.st_mtim.tv_sec <= current.tv_sec - (oneHour * i) && statbuf.st_mtim.tv_sec > current.tv_sec - (oneHour * (i + 1))) {
                            hourIntervals[i] += 1;
                            break;
                        }
                    }
                }

                // Ignore symlinks to avoid infinite loop
                if (S_ISLNK(statbuf.st_mode) == 0 && S_ISDIR(statbuf.st_mode) != 0) {
                    walkFileTree(std::string {path + filler + entry->d_name});
                }
            }
        }

        closedir(dir);
    } else {
        printf("%s [error opening dir]\n", path.c_str());
    }
}

int main(int argc, char ** argv) {

    timespec_get(&current, TIME_UTC);

    switch (argc) {
    case 1:
        walkFileTree(".");
        mods();
        break;
    case 2:
        walkFileTree(argv[1]);
        mods();
        break;
    default:
        printf("usage: mymtime.x [dir]\n");
        exit(EXIT_FAILURE);
    }

    return 0;
}