#include "misc/base.hpp"
#ifndef PANELUTL
#define PANELUTL

#include <iostream>
#include <string>
#include <vector>
#include <functional>


#ifdef _WIN32
#include <windows.h>
#include <conio.h>
#else
#include <unistd.h>
#include <termios.h>
#include <sys/ioctl.h>
#include <cstdlib> 
#endif

#include "panel/paneldef.hpp"


struct TermSize {
    int w, h;
};
static TermSize term;



#ifdef _WIN32

void get_terminal_size() {
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi);
    term.w = csbi.srWindow.Right - csbi.srWindow.Left + 1;
    term.h = csbi.srWindow.Bottom - csbi.srWindow.Top + 1;
}

void clear() {
    COORD topLeft = {0, 0};
    HANDLE console = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_SCREEN_BUFFER_INFO screen;
    DWORD written;
    GetConsoleScreenBufferInfo(console, &screen);
    FillConsoleOutputCharacterA(console, ' ', screen.dwSize.X * screen.dwSize.Y, topLeft, &written);
    FillConsoleOutputAttribute(console, FOREGROUND_GREEN | FOREGROUND_RED | FOREGROUND_BLUE, screen.dwSize.X * screen.dwSize.Y, topLeft, &written);
    SetConsoleCursorPosition(console, topLeft);
}

void move(int x, int y) {
    COORD coord;
    coord.X = x;
    coord.Y = y;
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), coord);
}

#else

struct termios orig_termios;

void disableRawMode() {
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios);
    const char* show_cursor_code = "\033[?25h";
    write(STDOUT_FILENO, show_cursor_code, 6);
}

void enableRawMode() {
    tcgetattr(STDIN_FILENO, &orig_termios);
    atexit(disableRawMode);
    struct termios raw = orig_termios;
    raw.c_lflag &= ~(ECHO | ICANON | ISIG | IEXTEN);
    raw.c_iflag &= ~(IXON | ICRNL);
    raw.c_cc[VMIN] = 0;
    raw.c_cc[VTIME] = 1;
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
    const char* hide_cursor_code = "\033[?25l";
    write(STDOUT_FILENO, hide_cursor_code, 6);
}

void get_terminal_size() {
    struct winsize w;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
    term.w = w.ws_col;
    term.h = w.ws_row;
}

void clear() {
    const char* clear_code = "\033[2J\033[H";
    write(STDOUT_FILENO, clear_code, 7);
}

void move(int x, int y) {
    std::string move_code = "\033[" + std::to_string(y + 1) + ";" + std::to_string(x + 1) + "H";
    write(STDOUT_FILENO, move_code.c_str(), move_code.length());
}
#endif



enum Key { UP, DOWN, RIGHT, LEFT, TAB, QUIT, OTHER, NONE , FILL, SUSPEND};

Key get_key() {
#ifdef _WIN32
    if (_kbhit()) {
        int c = _getch();
        switch (c) {
            case 9: return TAB;
            case 'q': return QUIT;
            case 224:
                switch (_getch()) {
                    case 72: return UP;
                    case 80: return DOWN;
                }
            default: return OTHER;
        }
    }
    return NONE;
#else
    char buf[3];
    int nread = read(STDIN_FILENO, buf, sizeof(buf));
    if (nread <= 0) return NONE;
    if (nread == 1) {
        if (buf[0] == 'q') return QUIT;
        if (buf[0] == 'f') return FILL;
        if (buf[0] == 's') return SUSPEND;
        if (buf[0] == '\t') return TAB;
    }
    if (nread == 3 && buf[0] == '\033' && buf[1] == '[') {
        if (buf[2] == 'A') return UP;
        if (buf[2] == 'B') return DOWN;
        if (buf[2] == 'C') return RIGHT;
        if (buf[2] == 'D') return LEFT;

    }
    return OTHER;
#endif
}


