#include <iostream>
#include <memory>
#include <cstring>
#include <cstdio>
#include <string>
#include <cctype>
#include <algorithm>
#include <utility>
#include <signal.h>
#include <epaper/epaper_display.h>
#include <app/config.h>

#define DISP_W 296
#define DISP_H 152

static volatile bool running = true;
static void onSignal(int) { running = false; }

struct Key {
    const char* label;
    int w;
};
static constexpr int GAP = 2;

struct KbdRow {
    const Key* keys;
    int count;
};

static Key row1[] = {{"Esc",22},{"F1",20},{"F2",20},{"F3",20},{"F4",20},{"F5",20},{"F6",20},{"F7",20},{"F8",20},{"F9",20},{"F10",20},{"F11",20},{"F12",20}};
static Key row2[] = {{"`~",18},{"1!",18},{"2@",18},{"3#",18},{"4$",18},{"5%",18},{"6^",18},{"7&",18},{"8*",18},{"9(",18},{"0)",18},{"-_",18},{"=+",18},{"BkSp",22}};
static Key row3[] = {{"Tab",20},{"Q",19},{"W",19},{"E",19},{"R",19},{"T",19},{"Y",19},{"U",19},{"I",19},{"O",19},{"P",19},{"[{",19},{"]}",19},{"\\|",18}};
static Key row4[] = {{"Caps",24},{"A",18},{"S",18},{"D",18},{"F",18},{"G",18},{"H",18},{"J",18},{"K",18},{"L",18},{"N~",18},{";:",18},{"'\"",18},{"Enter",26}};
static Key row5[] = {{"Shift",32},{"Z",19},{"X",19},{"C",19},{"V",19},{"B",19},{"N",19},{"M",19},{",<",19},{".>",19},{"/?",19},{"Shift",32}};
static Key row6[] = {{"Ctrl",26},{"Alt",22},{"______Space______",116},{"Alt",22},{"Ctrl",26},{"Del",26}};

static KbdRow kbdRows[] = {{row1,13},{row2,14},{row3,14},{row4,14},{row5,12},{row6,6}};
static constexpr int NUM_ROWS = 6;
static constexpr int ROW_H = 16;
static constexpr int KBD_Y0 = 30;

struct KeyPos { int row, col; };

static int keyX[NUM_ROWS][16];
static int keyY[NUM_ROWS];

static void initKeyPositions() {
    for (int r = 0; r < NUM_ROWS; r++) {
        int totalW = 0;
        for (int c = 0; c < kbdRows[r].count; c++)
            totalW += kbdRows[r].keys[c].w;
        totalW += (kbdRows[r].count - 1) * GAP;
        int x = (DISP_W - totalW) / 2;
        if (x < 0) x = 0;
        keyY[r] = KBD_Y0 + r * (ROW_H + GAP);
        for (int c = 0; c < kbdRows[r].count; c++) {
            keyX[r][c] = x;
            x += kbdRows[r].keys[c].w + GAP;
        }
    }
}

static KeyPos findKey(const std::string& id) {
    std::string lower = id;
    std::transform(lower.begin(), lower.end(), lower.begin(), ::tolower);
    static const std::pair<const char*,const char*> alias[] = {{"backspace","bksp"},{"delete","del"}};
    for (auto& [from,to] : alias)
        if (lower == from) { lower = to; break; }
    for (int phase = 0; phase < 4; phase++) {
        for (int r = 0; r < NUM_ROWS; r++) {
            for (int c = 0; c < kbdRows[r].count; c++) {
                std::string lbl = kbdRows[r].keys[c].label;
                std::transform(lbl.begin(), lbl.end(), lbl.begin(), ::tolower);
                bool match = false;
                if (phase == 0) match = (lbl == lower);
                else if (phase == 1) match = (lbl.find(lower) == 0);
                else if (phase == 2) match = (lbl.find(lower) != std::string::npos);
                else if (phase == 3) match = (lower.find(lbl) == 0);
                if (match) return {r, c};
            }
        }
    }
    return {-1, -1};
}

// =========== DRAWING HELPERS ===========

static void drawKey(EPAPER_DISPLAY::EpaperDisplay* d, int r, int c, bool hl) {
    int x = keyX[r][c];
    int y = keyY[r];
    int w = kbdRows[r].keys[c].w;
    int h = ROW_H;
    const char* label = kbdRows[r].keys[c].label;

    d->drawRectangle(x, y, w, h, true, hl);
    d->drawRectangle(x, y, w, h, false, !hl);
    int lblW = d->getTextWidth(label, FONT_3x8_TINY);
    int lblX = x + (w - lblW) / 2;
    int lblY = y + (h - 8) / 2;
    d->drawString(lblX, lblY, label, FONT_3x8_TINY, !hl);
}

