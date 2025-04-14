#include <stdio.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <termios.h>

#define ASSERT(expr) if (!(expr)) { ret = 1; goto exit; }

int get_pos(int *x, int *y) {
    int ret = 0;
    char buf[128];
    int len = 0;
    ASSERT(write(STDOUT_FILENO, "\033[6n", 4) > 0);
    do {
        ASSERT(read(STDIN_FILENO, buf + len, 1) > 0);
    }
    while (buf[len++] != 'R' && len < 127);
    buf[len] = 0;
    ASSERT(sscanf(buf, "\x1b[%d;%dR", y, x) == 2);
exit:
    return ret;
}

int get_winsize(int *w, int *h) {
    int ret = 0;
    struct winsize ws;
    ASSERT(ioctl(1, TIOCGWINSZ, &ws) >= 0);
    *w = ws.ws_col;
    *h = ws.ws_row;
exit:
    return ret;
}

int move(int y, int x) {
    return printf("\x1b[%u;%uH", y, x);
}

int clear_til_eos() {
    return printf("\x1b[0J");
}

int setup_term(struct termios *initial_termios) {
    int ret = 0;
    struct termios settings;
    ASSERT(tcgetattr(1, initial_termios) >= 0);
    settings = *initial_termios;
    settings.c_lflag &= ~(ECHO | ICANON);
    ASSERT(tcsetattr(1, TCSANOW, &settings) >= 0);
exit:
    return ret;
}

int restore_term(struct termios *initial_termios) {
    int ret = 0;
    ASSERT(tcsetattr(1, TCSANOW, initial_termios) >= 0);
exit:
    return ret;
}

int main(int argc, char **argv) {
    int ret = 0;

    struct termios initial_termios;
    if (setup_term(&initial_termios) != 0) {
        return 1;
    }

    int w, h;
    ASSERT(get_winsize(&w, &h) == 0);

    int scrolloff = 10;
    ASSERT(h >= scrolloff);
    int x, y;
    ASSERT(get_pos(&x, &y) == 0);
    ASSERT(y >= scrolloff);
    int scroll = h - scrolloff;
    for (int i = 0; i < scroll; i++) {
        ASSERT(printf("\n") > 0);
    }
    ASSERT(printf("\x1b[%dA", scroll + 1) > 0);
    ASSERT(clear_til_eos() > 0);

exit:
    return restore_term(&initial_termios) || ret;
}
