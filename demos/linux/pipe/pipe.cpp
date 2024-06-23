
#include <unistd.h>

int main(int argc, char* argv[]) {
    while (true) {
        char buffer[64];
        int ret = read(STDIN_FILENO, buffer, sizeof(buffer));
        if (ret > 0) {
            write(STDOUT_FILENO, buffer, ret);
        } else if (ret == 0) {
            break;
        } else {
            break;
        }
    }
    return 0;
}