static void drawAllKeys(EPAPER_DISPLAY::EpaperDisplay* d, int hlRow, int hlCol) {
    for (int r = 0; r < NUM_ROWS; r++)
        for (int c = 0; c < kbdRows[r].count; c++)
            drawKey(d, r, c, (r == hlRow && c == hlCol));
}

static void drawDialer(EPAPER_DISPLAY::EpaperDisplay* d, const std::string& num, int hlDigit) {
    char buf[32];
    snprintf(buf, sizeof(buf), "Numero: %s", num.c_str());
    d->drawString(6, 4, buf, FONT_5x8, true);
    d->drawLine(0, 16, 295, 16, true);

    static const char* DK = "123456789*0#";
    int kw = 80, kh = 28, gap = 4;
    int sx = (DISP_W - 3 * kw - 2 * gap) / 2;
    int sy = 22;

    for (int i = 0; i < 12; i++) {
        int r = i / 3, c = i % 3;
        int x = sx + c * (kw + gap);
        int y = sy + r * (kh + gap);
        bool hl = (i == hlDigit);
        char lb[2] = {DK[i], 0};
        d->drawRectangle(x, y, kw, kh, true, hl);
        d->drawRectangle(x, y, kw, kh, false, !hl);
        int tw = d->getTextWidth(lb, FONT_8x8);
        d->drawString(x + (kw - tw) / 2, y + (kh - 8) / 2, lb, FONT_8x8, !hl);
    }

    int by = sy + 3 * (kh + gap);
    d->drawRectangle(6, by, 130, 26, true, false);
    d->drawRectangle(6, by, 130, 26, false, true);
    int cw = d->getTextWidth("Llamar", FONT_5x8);
    d->drawString(6 + (130 - cw) / 2, by + 8, "Llamar", FONT_5x8, true);
    d->drawRectangle(160, by, 130, 26, true, false);
    d->drawRectangle(160, by, 130, 26, false, true);
    int ew = d->getTextWidth("Colgar", FONT_5x8);
    d->drawString(160 + (130 - ew) / 2, by + 8, "Colgar", FONT_5x8, true);
}

static void drawAppGrid(EPAPER_DISPLAY::EpaperDisplay* d, int selected) {
    static const char* apps[] = {
        "Contactos","Mensajes","Telefono",
        "Camara","Ajustes","Musica",
        "Reloj","Browser","Juegos"
    };
    int cw = 86, ch = 34;
    int sx = (DISP_W - 3 * cw) / 2;
    int sy = 18;
    for (int i = 0; i < 9; i++) {
        int r = i / 3, c = i % 3;
        int x = sx + c * cw, y = sy + r * ch;
        bool hl = (i == selected);
        d->drawRectangle(x, y, cw - 2, ch - 2, true, hl);
        d->drawRectangle(x, y, cw - 2, ch - 2, false, !hl);
        int nw = d->getTextWidth(apps[i], FONT_3x8_TINY);
        d->drawString(x + (cw - 2 - nw) / 2, y + (ch - 8) / 2, apps[i], FONT_3x8_TINY, !hl);
    }
}

// =========== MENU SYSTEM ===========

enum Screen { SCREEN_MAIN_MENU, SCREEN_KEYBOARD, SCREEN_DIALER, SCREEN_APPS };
static Screen currentScreen = SCREEN_MAIN_MENU;

static const char* menuItems[] = {
    "TECLADO",
    "DISCADOR",
    "APPS",
    "ACERCA DE",
    "SALIR"
};
static constexpr int MENU_COUNT = 5;

static void drawTopBar(EPAPER_DISPLAY::EpaperDisplay* d, const char* title) {
    d->drawString(2, 0, title, FONT_3x8_TINY, true);
    d->drawLine(0, 10, 295, 10, true);
}

static void drawBottomBar(EPAPER_DISPLAY::EpaperDisplay* d, const char* hints) {
    d->drawString(2, 143, hints, FONT_3x8_TINY, true);
}