void draw_line(int y, std::string text, bool highlight = false,std::string rm_buf = " ",std::string start_buffer="\033[7m",std::string end_buffer="\033[0m") {
    if (text == "<thinline>"){
        return draw_line(y,"",false,"─");
    }
    if (text.find("<fill>") != std::string::npos){
        //logger->log("Found <fill>","draw_line");
        text = replace(text,"<fill>","");
        return draw_line(y,text,false,"─");

    }
    move(0, y);
    std::string line_buffer;
    if (highlight) line_buffer += start_buffer;

    line_buffer += text;

    if ((int)text.length() < term.w) {
        line_buffer += repeat(rm_buf, term.w - text.length());
    }
    if (highlight) line_buffer += end_buffer;


    write(STDOUT_FILENO, line_buffer.c_str(), line_buffer.length());
}

struct Panel {
    std::string title;
    std::shared_ptr<ClientPanel> content;
    bool fill;
    int selected = 0;
};

class PanelManager {
    std::vector<Panel> panels;
public:
    int active_panel = 0;
    int startidx = 0;
    bool suspend=false;
    void add_panel(const Panel& p) { panels.push_back(p); }

    void draw() {
        get_terminal_size();
        clear();
        int row = 0;
        int remaining = term.h;
        for (auto& p : panels) {
            if (!p.fill) {
                remaining -= (int)p.content->draw().size() + 1;
            } else {
                 remaining -= 1;
            }
        }
        
        auto& p = panels[active_panel];
        auto lines = p.content->draw();
        std::string hdr = " " + p.title + " ";
        draw_line(row++, hdr, false,"━");
        int viewport_height = (p.fill) ? remaining : (int)lines.size();
        for (int j = startidx; j < startidx + viewport_height && j < (int)lines.size(); j++) {
            bool highlight = (j == p.selected);
            draw_line(row++, lines[j], highlight);
        }
        if ((p.fill) && (int)lines.size() < viewport_height-1) {
            for (int j = lines.size(); j < viewport_height-1; j++) draw_line(row++, "");
        }
        draw_line(row++, "", false,"━");
        
    }

    void move_selection(int dir) {
        if (panels.empty()) return;
        auto& p = panels[active_panel];
        auto lines = p.content->draw();
        if (lines.empty()) return;
        p.selected = (p.selected + dir + (int)lines.size()) % (int)lines.size();
        get_terminal_size();
        int viewport_height = term.h - 1;
        if (p.selected < startidx) {
            startidx = p.selected;
        } else if (p.selected >= startidx + viewport_height) {
            startidx = p.selected - viewport_height + 1;
        }


        if (startidx < 0) startidx = 0;
        if (startidx > (int)lines.size() - viewport_height)
            startidx = std::max(0, (int)lines.size() - viewport_height);
    }
    
    void run() {
        #ifndef _WIN32
            enableRawMode();
        #endif

        bool should_quit = false;
        while (!should_quit) {
            draw();
            Key key = get_key();
            switch (key) {
                case UP:
                    move_selection(-1);
                    break;
                case DOWN:
                    move_selection(+1); 
                    break;
                case RIGHT:
                    if (active_panel+1 == panels.size()){
                        active_panel = 0;
                    } else {
                        active_panel += 1;
                    }
                    break;
                case LEFT:
                    if (active_panel-1 == -1){
                        active_panel = panels.size()-1;
                    } else {
                        active_panel -= 1;
                    } 
                    break;
                case TAB:
                    if (!panels.empty()) active_panel = (active_panel + 1) % panels.size();
                    startidx = 0;
                    break;
                case QUIT:
                    
                    should_quit = true;
                    break;
                case FILL:
                    panels[active_panel].fill = !panels[active_panel].fill;
                    break;
                case SUSPEND:
                    suspend = !suspend;
                    if (suspend){
                        Key k = LEFT;
                        while ( k != SUSPEND){
                            k = get_key();
                            
                        }
                    }
                    break;
                case OTHER: break;
                case NONE: break;
            }
        }
        for (auto p :panels){
            std::this_thread::sleep_for(std::chrono::seconds(1));
            p.content->deinit();
        }
        clear();
    }
};

#endif