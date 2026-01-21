# kilotexteditor
#include <stdio.h> Standard Input Output Library
#include <ctype.h> Character Functions like iscntrl

#include <stdlib.h> Standard lib for functions like getchar
#include <termios.h>for commands which interact directly with the terminal 
#include <unistd.h>commands which 
struct termios orig_termios;   Here we are creating a struct which has termios and orig_termios which takes the original attributes of the file 





struct termios orig_termios; 
void disableRawMode(void) {   we use this so that the file gets back the original attributes and it doesnt work badly
  tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios); set attr sets teh attributes we are naming the STDIN standarnd input we do tcsa flush so that the input stream is clend and we add we point it into the orig_termios
}
void enableRawMode(void) {  enables rhe raw mode 
  tcgetattr(STDIN_FILENO, &orig_termios);
  atexit(disableRawMode); at exit we will run disbale Raw mode 
  struct termios raw = orig_termios; we put taw 
  raw.c_lflag &= ~(ECHO|ICANON); here we do we change the flags in the file as raw
  tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
}
int main(void) {
  enableRawMode(); we enable raw mmode 
  char c; make a varc char
  while (read(STDIN_FILENO, &c, 1) == 1 && c != 'q') { while loop will run till the time i type q
    if (iscntrl(c)) { we check that if is it a control character
      printf("%d\n", c);we print every cintral character if it is a control chacarer  cause we need to use the value specifier
    } else {
      printf("%d ('%c')\n", c, c);else we print the character 
    }
  }
  return 0;
KILO TEXT EDITOR — FULL LINE-BY-LINE EXPLANATION (SINGLE FLOW)

This program is a learning implementation inspired by the kilo text editor. Its purpose is to understand how a terminal works at a low level using C, POSIX system calls, and raw mode. The program switches the terminal into raw mode, reads input one character at a time, prints information about the pressed key, and exits cleanly when the user presses q.

The code starts by including required header files.

#include <stdio.h>
This header provides standard input/output functions. In this program it is used only for printf(), which prints text to the terminal.

#include <ctype.h>
This header provides character classification functions. We use iscntrl() to check whether a character is a control character such as Enter, Backspace, or Ctrl key combinations.

#include <stdlib.h>
This header provides general utility functions. Here it is used for atexit(), which allows us to register a function that will be executed automatically when the program exits.

#include <termios.h>
This header allows direct interaction with terminal settings. It defines the termios structure and functions like tcgetattr() and tcsetattr() which are used to read and modify how the terminal behaves.

#include <unistd.h>
This header provides low-level POSIX system calls such as read() and constants like STDIN_FILENO.

In Unix-like systems, everything is treated as a file. The keyboard, screen, and errors are represented by file descriptors. STDIN_FILENO is a constant with value 0 and represents standard input, which is the keyboard. STDOUT_FILENO is 1 and represents the screen output. STDERR_FILENO is 2 and represents error output. When we read from STDIN_FILENO, we are literally reading bytes coming from the keyboard.

Next, we define a global variable:

struct termios orig_termios;

struct termios is a structure that stores terminal configuration such as whether input is buffered, whether characters are echoed, and how special keys behave. orig_termios is used to store the original terminal settings so they can be restored when the program exits. This is extremely important because leaving the terminal in raw mode can break the shell.

Now we define the function that restores the terminal:

void disableRawMode(void) { tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios); }

This function resets the terminal back to its original configuration. tcsetattr() sets terminal attributes. The first argument specifies which terminal to modify, here the keyboard input. TCSAFLUSH tells the system to apply the changes after flushing any pending input. &orig_termios is a pointer to the saved original settings. This ensures the terminal behaves normally again after the program exits.

Next is the function that enables raw mode:

void enableRawMode(void) {

The first thing it does is save the current terminal configuration:

tcgetattr(STDIN_FILENO, &orig_termios);

This reads the terminal’s current settings and stores them in orig_termios.

Then we register a cleanup function:

atexit(disableRawMode);

This guarantees that when the program exits—normally or unexpectedly—disableRawMode() will be called, restoring the terminal.

Next we create a copy of the terminal settings:

struct termios raw = orig_termios;

We do this so we can modify the settings safely without losing the original configuration.

Now comes the most important line:

raw.c_lflag &= ~(ECHO | ICANON);

c_lflag stands for “local flags”. ECHO controls whether typed characters automatically appear on the screen. ICANON controls canonical mode, which normally buffers input until Enter is pressed. The ~ operator inverts bits, and &= applies the change. This line disables both echoing and line buffering.

As a result, each keypress is available immediately to the program and nothing is printed unless the program explicitly prints it. This is what raw mode means.

Finally, we apply the new settings:

tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);

The terminal is now in raw mode.

Now execution enters the main function:

int main(void) {

The first thing we do is enable raw mode:

enableRawMode();

Next, we declare a character variable:

char c;

This variable will store one byte read from the keyboard.

Now we enter the input loop:

while (read(STDIN_FILENO, &c, 1) == 1 && c != 'q') {

read() is a system call that reads raw bytes from a file descriptor. Here it reads exactly one byte from the keyboard and stores it in c. The loop continues as long as exactly one byte is read and the character is not q. Pressing q exits the loop.

Inside the loop, we check whether the character is a control character:

if (iscntrl(c)) {

Control characters do not have a printable representation, so we print only their ASCII value:

printf("%d\n", c);

If the character is printable, we print both its ASCII value and the character itself:

printf("%d ('%c')\n", c, c);

This lets us see exactly what bytes the keyboard sends to the program.

Once the loop ends, the program returns:

return 0;

When the program exits, atexit() automatically calls disableRawMode(), restoring the terminal to its original state.

This program demonstrates how text editors, shells, and terminal applications work internally. It shows that the keyboard is just a stream of bytes, the terminal is configurable, and raw mode is the foundation for interactive software like Vim, Nano, and games.}
