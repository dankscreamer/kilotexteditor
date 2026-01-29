/*** Includes ***/
#include <ctype.h>   // Character classification helpers
#include <errno.h>   // errno and error codes like EAGAIN
#include <stdio.h>   // perror()
#include <stdlib.h>  // exit(), atexit()
#include <termios.h> // Terminal control (raw / canonical mode)
#include <unistd.h>  // read(), STDIN_FILENO

/*** Macros ***/

/*
 * Maps Ctrl + letter to the corresponding ASCII control character.
 *
 * ASCII control characters are created by masking the lower 5 bits.
 * Example:
 *   'q'  = 113 (0b01110001)
 *   0x1f =  31 (0b00011111)
 *   ---------------------
 *   Ctrl-Q = 17
 *
 * This allows us to detect key combinations like Ctrl-Q, Ctrl-C, etc.
 */
#define CTRL_KEY(k) ((k) & 0x1f)

/*** Global Data ***/

/*
 * Stores the original terminal settings.
 * We must restore these before exiting, otherwise the user's shell
 * will remain stuck in raw mode (very bad UX).
 */
struct termios orig_termios;

/*** Terminal Control ***/

/*
 * Fatal error handler.
 *
 * Before printing the error, we:
 *  - Clear the screen
 *  - Move the cursor to the top-left corner
 *
 * This ensures the error message is visible even if the editor
 * was mid-draw when the failure occurred.
 */

void die(const char *s) {
     write(STDOUT_FILENO,"\x1b[2J",4);
    write(STDOUT_FILENO,"\x1b[H",3);

  perror(s);
  exit(1);
}

/*
 * Restores the terminal to its original (canonical) mode.
 * This function is registered with atexit(), so it runs
 * automatically when the program exits.
 */
void disableRawMode(void) {
  if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios) == -1)
    die("tcsetattr");
}

/*
 * Enables raw mode by modifying terminal flags.
 *
 * Raw mode disables:
 *  - Line buffering (no waiting for Enter)
 *  - Echoing typed characters
 *  - Signal generation (Ctrl-C, Ctrl-Z)
 *  - Input/output post-processing
 *
 * This allows us to read each keypress immediately.
 */
void enableRawMode(void) {
  /* Read current terminal attributes */
  if (tcgetattr(STDIN_FILENO, &orig_termios) == -1)
    die("tcgetattr");

  /* Ensure terminal is restored on exit */
  atexit(disableRawMode);

  /* Create a modifiable copy of the original settings */
  struct termios raw = orig_termios;

  /*
   * Input flags:
   *  - BRKINT  : disable Ctrl-Break
   *  - ICRNL   : disable carriage return -> newline translation
   *  - INPCK   : disable input parity checking
   *  - ISTRIP  : disable stripping 8th bit
   *  - IXON    : disable Ctrl-S / Ctrl-Q flow control
   */
  raw.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);

  /*
   * Output flags:
   *  - OPOST : disable output post-processing
   */
  raw.c_oflag &= ~(OPOST);

  /*
   * Control flags:
   *  - CS8 : force 8-bit characters
   */
  raw.c_cflag |= (CS8);

  /*
   * Local flags:
   *  - ECHO   : disable echoing input
   *  - ICANON : disable canonical mode (line buffering)
   *  - IEXTEN : disable Ctrl-V and similar extensions
   *  - ISIG   : disable signals (Ctrl-C, Ctrl-Z)
   */
  raw.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);

  /*
   * Control characters:
   *
   * VMIN  = 0  → read() returns immediately
   * VTIME = 1  → read() waits up to 100ms
   *
   * This makes input non-blocking with a short timeout.
   */
  raw.c_cc[VMIN] = 0;
  raw.c_cc[VTIME] = 1;

  /* Apply the modified settings */
  if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) == -1)
    die("tcsetattr");
}

/*
 * Reads a single keypress from stdin.
 *
 * In raw mode, read() returns as soon as input is available.
 * If no input is present, it may return -1 with errno = EAGAIN,
 * in which case we retry.
 */
char editorReadKey(void) {
  int nread;
  char c;

  while ((nread = read(STDIN_FILENO, &c, 1)) != 1) {
    if (nread == -1 && errno != EAGAIN)
      die("read");
  }

  return c;
}

/*** Input Handling ***/

/*
 * Processes a single keypress.
 *
 * Ctrl-Q:
 *  - Clears the screen
 *  - Repositions the cursor
 *  - Exits the program cleanly
 *
 * Clearing the screen prevents leftover editor artifacts
 * from remaining in the user's terminal.
 */

void editorProcessKeypress(void) {
  char c = editorReadKey();

  switch (c) {
    case CTRL_KEY('q'):
    write(STDOUT_FILENO,"\x1b[2J",4);
    write(STDOUT_FILENO,"\x1b[H",3);

      exit(0);
      break;
  }
}








/*** Output Handling ***/

/*
 * Draws the editor rows to the screen.
 *
 * For now, this function renders a placeholder screen:
 *  - Each row contains a '~' character (like vim)
 *  - Followed by carriage return + newline to move to the next line
 *
 * The '~' indicates lines that are outside the file buffer.
 * This is temporary and will later be replaced by actual file contents.
 */
void editorDrawRows(void){
    int y;

    /*
     * Loop over each visible row of the terminal.
     *
     * Currently hardcoded to 24 rows.
     * This will later be replaced by dynamically
     * detecting the terminal window size.
     */
    for (y = 0; y < 24; y++){
        /*
         * "~"   → visual placeholder for empty lines
         * "\r\n" → carriage return + newline
         *
         * We use write() instead of printf() because:
         *  - It is unbuffered
         *  - It works predictably in raw mode
         */
        write(STDOUT_FILENO, "~\r\n", 3);
    }
}



/*
 * Refreshes the terminal screen.
 *
 * \x1b[2J → ANSI escape code to clear the entire screen
 * \x1b[H  → Move the cursor to the top-left corner (row 1, col 1)
 *
 * This function will later be expanded to:
 *  - Draw file contents
 *  - Render status bars
 *  - Position the cursor correctly
 */


void editorRefreshScreen(void){
    write(STDOUT_FILENO,"\x1b[2J",4);//clear entire screen
    write(STDOUT_FILENO,"\x1b[H",3);//position the cursor to the top
    editorDrawRows();
    write(STDOUT_FILENO,"\x1b[H",3);
    }








/*
 * Program entry point.
 *
 * Enables raw mode and enters the editor loop.
 *
 * Loop structure:
 *  1. Clear and redraw the screen
 *  2. Wait for and process a keypress
 *
 * This mirrors the basic render-input cycle
 * used in most interactive terminal programs.
 */
int main(void) {
  enableRawMode();

  while (1) {
     editorRefreshScreen();

    editorProcessKeypress();
  }

  return 0;
}

