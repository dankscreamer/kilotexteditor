/*** Includes ***/
#include <ctype.h>      // character classification (iscntrl, isdigit, etc.)
#include <errno.h>      // errno values like EAGAIN for non-blocking read
#include <stdio.h>      // perror(), snprintf()
#include <stdlib.h>     // exit(), atexit(), malloc/realloc/free
#include <termios.h>    // terminal control (raw vs canonical mode)
#include <unistd.h>     // read(), write(), STDIN_FILENO, STDOUT_FILENO
#include <sys/ioctl.h>  // ioctl(), TIOCGWINSZ for terminal size
#include <string.h>     // memcpy()

/*** Macros ***/
#define CTRL_KEY(k) ((k) & 0x1f)   // maps Ctrl+<key> to ASCII control code
#define KILO_VERSION "0.0.1"       // editor version string
enum editorKey {
  ARROW_LEFT = 1000,
  ARROW_RIGHT,
  ARROW_UP,
  ARROW_DOWN,
  DEL_KEY,
  HOME_KEY,
  END_KEY,
  PAGE_UP,
  PAGE_DOWN
};
/*** Global Data ***/
struct editorConfig {
    int cx,cy;                     // cursor coordinates
    int screenrows;                // number of terminal rows
    int screencols;                // number of terminal columns
    struct termios orig_termios;   // original terminal settings backup
};

struct editorConfig E;             // global editor state

/*** Terminal Control ***/
void die(const char *s) {           // fatal error handler
    write(STDOUT_FILENO, "\x1b[2J", 4);  // clear entire screen
    write(STDOUT_FILENO, "\x1b[H", 3);   // move cursor to top-left
    perror(s);                           // print error message
    exit(1);                             // exit with failure
}

void disableRawMode(void) {          // restore terminal settings
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &E.orig_termios); // reset terminal mode
}

void enableRawMode(void) {            // enable raw input mode
    tcgetattr(STDIN_FILENO, &E.orig_termios); // save current terminal state
    atexit(disableRawMode);                   // restore terminal on exit

    struct termios raw = E.orig_termios;      // copy terminal attributes

    raw.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON); // disable input processing
    raw.c_oflag &= ~(OPOST);                                 // disable output processing
    raw.c_cflag |= (CS8);                                   // force 8-bit characters
    raw.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);        // disable echo, canonical mode, signals

    raw.c_cc[VMIN] = 0;       // read() returns immediately
    raw.c_cc[VTIME] = 1;      // read() waits up to 100ms

    tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw); // apply raw mode
}

int editorReadKey(void) {
  int nread;
  char c;
  while ((nread = read(STDIN_FILENO, &c, 1)) != 1) {
    if (nread == -1 && errno != EAGAIN) die("read");
  }
  if (c == '\x1b') {
    char seq[3];
    if (read(STDIN_FILENO, &seq[0], 1) != 1) return '\x1b';
    if (read(STDIN_FILENO, &seq[1], 1) != 1) return '\x1b';
    if (seq[0] == '[') {
      if (seq[1] >= '0' && seq[1] <= '9') {
        if (read(STDIN_FILENO, &seq[2], 1) != 1) return '\x1b';
        if (seq[2] == '~') {
          switch (seq[1]) {
            case '1': return HOME_KEY;
            case '4': return END_KEY;
            case '3': return DEL_KEY;
            
            case '5': return PAGE_UP;
            case '6': return PAGE_DOWN;
            case '7': return HOME_KEY;
            case '8': return END_KEY;
          }
        }
         } else if (seq[0] == 'O') {
         switch (seq[1]) {
        case 'H': return HOME_KEY;
        case 'F': return END_KEY;
      }
      } else {
        switch (seq[1]) {
          case 'A': return ARROW_UP;
          case 'B': return ARROW_DOWN;
          case 'C': return ARROW_RIGHT;
          case 'D': return ARROW_LEFT;
          case 'H': return HOME_KEY;
          case 'F': return END_KEY;
        }
      }
    }
    return '\x1b';
  } else {
    return c;
  }
}

int getCursorPosition(int *rows, int *cols) { // get cursor position from terminal
    char buf[32];                              // response buffer
    unsigned int i = 0;                        // buffer index

    write(STDOUT_FILENO, "\x1b[6n", 4);        // request cursor position

    while (i < sizeof(buf) - 1) {              // read response
        if (read(STDIN_FILENO, &buf[i], 1) != 1) break; // stop on failure
        if (buf[i] == 'R') break;               // 'R' marks end of response
        i++;
    }

    buf[i] = '\0';                             // null-terminate response

    if (buf[0] != '\x1b' || buf[1] != '[') return -1; // validate escape sequence
    if (sscanf(&buf[2], "%d;%d", rows, cols) != 2) return -1; // parse row/col

    return 0;                                  // success
}

