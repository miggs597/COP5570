#include <vector>
#include <string>
#include <cstdio>
#include <cstdlib>
#include <unistd.h>
#include <dirent.h>
#include <time.h>
#include <sys/stat.h>
#include <sys/types.h>

/*

recurse dirs
Collect all mod times of regualr files
make sure to ignore symlinks

push back all of those times

Then sort them into buckets of hour intervals
*/
void mods(std::string path) {

    DIR * dir;
    struct stat statbuf;

    char buf[100];
    struct timespec current;
    timespec_get(&current, TIME_UTC);
    printf("%ld\n", current.tv_sec);
    strftime(buf, sizeof(buf), "%D %T", localtime(&current.tv_sec));
    printf("CURRENT TIMEasdf: %s\n", buf);
    
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
        std::vector<struct dirent *> entries;
        while ((entry = readdir(dir)) != NULL) {
            if (entry->d_name[0] != '.') {
                entries.push_back(entry);
            }
        }

        for (const auto & i : entries) {
            printf("%s\n", i->d_name);
        }

    } else {
        printf("%s [error opening dir]\n", path.c_str());
        exit(EXIT_FAILURE);
    }
}

int main(int argc, char ** argv) {

    switch (argc) {
    case 1:
        mods(".");
        break;
    case 2:
        mods(argv[1]);
        break;
    default:
        printf("usage: mymtime.x [dir]\n");
        exit(EXIT_FAILURE);
    }

    return 0;
}
