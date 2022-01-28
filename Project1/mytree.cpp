#include <string>
#include <vector>
#include <iostream>
#include <algorithm>
#include <filesystem>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>

namespace fs = std::filesystem;

static int files = 0;
static int dirs = 0;

void walkFileTree(fs::path cwd, int depth) {
    std::vector<fs::directory_entry> test {fs::begin(fs::directory_iterator {cwd, fs::directory_options::skip_permission_denied}), fs::end(fs::directory_iterator {cwd, fs::directory_options::skip_permission_denied})};
    // case sensitive :(
    std::sort(test.begin(), test.end(), [](fs::directory_entry lhs, fs::directory_entry rhs) { return lhs.path().filename().string() < rhs.path().filename().string(); });

    for (const auto & dirEntry : test) {
        try {
            // Would fuse the ifs but for some reason it wasn't short circuiting
            // So it was like dereferencing a null pointer
            if (dirEntry.exists()) {
                if (dirEntry.path().filename().string()[0] != '.') {
                    
                    for (int i = 0; i < depth; i++) {
                        std::cout << '\t';
                    }

                    std::cout << dirEntry.path().filename().string();
                    
                    if (dirEntry.is_symlink()) {
                        std::cout << " -> " << fs::read_symlink(dirEntry.path());
                        std::cout << std::endl;

                        dirEntry.is_directory() ? dirs++ : files++;
                    } else if (dirEntry.is_directory()) {
                        std::cout << std::endl;
                        dirs++;
                        walkFileTree(dirEntry.path(), depth + 1);
                    } else {
                        std::cout << std::endl;
                        files++;
                    }
                }
            }
        } catch (const fs::filesystem_error & ex) {
            // some things get past skip_permission_denied
            // std::cout << "[error opening dir]" << std::endl;
            // std::cout << ex.what() << std::endl;
        }
    }
}

void walkFileTree(std::string path, int depth) {

    DIR * dir;
    struct stat statbuf;

    if ((lstat(path.c_str(), &statbuf) < 0)) {
        printf("lstat error\n");
        exit(EXIT_FAILURE);
    }

    if (S_ISDIR(statbuf.st_mode) == 0) {
        printf("'%s' is not a directory\n", path.c_str());
    }

    if ((dir = opendir(path.c_str())) != NULL) {

        struct dirent * entry;
        std::vector<struct dirent *> dirEntries;
        while ((entry = readdir(dir)) != NULL) {
            if (entry->d_name[0] != '.') {
                dirEntries.push_back(entry);
            }
        }

        std::sort(dirEntries.begin(), dirEntries.end(), [](struct dirent * lhs, struct dirent * rhs) { return std::string{lhs->d_name} < std::string{rhs->d_name}; });

        for (auto & i : dirEntries) {
            printf("%s\n", i->d_name);
        }

    } else {
        printf("couldn't open '%s'\n", path.c_str());
        exit(EXIT_FAILURE);
    }

}

int main(int argc, char ** argv) {

    switch (argc) {
    case 1:
        walkFileTree(fs::path{"."}, 0);
        break;
    case 2:
        walkFileTree(std::string{argv[1]}, 0);
        break;
    default:
        printf("usage: mytree.x [dir]\n");
        exit(EXIT_FAILURE);
        break;
    }

    std::string dPlural = dirs == 1 ? "directory" : "directories"; 
    std::string fPlural = files == 1 ? "file" : "files";
    printf("%d %s, %d %s\n", dirs, dPlural.c_str(), files, fPlural.c_str());
    return 0;
}
