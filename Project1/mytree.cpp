#include <string>
#include <vector>
#include <iostream>
#include <filesystem>
#include <cstdlib>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>

namespace fs = std::filesystem;

static int files = 0;
static int dirs = 0;

// c++ version
// void walkFileTree(fs::path cwd, int depth) {

//     for (const auto & dirEntry : fs::directory_iterator {cwd, fs::directory_options::skip_permission_denied}) {
//         try {
//             // Would fuse the ifs but for some reason it wasn't short circuiting
//             // So it was like dereferencing a null pointer
//             if (dirEntry.exists()) {
//                 if (dirEntry.path().filename().string()[0] != '.') {
                    
//                     for (int i = 0; i < depth; i++) {
//                         std::cout << '\t';
//                     }

//                     std::cout << dirEntry.path().filename().string();
                    
//                     if (dirEntry.is_symlink()) {
//                         std::cout << " -> " << fs::read_symlink(dirEntry.path());
//                         std::cout << std::endl;

//                         dirEntry.is_directory() ? dirs++ : files++;
//                     } else if (dirEntry.is_directory()) {
//                         std::cout << std::endl;
//                         dirs++;
//                         walkFileTree(dirEntry.path(), depth + 1);
//                     } else {
//                         std::cout << std::endl;
//                         files++;
//                     }
//                 }
//             }
//         } catch (const fs::filesystem_error & ex) {
//             // some things get past skip_permission_denied
//             // std::cout << "[error opening dir]" << std::endl;
//             std::cout << ex.what() << std::endl;
//         }
//     }
// }

// c version
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
        while ((entry = readdir(dir)) != NULL) {
            if (entry->d_name[0] != '.') {

                for (int i = 0; i < depth; i++) {
                    printf("  ");
                }

                printf("%s", entry->d_name);

                if ((lstat(std::string{path + "/" + entry->d_name}.c_str(), &statbuf) < 0)) {
                    // If I can't get any info about the entry just skip it
                    continue;
                }

                std::string filler = path[path.length() - 1] == '/' ? "" : "/";

                if (S_ISLNK(statbuf.st_mode) != 0) {
                    printf(" -> %s", realpath(std::string {path + filler + entry->d_name}.c_str(), NULL));
                    S_ISDIR(statbuf.st_mode) != 0 ? dirs++ : files++;
                } else if (S_ISDIR(statbuf.st_mode) != 0) {
                    printf("\n");
                    dirs++;
                    walkFileTree(std::string { path + filler + entry->d_name }, depth + 2);
                } else {
                    printf("\n");
                    files++;
                }
            }
        }

        closedir(dir);
    } else {
        printf("%s [error opening dir]\n", path.c_str());
    }
}

int main(int argc, char ** argv) {

    switch (argc) {
    case 1:
        walkFileTree(".", 0);
        break;
    case 2:
        walkFileTree(argv[1], 0);
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