#include <stdlib.h>
#include <stdio.h>
#include <termios.h>
#include <string.h>
#include <unistd.h>

// Store original attributes in a global static variable
static struct termios original_tty_attr;
static int tty_saved = 0;

/*
 * Sets terminal into raw mode.
 * This causes having the characters available
 * immediately instead of waiting for a newline.
 * Also there is no automatic echo.
 */
void tty_raw_mode(void)
{
    struct termios tty_attr;

    if (!tty_saved) {
        // Save original mode once
        tcgetattr(0, &original_tty_attr);
        tty_saved = 1;
    }

    tty_attr = original_tty_attr;

    /* Set raw mode. */
    tty_attr.c_lflag &= (~(ICANON | ECHO));
    tty_attr.c_lflag |= ISIG; // Allow Ctrl-C
    tty_attr.c_cc[VTIME] = 0;
    tty_attr.c_cc[VMIN] = 1;

    tcsetattr(0, TCSANOW, &tty_attr);
}

/*
 * Restores terminal to the original mode
 */
void tty_restore_mode(void)
{
    if (tty_saved) {
        tcsetattr(0, TCSANOW, &original_tty_attr);
    }
}

