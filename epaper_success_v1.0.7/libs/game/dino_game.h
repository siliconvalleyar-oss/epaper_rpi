//////////////////////////////////////////////////////////////////////////////
//     
//          filename            :   dino_game.h
//          Description         :   Dino jump game for e-paper display
//          License             :   GNU 
//          Author              :   Lio
//          Hardware            :   Raspberry Pi Zero 2W + e-Paper 2.66"
//     
//////////////////////////////////////////////////////////////////////////////

#pragma once

#include <cstdint>
#include <cstring>

// Screen dimensions
#define SCREEN_W 296
#define SCREEN_H 152

// Game area
#define GROUND_Y    130
#define SKY_Y       20

// Dino size
#define DINO_W      16
#define DINO_H      18
#define DINO_X      40

// Cactus size
#define CACTUS_W    10
#define CACTUS_H    20

// Jump physics
#define GRAVITY     1
#define JUMP_FORCE  -12

class DinoGame {
public:
    DinoGame();
    
    // Game loop
    void update();
    void render(uint8_t* buffer);
    
    // Input
    void jump();
    void autoJump();  // Auto-jump when cactus approaches
    
    // State
    bool isGameOver() const { return m_gameOver; }
    int getScore() const { return m_score; }
    int getCactusX() const { return m_cactusX; }
    bool isJumping() const { return m_jumping; }
    void reset();

private:
    void drawDino(uint8_t* buffer, int x, int y);
    void drawCactus(uint8_t* buffer, int x, int y);
    void drawGround(uint8_t* buffer);
    void drawScore(uint8_t* buffer);
    void drawPixel(uint8_t* buffer, int x, int y);
    
    // Dino state
    int m_dinoY;
    int m_dinoVelY;
    bool m_jumping;
    bool m_duckFrame;
    
    // Cactus
    int m_cactusX;
    int m_cactusType;  // 0: small, 1: medium, 2: large
    
    // Game state
    int m_score;
    int m_speed;
    int m_frameCount;
    bool m_gameOver;
    bool m_gameStarted;
    
    // Frame buffer for double buffering
    uint8_t m_prevBuffer[(SCREEN_W * SCREEN_H) / 8];
};
