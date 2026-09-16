//////////////////////////////////////////////////////////////////////////////
//     
//          filename            :   fonts_manager.cpp
//          License             :   GNU 
//          Author              :   Lio
//          Modified for        :   Raspberry Pi with bcm2835
//          Hardware            :   Raspberry Pi 2W
//          Complier            :   g++
//          Dependencies        :   bcm2835
//
//////////////////////////////////////////////////////////////////////////////
// fonts_manager.cpp
// A C++ implementation for managing multiple bitmap fonts
// Includes all font definitions and methods to render characters
#include <fonts/fonts.h>
#include <fonts/fonts_manager.h>
#include <cstring>


// Helper to get font data based on type
const void* FontManager::getFontData(FontType type) {
    switch(type) {
        case FONT_8x8: return font;
        case FONT_5x8: return Font_One;
        case FONT_7x8_THICK: return Font_Two;
        case FONT_4x8_SEG: return Font_Three;
        case FONT_8x8_WIDE: return Font_Four;
        case FONT_3x8_TINY: return Font_Five;
        case FONT_7x8_HOMESPUN: return Font_Six;
        case FONT_16x32_BIGNUM: return Font_Seven;
        case FONT_16x16_MEDNUM: return Font_Eight;
        default: return Font_One;
    }
}

// Get the size of each character in bytes for different fonts
uint8_t FontManager::getCharSize(FontType type) {
    switch(type) {
        case FONT_8x8: return 8;          // 8 bytes per character
        case FONT_5x8: return 5;          // 5 bytes per character
        case FONT_7x8_THICK: return 7;    // 7 bytes per character
        case FONT_4x8_SEG: return 4;      // 4 bytes per character
        case FONT_8x8_WIDE: return 8;     // 8 bytes per character
        case FONT_3x8_TINY: return 3;     // 3 bytes per character
        case FONT_7x8_HOMESPUN: return 7; // 7 bytes per character
        case FONT_16x32_BIGNUM: return 64; // 64 bytes per character
        case FONT_16x16_MEDNUM: return 32; // 32 bytes per character
        default: return 5;
    }
}

FontManager::FontManager() {
    // Default to 5x8 font
    currentFont.type = FONT_5x8;
    currentFont.data = Font_One;
    currentFont.width = 5;
    currentFont.height = 8;
    currentFont.start_char = 32;
    currentFont.end_char = 127;
    currentFont.name = "Standard 5x8";
}

// Change current font
void FontManager::setFont(FontType type) {
    currentFont.type = type;
    currentFont.data = getFontData(type);
    
    // Set dimensions based on font type
    switch(type) {
        case FONT_8x8:
            currentFont.width = 8;
            currentFont.height = 8;
            currentFont.name = "8x8 (myc64_lower)";
            currentFont.start_char = 32;
            currentFont.end_char = 127;
            break;
        case FONT_5x8:
            currentFont.width = 5;
            currentFont.height = 8;
            currentFont.name = "5x8 Standard";
            currentFont.start_char = 32;
            currentFont.end_char = 127;
            break;
        case FONT_7x8_THICK:
            currentFont.width = 7;
            currentFont.height = 8;
            currentFont.name = "7x8 Thick";
            currentFont.start_char = 32;
            currentFont.end_char = 127;
            break;
        case FONT_4x8_SEG:
            currentFont.width = 4;
            currentFont.height = 8;
            currentFont.name = "4x8 Seven Segment";
            currentFont.start_char = 32;
            currentFont.end_char = 127;
            break;
        case FONT_8x8_WIDE:
            currentFont.width = 8;
            currentFont.height = 8;
            currentFont.name = "8x8 Wide";
            currentFont.start_char = 32;
            currentFont.end_char = 127;
            break;
        case FONT_3x8_TINY:
            currentFont.width = 3;
            currentFont.height = 8;
            currentFont.name = "3x8 Tiny";
            currentFont.start_char = 32;
            currentFont.end_char = 127;
            break;
        case FONT_7x8_HOMESPUN:
            currentFont.width = 7;
            currentFont.height = 8;
            currentFont.name = "7x8 Homespun";
            currentFont.start_char = 32;
            currentFont.end_char = 127;
            break;
        case FONT_16x32_BIGNUM:
            currentFont.width = 16;
            currentFont.height = 32;
            currentFont.name = "16x32 Big Numbers";
            currentFont.start_char = 48;  // '0'
            currentFont.end_char = 58;    // '9' + ':'
            break;
        case FONT_16x16_MEDNUM:
            currentFont.width = 16;
            currentFont.height = 16;
            currentFont.name = "16x16 Medium Numbers";
            currentFont.start_char = 48;  // '0'
            currentFont.end_char = 58;    // '9' + ':'
            break;
    }
}

