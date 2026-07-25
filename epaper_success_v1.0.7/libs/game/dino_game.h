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

// Dino size (32x32 sprites)
#define DINO_W      32
#define DINO_H      32
#define DINO_X      30

// Bird size (32x32 sprites)
#define BIRD_W      32
#define BIRD_H      32

// Cactus size (kept for backwards compat, but removed from game)
#define CACTUS_W    10
#define CACTUS_H    20

// Cloud size
#define CLOUD_W     32
#define CLOUD_H     32

// Jump physics
#define GRAVITY     1
#define JUMP_FORCE  -14

// Obstacle types
#define OBS_CACTUS  0
#define OBS_BIRD    1

class DinoGame {
public:
    DinoGame();
    
    void update();
    void render(uint8_t* buffer);
    
    void jump();
    void autoJump();
    
    bool isGameOver() const { return m_gameOver; }
    int getScore() const { return m_score; }
    int getObstacleX() const { return m_obstacleX; }
    bool isJumping() const { return m_jumping; }
    void reset();

private:
    void drawDino(uint8_t* buffer, int x, int y);
    void drawObstacle(uint8_t* buffer, int x, int y);
    void drawCloud(uint8_t* buffer, int x, int y);
    void drawGround(uint8_t* buffer);
    void drawScore(uint8_t* buffer);
    void drawPixel(uint8_t* buffer, int x, int y);
    void drawSprite(uint8_t* buffer, const uint8_t* sprite, int x, int y);
    
    int m_dinoY;
    int m_dinoVelY;
    bool m_jumping;
    bool m_ducking;
    
    // Obstacle
    int m_obstacleX;
    int m_obstacleType;  // OBS_CACTUS or OBS_BIRD
    int m_obstacleY;
    
    // Clouds
    int m_cloudX[3];
    int m_cloudY[3];
    
    // Game state
    int m_score;
    int m_speed;
    int m_frameCount;
    bool m_gameOver;
    bool m_gameStarted;
    
    uint8_t m_prevBuffer[(SCREEN_W * SCREEN_H) / 8];
};