int getWindowSize(int *rows, int *cols) { // get terminal size
    struct winsize ws;                     // window size struct

    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == -1 || ws.ws_col == 0) { // ioctl failed
        write(STDOUT_FILENO, "\x1b[999C\x1b[999B", 12); // move cursor bottom-right
        return getCursorPosition(rows, cols);          // infer size via cursor
    } else {
        *cols = ws.ws_col;                  // store columns
        *rows = ws.ws_row;                  // store rows
        return 0;
    }
}

/*** Append Buffer ***/
struct abuf {
    char *b;                                // pointer to buffer
    int len;                                // buffer length
};

#define ABUF_INIT {NULL, 0}                 // initialize empty buffer

void abAppend(struct abuf *ab, const char *s, int len) { // append data to buffer
    char *new = realloc(ab->b, ab->len + len); // resize buffer
    if (new == NULL) return;                    // abort on allocation failure
    memcpy(&new[ab->len], s, len);              // copy new data
    ab->b = new;                                // update buffer pointer
    ab->len += len;                             // update length
}

void abFree(struct abuf *ab) {              // free append buffer
    free(ab->b);                             // release memory
}

/*** Input Handling ***/



void editorMoveCursor(int key) {
  switch (key) {
    case ARROW_LEFT:
      if (E.cx != 0) {
        E.cx--;
      }
      break;
    case ARROW_RIGHT:
      if (E.cx != E.screencols - 1) {
        E.cx++;
      }
      break;
    case ARROW_UP:
      if (E.cy != 0) {
        E.cy--;
      }
      break;
    case ARROW_DOWN:
      if (E.cy != E.screenrows - 1) {
        E.cy++;
      }
      break;
  }
}







void editorProcessKeypress(void) {           // handle keypress
    int  c = editorReadKey();                // read key

    switch (c) {
        case CTRL_KEY('q'):                  // Ctrl-Q pressed
            write(STDOUT_FILENO, "\x1b[2J", 4); // clear screen
            write(STDOUT_FILENO, "\x1b[H", 3);  // move cursor home
            exit(0);                            // exit editor
            break;
        case HOME_KEY:
            E.cx = 0;
             break;
        case END_KEY:
             E.cx = E.screencols - 1;
             break;
            

       case ARROW_UP:
       case ARROW_DOWN:
       case ARROW_LEFT:
       case ARROW_RIGHT:
        editorMoveCursor(c);
        break;


    }
   
}

/*** Output Handling ***/
void editorDrawRows(struct abuf *ab) {        // draw editor rows
    int y;                                   // row index

    for (y = 0; y < E.screenrows; y++) {     // loop through rows
        if (y == E.screenrows / 3) {         // draw welcome message
            char welcome[80];                // welcome buffer
            int welcomelen = snprintf(welcome, sizeof(welcome),
                "Kilo editor -- version %s", KILO_VERSION); // format message
            if (welcomelen > E.screencols) welcomelen = E.screencols; // truncate
            int padding = (E.screencols - welcomelen) / 2; // center text
            if (padding) { abAppend(ab, "~", 1); padding--; } // left tilde
            while (padding--) abAppend(ab, " ", 1); // add spaces
            abAppend(ab, welcome, welcomelen); // draw message
        } else {
            abAppend(ab, "~", 1);             // draw tilde on empty lines
        }
        abAppend(ab, "\x1b[K", 3);            // clear rest of line
        if (y < E.screenrows - 1) abAppend(ab, "\r\n", 2); // newline
    }
}

void editorRefreshScreen(void) {              // redraw entire screen
    struct abuf ab = ABUF_INIT;               // create append buffer

    abAppend(&ab, "\x1b[?25l", 6);             // hide cursor
    abAppend(&ab, "\x1b[H", 3);                // move cursor home

    editorDrawRows(&ab);                       // draw rows

    char buf[32];
    snprintf(buf, sizeof(buf),"\x1b[%d;%dH",E.cy+1,E.cx+1);
    abAppend(&ab,buf,strlen(buf));




                              
    abAppend(&ab, "\x1b[?25h", 6);             // show cursor

    write(STDOUT_FILENO, ab.b, ab.len);        // write buffer to terminal
    abFree(&ab);                               // free buffer
}

/*** Init ***/
void initEditor(void) {                        // initialize editor
    E.cx=0;
    E.cy=0;
    if (getWindowSize(&E.screenrows, &E.screencols) == -1) // get terminal size
        die("getWindowSize");                  // abort on failure
}

int main(void) {                               // program entry point
    enableRawMode();                           // enable raw terminal mode
    initEditor();                              // initialize editor state

    while (1) {                                // main loop
        editorRefreshScreen();                 // redraw screen
        editorProcessKeypress();               // handle input
    }

    return 0;                                  // unreachable
}

