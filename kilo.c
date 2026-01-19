#include <stdlib.h>
#include <termios.h>
#include <unistd.h>

void enableRawMode(void) {
    struct termios raw;

    tcgetattr(STDIN_FILENO, &raw);
    raw.c_lflag &= ~(ECHO | ICANON);
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
}

int main(void) {
    enableRawMode();

    char c;
    while (read(STDIN_FILENO, &c, 1) == 1) {
        if (c == 'q') break;
        write(STDOUT_FILENO, &c, 1);
    }

    return 0;
}