// Get character bitmap data
const uint8_t* FontManager::getCharBitmap(char c) {
    int index = (int)c - currentFont.start_char;
    if (index < 0 || index > (currentFont.end_char - currentFont.start_char)) {
        return nullptr; // Character not in font
    }
    
    size_t charSize = getCharSize(currentFont.type);
    const uint8_t* fontData = static_cast<const uint8_t*>(currentFont.data);
    
    // Handle 2D arrays for big number fonts
    if (currentFont.type == FONT_16x32_BIGNUM) {
        // Font_Seven is a 2D array [11][64]
        // Each character is 64 bytes
        const uint8_t (*bigFont)[64] = static_cast<const uint8_t(*)[64]>(currentFont.data);
        return bigFont[index];
    } else if (currentFont.type == FONT_16x16_MEDNUM) {
        // Font_Eight is a 2D array [11][32]
        // Each character is 32 bytes
        const uint8_t (*medFont)[32] = static_cast<const uint8_t(*)[32]>(currentFont.data);
        return medFont[index];
    } else {
        return &fontData[index * charSize];
    }
}

// Print character to console (visual representation)
void FontManager::printChar(char c) {
    const uint8_t* bitmap = getCharBitmap(c);
    if (!bitmap) {
        std::cout << "?" << std::endl;
        return;
    }
    
    size_t charSize = getCharSize(currentFont.type);
    
    // Print the character as ASCII art
    for (int row = 0; row < currentFont.height; row++) {
        for (int col = 0; col < currentFont.width; col++) {
            // For fonts where each byte represents a column
            if (currentFont.type != FONT_16x32_BIGNUM && 
                currentFont.type != FONT_16x16_MEDNUM) {
                // Regular fonts: each byte is a column
                if (col < (int)charSize) {
                    uint8_t byte = bitmap[col];
                    bool pixel = (byte >> row) & 0x01;
                    std::cout << (pixel ? "#" : " ");
                } else {
                    std::cout << " ";
                }
            } else {
                // Big number fonts: bitmap is row-major (16x16 or 16x32)
                // For demonstration, show a simplified representation
                if (row < currentFont.height && col < currentFont.width) {
                    int byteIndex = (row * (currentFont.width / 8)) + (col / 8);
                    int bitIndex = 7 - (col % 8);
                    if (byteIndex < (int)charSize) {
                        bool pixel = (bitmap[byteIndex] >> bitIndex) & 0x01;
                        std::cout << (pixel ? "#" : " ");
                    } else {
                        std::cout << " ";
                    }
                }
            }
        }
        std::cout << std::endl;
    }
}

// Print string to console
void FontManager::printString(const std::string& text) {
    std::cout << "\n=== Using font: " << currentFont.name << " ===" << std::endl;
    std::cout << "Width: " << (int)currentFont.width << ", Height: " << (int)currentFont.height << std::endl;
    std::cout << "----------------------------------------" << std::endl;
    
    for (char c : text) {
        std::cout << "Character: '" << c << "'" << std::endl;
        printChar(c);
        std::cout << std::endl;
    }
}

// Get font information
void FontManager::printFontInfo() {
    std::cout << "Font: " << currentFont.name << std::endl;
    std::cout << "Dimensions: " << (int)currentFont.width << "x" << (int)currentFont.height << std::endl;
    std::cout << "Character range: " << (int)currentFont.start_char << " to " << (int)currentFont.end_char << std::endl;
}

// Function to get character width for positioning
int getCharacterWidth(FontType type, char c) {
    FontManager temp;
    temp.setFont(type);
    const uint8_t* bitmap = temp.getCharBitmap(c);
    if (bitmap) {
        return temp.getFontWidth();
    }
    return 0;
}