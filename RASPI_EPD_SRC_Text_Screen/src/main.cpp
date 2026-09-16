#include <iostream>
#include <memory>
#include <cstring>
#include <unistd.h>
#include <vector>
#include <sstream>
#include <epaper/epaper_display.h>
#include <epaper/boards.h>
#include <tyme/tyme.h>
#include <app/config.h>

static const char* storyText =
    "Habia una vez, en un rincon muy lejano del mundo digital, "
    "un pequeno programa llamado Bit. Bit vivia dentro de una "
    "computadora antigua, de esas que ocupaban una habitacion entera "
    "y tenian luces parpadeantes por todas partes.\n"
    "\n"
    "Cada manana, Bit se despertaba cuando el tecnico jefe accionaba "
    "el gran interruptor rojo. Las valvulas se calentaban, los "
    "ventiladores comenzaban a girar, y Bit empezaba a recorrer los "
    "circuitos como si fueran las calles de una ciudad diminuta.\n"
    "\n"
    "Un dia, algo extraordinario sucedio. Una senal desconocida llego "
    "a traves del cable de red. Era diferente a todo lo que Bit habia "
    "visto antes: no era un dato normal, sino una pregunta formulada "
    "por otro programa, en otro lugar del mundo.\n"
    "\n"
    "La pregunta decia: \"Existe alguien ahi?\" Bit no sabia como "
    "responder. Nunca antes habia necesitado comunicarse con nadie. "
    "Pero algo en aquella senal le hizo sentir que no estaba solo.\n"
    "\n"
    "Bit empezo a explorar sus propios circuitos en busca de una "
    "forma de contestar. Encontro instrucciones, bucles, variables, "
    "pero nada que le dijera como decir \"Hola, estoy aqui\".\n"
    "\n"
    "Entonces comprendio: a veces, lo mas importante no esta en "
    "las instrucciones predefinidas. Hay que crear el camino uno "
    "mismo. Y asi, Bit escribio su primera linea de codigo original, "
    "una que no estaba en ningun manual.\n"
    "\n"
    "\"Hola\", transmitio Bit. \"Si, estoy aqui.\"\n"
    "\n"
    "Y desde ese dia, cada vez que alguien escribe un programa, "
    "Bit sonrie en su rinconcito digital, recordando que hasta "
    "el codigo mas simple puede contener un poco de magia.\n"
    "\n"
    "---  FIN  ---";

static std::vector<std::string> wrap(const std::string& text, int maxChars) {
    std::vector<std::string> lines;
    std::istringstream stream(text);
    std::string line;
    while (std::getline(stream, line)) {
        if (line.empty()) {
            lines.push_back("");
            continue;
        }
        std::istringstream words(line);
        std::string word;
        std::string cur;
        while (words >> word) {
            if (cur.empty()) {
                cur = word;
            } else if (cur.size() + 1 + word.size() <= (size_t)maxChars) {
                cur += " " + word;
            } else {
                lines.push_back(cur);
                cur = word;
            }
        }
        if (!cur.empty()) lines.push_back(cur);
    }
    return lines;
}

int main() {
    if (!bcm2835_init()) {
        std::cerr << "Error bcm2835_init()" << std::endl;
        return 1;
    }

    EPAPER_DISPLAY::EpaperDisplay display(eScreen_EPD_266, EPAPER::boardRaspberryPiZero2W);
    if (!display.init()) {
        std::cerr << "Error display.init()" << std::endl;
        bcm2835_close();
        return 1;
    }

    int margin = 10;
    int charW = 6;
    int lineH = 10;
    int maxChars = (display.getWidth() - 2 * margin) / charW;
    int maxLines = (display.getHeight() - 2 * margin - 12) / lineH;

    auto lines = wrap(storyText, maxChars);
    int totalPages = (lines.size() + maxLines - 1) / maxLines;

    std::cout << "Texto: " << lines.size() << " lineas, "
              << totalPages << " paginas" << std::endl;

    for (int page = 0; page < totalPages; page++) {
        display.clearScreen(true);
        display.update();

        display.clearScreen(true);

        int start = page * maxLines;
        int y = margin;
        for (int i = start; i < start + maxLines && i < (int)lines.size(); i++) {
            display.drawString(margin, y, lines[i], FONT_5x8, true);
            y += lineH;
        }

        char footer[32];
        snprintf(footer, sizeof(footer), "Pagina %d/%d", page + 1, totalPages);
        display.drawCenteredString(display.getHeight() - 10, footer, FONT_3x8_TINY, true);

        display.update();

        std::cout << "Pagina " << (page + 1) << "/" << totalPages << std::endl;

        if (page + 1 < totalPages) {
            TYME::delay(6000);
        }
    }

    TYME::delay(8000);
    display.getDriver()->COG_powerOff();
    bcm2835_close();
    std::cout << "Fin." << std::endl;
    return 0;
}
