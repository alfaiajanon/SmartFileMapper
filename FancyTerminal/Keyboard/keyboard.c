#ifdef _WIN32
#include <windows.h>
#include <conio.h>
#else
#include <termios.h>
#include <unistd.h>
#include <sys/select.h>
#include <fcntl.h>
#endif

#include "keyboard.h"

// Key constants - using your enum values
enum Essentials{
    NONE = -1,
    ARROW_LEFT = 99900,
    ARROW_RIGHT = 99901,
    ARROW_UP = 99902,
    ARROW_DOWN = 99903,
    BACKSPACE = 99904,
    SPACE = 99905,
    ESC = 99906,
    ENTER = 99907,
};

#ifdef _WIN32
// Windows implementation
static HANDLE hStdin = NULL;
static DWORD fdwSaveOldMode;

void init_keyboard() {
    hStdin = GetStdHandle(STD_INPUT_HANDLE);
    GetConsoleMode(hStdin, &fdwSaveOldMode);
    SetConsoleMode(hStdin, 0); // Disable all input processing
}

void restore_keyboard() {
    SetConsoleMode(hStdin, fdwSaveOldMode);
}

int detect_key() {
    if (!_kbhit()) {
        return NONE; // No key pressed
    }
    
    int key = _getch();
    
    if (key == 0 || key == 224) { // Extended key prefix
        key = _getch(); // Get the actual key code
        switch (key) {
            case 72: return ARROW_UP;
            case 80: return ARROW_DOWN;
            case 75: return ARROW_LEFT;
            case 77: return ARROW_RIGHT;
            default: return key + 1000; // Other extended keys
        }
    }
    
    // Map special keys to enum values
    switch (key) {
        case 8:   return BACKSPACE;  // Backspace
        case 127: return BACKSPACE;  // DEL (sometimes used as backspace)
        case 13:  return ENTER;      // Enter/Return
        case 27:  return ESC;        // Escape
        case 32:  return SPACE;      // Space
        default:  return key;        // Return normal ASCII value
    }
}

#else
// Linux/Unix implementation
static struct termios old_termios;
static int old_flags;

void __init_keyboard() {
    // Save current terminal settings
    tcgetattr(STDIN_FILENO, &old_termios);
    
    // Set raw mode
    struct termios new_termios = old_termios;
    new_termios.c_lflag &= ~(ICANON | ECHO | ISIG);
    new_termios.c_iflag &= ~(IXON | ICRNL);
    new_termios.c_cc[VMIN] = 0;  // Non-blocking read
    new_termios.c_cc[VTIME] = 0;
    tcsetattr(STDIN_FILENO, TCSANOW, &new_termios);
    
    // Make stdin non-blocking
    old_flags = fcntl(STDIN_FILENO, F_GETFL);
    fcntl(STDIN_FILENO, F_SETFL, old_flags | O_NONBLOCK);
}

void __restore_keyboard() {
    // Restore terminal settings
    tcsetattr(STDIN_FILENO, TCSANOW, &old_termios);
    fcntl(STDIN_FILENO, F_SETFL, old_flags);
}

int __detect_key() {
    char c;
    int bytes_read = read(STDIN_FILENO, &c, 1);
    
    if (bytes_read <= 0) {
        return NONE; // No key pressed
    }
    
    if (c == 27) { // ESC sequence
        // Try to read the next character immediately
        char seq[2];
        int n1 = read(STDIN_FILENO, &seq[0], 1);
        if (n1 <= 0) {
            return ESC; // Just ESC key
        }
        
        if (seq[0] == '[') {
            int n2 = read(STDIN_FILENO, &seq[1], 1);
            if (n2 <= 0) {
                return ESC; // Incomplete sequence
            }
            
            switch (seq[1]) {
                case 'A': return ARROW_UP;
                case 'B': return ARROW_DOWN;
                case 'C': return ARROW_RIGHT;
                case 'D': return ARROW_LEFT;
                default: return ESC; // Unknown sequence
            }
        }
        return ESC; // ESC + something else
    }
    
    // Map special keys to enum values
    switch (c) {
        case 8:   return BACKSPACE;  // Backspace
        case 127: return BACKSPACE;  // DEL (sometimes used as backspace)
        case 13:  return ENTER;      // Enter/Return
        case 10:  return ENTER;      // Line Feed (sometimes used as enter)
        case 32:  return SPACE;      // Space
        default:  return (int)c;     // Return normal ASCII value
    }
}
#endif





