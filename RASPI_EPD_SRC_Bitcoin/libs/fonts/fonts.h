//////////////////////////////////////////////////////////////////////////////
//     
//          filename            :   fonts.h
//          License             :   GNU 
//          Author              :   Lio
//          Modified for        :   Raspberry Pi with bcm2835
//          Hardware            :   Raspberry Pi 2W
//          Complier            :   g++
//          Dependencies        :   bcm2835
//
//////////////////////////////////////////////////////////////////////////////
#ifndef FONTS_H
#define FONTS_H

#include <cstdint>

// Declaraciones de todas las fuentes
extern uint8_t font[1024];
extern const unsigned char Font_One[];
extern const unsigned char Font_Two[];
extern const unsigned char Font_Three[];
extern const unsigned char Font_Four[];
extern const unsigned char Font_Five[];
extern const unsigned char Font_Six[];
extern const uint8_t Font_Seven[11][64];
extern const uint8_t Font_Eight[11][32];

// Punteros para acceso rápido
extern const unsigned char* pFontDefaultptr;
extern const unsigned char* pFontThickptr;
extern const unsigned char* pFontSevenSegptr;
extern const unsigned char* pFontWideptr;
extern const unsigned char* pFontTinyptr;
extern const unsigned char* pFontHomeSpunptr;
extern const uint8_t* pFontBigNumptr;
extern const uint8_t* pFontMedNumptr;

#endif // FONTS_H

