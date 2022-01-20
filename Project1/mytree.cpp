#include <string>
#include <vector>
#include <iostream>
#include <algorithm>
#include <filesystem>

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
                        dirs++;
                    } else if (dirEntry.is_directory() && !dirEntry.is_symlink()) {
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
