#include <iostream>
#include <unistd.h>
#include <termios.h>
#include <cstdlib>

// This struct will hold the original terminal settings
struct termios orig_termios;

// Restores the terminal to its original settings
void disableRawMode() {
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios);
    std::cout << "Terminal restored." << std::endl;
}

// Puts the terminal into raw mode (no echo, no line buffering)
void enableRawMode() {
    // Get the original settings
    if (tcgetattr(STDIN_FILENO, &orig_termios) == -1) {
        perror("tcgetattr failed");
        exit(1);
    }
    // Ensure disableRawMode is called on exit
    atexit(disableRawMode);

    struct termios raw = orig_termios;
    // Disable echo, canonical mode (line buffering), signal chars (Ctrl+C),
    // and extended input processing.
    raw.c_lflag &= ~(ECHO | ICANON | ISIG | IEXTEN);
    
    // Set to block until at least 1 character is read
    raw.c_cc[VMIN] = 1;
    raw.c_cc[VTIME] = 0;

    // Apply the new settings
    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) == -1) {
        perror("tcsetattr failed");
        exit(1);
    }
}

int main() {
    enableRawMode();

    std::cout << "Raw mode enabled. Press keys to see their byte values. Press 'q' to quit." << std::endl;
    
    char c;
    // Loop, reading one byte at a time
    while (true) {
        // This read will block until you press a key
        if (read(STDIN_FILENO, &c, 1) == 1) {
            // Check if the character is printable or a control character
            if (iscntrl(c)) {
                // Print the integer value of control characters (like Enter, Esc, etc.)
                std::cout << "Read value: " << static_cast<int>(c) << std::endl;
            } else {
                // Print the character and its integer value
                std::cout << "Read char: '" << c << "' | value: " << static_cast<int>(c) << std::endl;
            }

            // Quit condition
            if (c == 'q') {
                break;
            }
        } else {
            // read() should not fail here, but if it does, break the loop
            std::cout << "Read failed." << std::endl;
            break;
        }
    }

    return 0;
}