static void drawMainMenu(EPAPER_DISPLAY::EpaperDisplay* d, int cursor) {
    int startY = 24;
    int itemH = 20;
    for (int i = 0; i < MENU_COUNT; i++) {
        int y = startY + i * itemH;
        bool hl = (i == cursor);
        int w = d->getTextWidth(menuItems[i], FONT_7x8_THICK);
        int x = (DISP_W - w) / 2 - 6;

        if (hl) {
            d->drawRectangle(x - 4, y - 1, w + 12, itemH, true, true);
            d->drawString(x, y + 2, menuItems[i], FONT_7x8_THICK, false);
            d->drawString(x - w + 2, y + 2, ">", FONT_7x8_THICK, false);
        } else {
            d->drawRectangle(x - 4, y - 1, w + 12, itemH, true, false);
            d->drawString(x, y + 2, menuItems[i], FONT_7x8_THICK, true);
        }
    }
}

static const char* screenTitle() {
    switch (currentScreen) {
        case SCREEN_MAIN_MENU: return "SmartPhone Demo  [Menu]";
        case SCREEN_KEYBOARD:  return "SmartPhone Demo  [Teclado]";
        case SCREEN_DIALER:    return "SmartPhone Demo  [Discador]";
        case SCREEN_APPS:      return "SmartPhone Demo  [Apps]";
    }
    return "";
}

static const char* screenHints() {
    switch (currentScreen) {
        case SCREEN_MAIN_MENU: return "w/s navega  Enter selecciona  q salir";
        case SCREEN_KEYBOARD:  return "tecla=letra  q=volver  quit=salir";
        case SCREEN_DIALER:    return "digito=numero  q=volver  quit=salir";
        case SCREEN_APPS:      return "1-9 app  q=volver  quit=salir";
    }
    return "";
}

// =========== REDRAW ===========

static void redraw(EPAPER_DISPLAY::EpaperDisplay* d,
                   int hlRow, int hlCol,
                   int menuCursor,
                   const std::string& dialNum, int hlDigit,
                   int appSel,
                   const std::string& textInput)
{
    d->clearScreen(true);
    drawTopBar(d, screenTitle());

    switch (currentScreen) {
        case SCREEN_MAIN_MENU:
            drawMainMenu(d, menuCursor);
            break;
        case SCREEN_KEYBOARD:
            drawAllKeys(d, hlRow, hlCol);
            if (!textInput.empty()) {
                int maxW = 280;
                std::string vis;
                int tw = d->getTextWidth(textInput, FONT_5x8);
                if (tw > maxW) {
                    size_t ta = textInput.size() * maxW / tw;
                    if (ta > 5) ta -= 5;
                    vis = ".." + textInput.substr(ta);
                } else {
                    vis = textInput;
                }
                d->drawString(4, 14, vis, FONT_5x8, true);
            }
            break;
        case SCREEN_DIALER:
            drawDialer(d, dialNum, hlDigit);
            break;
        case SCREEN_APPS:
            drawAppGrid(d, appSel);
            break;
    }

    drawBottomBar(d, screenHints());
    d->update();
}

