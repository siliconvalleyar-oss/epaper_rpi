//////////////////////////////////////////////////////////////////////////////
//     
//          filename            :   fonts_manger.h
//          License             :   GNU 
//          Author              :   Lio
//          Modified for        :   Raspberry Pi with bcm2835
//          Hardware            :   Raspberry Pi 2W
//          Complier            :   g++
//          Dependencies        :   bcm2835
//
//////////////////////////////////////////////////////////////////////////////
#pragma once

#include <iostream>
#include <string>
#include <cstdint>

// Enum to identify different font types
enum FontType {
    FONT_8x8 = 0,      // Original myc64_lower.64c font (8x8)
    FONT_5x8,          // Standard ASCII 5x8 font (Font_One)
    FONT_7x8_THICK,    // Thick font 7x8 (Font_Two)
    FONT_4x8_SEG,      // Seven segment 4x8 (Font_Three)
    FONT_8x8_WIDE,     // Wide font 8x8 (Font_Four)
    FONT_3x8_TINY,     // Tiny font 3x8 (Font_Five)
    FONT_7x8_HOMESPUN, // Homespun font 7x8 (Font_Six)
    FONT_16x32_BIGNUM, // Big numbers 16x32 (Font_Seven)
    FONT_16x16_MEDNUM  // Medium numbers 16x16 (Font_Eight)
};

// Structure to store font metadata
struct FontInfo {
    FontType type;
    const void* data;      // Pointer to font data
    uint8_t width;         // Character width in pixels
    uint8_t height;        // Character height in pixels
    uint8_t start_char;    // First character in font (usually 32 for space)
    uint8_t end_char;      // Last character in font
    const char* name;
};

class FontManager {
private:
    FontInfo currentFont;
    
    // Helper to get font data based on type
    const void* getFontData(FontType type);
    
    // Get the size of each character in bytes for different fonts
    uint8_t getCharSize(FontType type);
    
public:
    FontManager();
    
    // Change current font
    void setFont(FontType type);
    
    // Get character bitmap data
    const uint8_t* getCharBitmap(char c);
    
    // Print character to console (visual representation)
    void printChar(char c);
    
    // Print string to console
    void printString(const std::string& text);
    
    // Get font information
    void printFontInfo();
    
    // Get current font width
    uint8_t getFontWidth() const { return currentFont.width; }
    
    // Get current font height
    uint8_t getFontHeight() const { return currentFont.height; }
};

// Function to get character width for positioning
int getCharacterWidth(FontType type, char c);

