/*** Includes ***/
#include <ctype.h>      // iscntrl(), isdigit() etc. (used later for key handling)
#include <errno.h>      // errno values like EAGAIN for non-blocking read()
#include <stdio.h>      // perror() for readable error messages
#include <stdlib.h>     // exit(), atexit()
#include <termios.h>    // terminal control (raw vs canonical mode)
#include <unistd.h>     // read(), write(), STDIN_FILENO, STDOUT_FILENO
#include <sys/ioctl.h>  // ioctl(), TIOCGWINSZ for terminal size

/*** Macros ***/
#define CTRL_KEY(k) ((k) & 0x1f)   // Maps Ctrl+<key> to its ASCII control code

/*** Global Data ***/
struct editorConfig {
    int screenrows;                // Number of rows in the terminal
    int screencols;                // Number of columns in the terminal
    struct termios orig_termios;   // Original terminal settings (to restore later)
};

struct editorConfig E;             // Global editor state

/*** Terminal Control ***/
void die(const char *s) {
    write(STDOUT_FILENO, "\x1b[2J", 4);  // Clear entire screen
    write(STDOUT_FILENO, "\x1b[H", 3);   // Move cursor to top-left
    perror(s);                           // Print error message based on errno
    exit(1);                             // Exit with failure status
}

void disableRawMode(void) {
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &E.orig_termios); // Restore original terminal state
}

void enableRawMode(void) {
    tcgetattr(STDIN_FILENO, &E.orig_termios); // Read current terminal attributes
    atexit(disableRawMode);                   // Ensure terminal is restored on exit

    struct termios raw = E.orig_termios;      // Make a modifiable copy

    raw.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON); // Disable special input processing
    raw.c_oflag &= ~(OPOST);                                   // Disable output post-processing
    raw.c_cflag |= (CS8);                                     // Force 8-bit characters
    raw.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);          // Disable echo, canonical mode & signals

    raw.c_cc[VMIN] = 0;       // read() returns immediately
    raw.c_cc[VTIME] = 1;      // read() waits up to 100ms

    tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw); // Apply raw mode settings
}

char editorReadKey(void) {
    int nread;
    char c;

    while ((nread = read(STDIN_FILENO, &c, 1)) != 1) { // Read single byte
        if (nread == -1 && errno != EAGAIN)            // Ignore temporary unavailability
            die("read");
    }
    return c;                                          // Return pressed key
}

int getCursorPosition(int *rows, int *cols) {
    char buf[32];                                      // Buffer for terminal response
    unsigned int i = 0;

    write(STDOUT_FILENO, "\x1b[6n", 4);                // Ask terminal for cursor position

    while (i < sizeof(buf) - 1) {                      // Read response byte-by-byte
        if (read(STDIN_FILENO, &buf[i], 1) != 1) break;
        if (buf[i] == 'R') break;                      // End of response
        i++;
    }

    buf[i] = '\0';                                     // Null-terminate string

    if (buf[0] != '\x1b' || buf[1] != '[') return -1;  // Validate escape sequence
    if (sscanf(&buf[2], "%d;%d", rows, cols) != 2)     // Parse row;column
        return -1;

    return 0;                                          // Success
}

int getWindowSize(int *rows, int *cols) {
    struct winsize ws;                                 // Kernel-defined terminal size struct

    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == -1 || ws.ws_col == 0) {
        write(STDOUT_FILENO, "\x1b[999C\x1b[999B", 12); // Move cursor to bottom-right
        return getCursorPosition(rows, cols);          // Query actual cursor position
    } else {
        *cols = ws.ws_col;                             // Store terminal width
        *rows = ws.ws_row;                             // Store terminal height
        return 0;
    }

}
/***Append Buffer ***/
struct abuf{
    char *b;
    int len;
    };
#define ABUF_INIT {NULL, 0};

/*** Input Handling ***/
void editorProcessKeypress(void) {
    char c = editorReadKey();                          // Read one keypress

    switch (c) {
        case CTRL_KEY('q'):                            // Ctrl-Q pressed
            write(STDOUT_FILENO, "\x1b[2J", 4);        // Clear screen
            write(STDOUT_FILENO, "\x1b[H", 3);         // Move cursor home
            exit(0);                                  // Exit cleanly
    }
}

/*** Output Handling ***/
void editorDrawRows(void) {
    int y;

    for (y = 0; y < E.screenrows; y++) {               // Loop through visible rows
        write(STDOUT_FILENO, "~", 1);                  // Placeholder for empty line
        if (y < E.screenrows - 1)
            write(STDOUT_FILENO, "\r\n", 2);           // Move to next line
    }
}

void editorRefreshScreen(void) {
    write(STDOUT_FILENO, "\x1b[2J", 4);                // Clear screen
    write(STDOUT_FILENO, "\x1b[H", 3);                 // Move cursor home
    editorDrawRows();                                  // Draw editor contents
    write(STDOUT_FILENO, "\x1b[H", 3);                 // Reset cursor position
}

/*** Init ***/
void initEditor(void) {
    if (getWindowSize(&E.screenrows, &E.screencols) == -1)
        die("getWindowSize");                          // Abort if terminal size fails
}

int main(void) {
    enableRawMode();                                   // Enable raw input mode
    initEditor();                                      // Initialize editor state

    while (1) {
        editorRefreshScreen();                         // Redraw screen
        editorProcessKeypress();                       // Handle input
    }

    return 0;
}