int main() {
    signal(SIGINT, onSignal);
    signal(SIGTERM, onSignal);

    std::cout << "=== SMARTPHONE DEMO ===" << std::endl;
    std::cout << "Menu principal. Navega con w/s, Enter selecciona." << std::endl;

    if (!bcm2835_init()) {
        std::cerr << "ERROR: bcm2835_init() failed" << std::endl;
        return 1;
    }

    auto display = std::make_unique<EPAPER_DISPLAY::EpaperDisplay>(
        eScreen_EPD_266, EPAPER::boardRaspberryPi
    );
    if (!display->init()) {
        std::cerr << "ERROR: display init failed" << std::endl;
        return 1;
    }

    initKeyPositions();

    int menuCursor = 0;
    int hlRow = -1, hlCol = -1;
    std::string dialNumber;
    int hlDigit = -1;
    int appSel = -1;
    std::string textInput;

    redraw(display.get(), hlRow, hlCol, menuCursor, dialNumber, hlDigit, appSel, textInput);

    char line[128];
    while (running) {
        std::cout << "> ";
        std::cout.flush();
        if (!fgets(line, sizeof(line), stdin)) break;

        size_t len = strlen(line);
        while (len > 0 && (line[len-1] == '\n' || line[len-1] == '\r'))
            line[--len] = '\0';
        if (len == 0) {
            if (currentScreen == SCREEN_MAIN_MENU) {
                std::cout << "Seleccionaste: " << menuItems[menuCursor] << std::endl;
                switch (menuCursor) {
                    case 0: currentScreen = SCREEN_KEYBOARD; break;
                    case 1: currentScreen = SCREEN_DIALER; dialNumber.clear(); hlDigit = -1; break;
                    case 2: currentScreen = SCREEN_APPS; appSel = -1; break;
                    case 3:
                        std::cout << "SmartPhone Demo v1.0 - E-Paper 2.66" << std::endl;
                        break;
                    case 4: running = false; continue;
                }
            }
            redraw(display.get(), hlRow, hlCol, menuCursor, dialNumber, hlDigit, appSel, textInput);
            continue;
        }

        std::string cmd = line;
        std::string lower = cmd;
        std::transform(lower.begin(), lower.end(), lower.begin(), ::tolower);

        if (lower == "quit" || lower == "exit") {
            break;
        }

        if (lower == "q" || lower == "back") {
            if (currentScreen != SCREEN_MAIN_MENU) {
                currentScreen = SCREEN_MAIN_MENU;
                redraw(display.get(), hlRow, hlCol, menuCursor, dialNumber, hlDigit, appSel, textInput);
                continue;
            }
        }

        if (currentScreen == SCREEN_MAIN_MENU) {
            if (lower == "w" || lower == "up") {
                menuCursor = (menuCursor - 1 + MENU_COUNT) % MENU_COUNT;
            } else if (lower == "s" || lower == "down") {
                menuCursor = (menuCursor + 1) % MENU_COUNT;
            } else {
                int n = atoi(cmd.c_str());
                if (n >= 1 && n <= MENU_COUNT) {
                    menuCursor = n - 1;
                    std::cout << "Seleccionaste: " << menuItems[menuCursor] << std::endl;
                    switch (menuCursor) {
                        case 0: currentScreen = SCREEN_KEYBOARD; break;
                        case 1: currentScreen = SCREEN_DIALER; dialNumber.clear(); hlDigit = -1; break;
                        case 2: currentScreen = SCREEN_APPS; appSel = -1; break;
                        case 3: std::cout << "SmartPhone Demo v1.0 - E-Paper 2.66" << std::endl; break;
                        case 4: running = false; continue;
                    }
                }
            }
        } else if (currentScreen == SCREEN_KEYBOARD) {
            textInput += cmd;
            auto pos = findKey(lower);
            hlRow = pos.row;
            hlCol = pos.col;
            if (pos.row >= 0)
                std::cout << "Key: " << kbdRows[pos.row].keys[pos.col].label << std::endl;
        } else if (currentScreen == SCREEN_DIALER) {
            if (lower == "backspace" || lower == "bksp") {
                if (!dialNumber.empty()) dialNumber.pop_back();
                hlDigit = -1;
            } else if (lower == "call" || lower == "llamar") {
                std::cout << "Llamando a " << dialNumber << "..." << std::endl;
            } else if (lower == "end" || lower == "colgar") {
                std::cout << "Llamada finalizada." << std::endl;
            } else if (lower.size() == 1) {
                char c = lower[0];
                if ((c >= '0' && c <= '9') || c == '*' || c == '#') {
                    dialNumber += c;
                    static const char* DK = "123456789*0#";
                    for (int i = 0; i < 12; i++)
                        if (DK[i] == c) { hlDigit = i; break; }
                }
            } else if (lower == "asterisk") {
                dialNumber += '*';
                hlDigit = 9;
            } else if (lower == "hash" || lower == "pound") {
                dialNumber += '#';
                hlDigit = 11;
            }
        } else if (currentScreen == SCREEN_APPS) {
            int n = atoi(cmd.c_str());
            if (n >= 1 && n <= 9) {
                appSel = n - 1;
                static const char* apps[] = {
                    "Contactos","Mensajes","Telefono",
                    "Camara","Ajustes","Musica",
                    "Reloj","Browser","Juegos"
                };
                std::cout << "App: " << apps[appSel] << std::endl;
            } else {
                auto pos = findKey(lower);
                if (pos.row >= 0) {
                    hlRow = pos.row; hlCol = pos.col;
                    std::cout << "Key: " << kbdRows[pos.row].keys[pos.col].label << std::endl;
                }
            }
        }

        redraw(display.get(), hlRow, hlCol, menuCursor, dialNumber, hlDigit, appSel, textInput);
    }

    std::cout << "Shutting down..." << std::endl;
    display.reset();
    bcm2835_close();
    std::cout << "Done." << std::endl;
    return 0;
}
