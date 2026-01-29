



This project is based on the "Build Your Own Text Editor" (kilo) tutorial
by Salvatore Sanfilippo.

The code was written for learning purposes.
Inline comments and explanations were assisted by AI tools.

## What This Program Does

This program transforms your terminal into raw mode, where each keypress is immediately available to the application without waiting for Enter, and nothing is echoed to the screen unless explicitly printed. It displays the ASCII value and character representation of each key you press, providing a window into how terminals actually process keyboard input.

**Key behaviors:**
- Reads input character by character (not line by line)
- Shows ASCII codes for every keypress
- Distinguishes between printable and control characters
- Exits cleanly when you press `q`
- Automatically restores terminal settings on exit

## Why This Matters

In normal terminal operation, your shell operates in "cooked mode" where:
- Input is buffered until you press Enter
- Characters are automatically echoed to the screen
- Special keys like Ctrl+C send signals to programs

Text editors need finer control. They must:
- Respond to every keypress immediately
- Control exactly what appears on screen
- Handle special key combinations themselves

This program demonstrates the foundation of interactive terminal applications.

## Building and Running

```bash
# Compile the program
gcc -o kilo kilo.c -Wall -Wextra -std=c99

# Run it
./kilo
```

Once running, type any keys and observe their ASCII values. Press `q` to exit.

## Code Walkthrough

### Terminal Configuration (`struct termios`)

The `termios` structure controls terminal behavior:
- **Input flags**: How input is processed
- **Output flags**: How output is formatted
- **Control flags**: Hardware control settings
- **Local flags**: Terminal modes (echo, canonical, etc.)

### Key Functions

**`enableRawMode()`**
- Saves original terminal settings
- Disables `ECHO` (automatic character display)
- Disables `ICANON` (line buffering)
- Registers cleanup function with `atexit()`

**`disableRawMode()`**
- Restores original terminal settings
- Called automatically on program exit
- Prevents leaving terminal in broken state

**`main()`**
- Enters raw mode
- Reads keyboard input byte by byte using `read()`
- Prints ASCII values and characters
- Exits on `q`

### The Magic Line

```c
raw.c_lflag &= ~(ECHO | ICANON);
```

This single line transforms the terminal:
- `~(ECHO | ICANON)` creates a bitmask with these flags cleared
- `&=` applies the mask, turning off echo and canonical mode
- Result: raw mode where you control everything

## What You'll See

```
$ ./kilo
97 ('a')
98 ('b')
99 ('c')
13
65 ('A')
27
113 ('q')
$
```

**Observations:**
- Regular letters show both ASCII and character
- Control characters (like Enter = 13) show only ASCII
- Escape key produces ASCII 27
- Capital letters have different ASCII values than lowercase

## File Descriptors Explained

Unix treats everything as a file:
- `STDIN_FILENO` (0): Keyboard input
- `STDOUT_FILENO` (1): Screen output
- `STDERR_FILENO` (2): Error output

The `read()` system call reads raw bytes from stdin, treating the keyboard as a byte stream.

## Safety Features

1. **Original settings saved**: Terminal state preserved before modification
2. **Automatic cleanup**: `atexit()` ensures restoration even on crashes
3. **`TCSAFLUSH` flag**: Discards unread input when changing modes

Without these safeguards, exiting unexpectedly would leave your terminal unusable.

## Learning Objectives

This implementation teaches:
- ✅ How terminals actually work (not just abstract I/O)
- ✅ The difference between cooked and raw mode
- ✅ POSIX system calls (`read`, `tcgetattr`, `tcsetattr`)
- ✅ Bit manipulation for configuration flags
- ✅ Resource cleanup and program safety
- ✅ Foundation for building text editors

## Next Steps

To build a full editor, you would add:
- **Screen control**: ANSI escape sequences to move cursor and clear screen
- **Input handling**: Arrow keys, Ctrl combinations, function keys
- **Buffer management**: In-memory representation of file contents
- **File I/O**: Reading and writing files
- **Syntax highlighting**: Tokenizing and coloring code
- **Search and replace**: Pattern matching through text

## References

This implementation is inspired by the [Build Your Own Text Editor](https://viewsourcecode.org/snaptoken/kilo/) tutorial, which provides a complete step-by-step guide to building a functional text editor from scratch.

## License

This is educational code. Use it to learn, modify it freely, and build something amazing.

---

**Remember:** Every text editor, shell, and interactive terminal program you've ever used is built on these same fundamental concepts. You're looking at the foundation of command-line software.h>
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
