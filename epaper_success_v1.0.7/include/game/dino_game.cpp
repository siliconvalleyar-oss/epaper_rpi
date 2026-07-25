//////////////////////////////////////////////////////////////////////////////
//     
//          filename            :   dino_game.cpp
//          Description         :   Dino jump game implementation (PNG sprites)
//          License             :   GNU 
//          Author              :   Lio
//          Hardware            :   Raspberry Pi Zero 2W + e-Paper 2.66"
//     
//////////////////////////////////////////////////////////////////////////////

#include "dino_game.h"
#include "dino_sprites.h"
#include <fonts/fonts_manager.h>
#include <cstring>

DinoGame::DinoGame() {
    m_spritesLoaded = loadSprites();
    if (!m_spritesLoaded) {
        m_dinoW = 32;
        m_dinoH = 32;
    }
    reset();
}

DinoGame::~DinoGame() {}

bool DinoGame::loadSprites() {
    const char* paths[4] = {
        "assets/dino_run_00.png",
        "assets/dino_run_01.png",
        "assets/dino_run_02.png",
        "assets/dino_run_03.png"
    };

    int maxW = 0, maxH = 0;
    SpriteData loaded[4];

    for (int i = 0; i < 4; i++) {
        loaded[i] = loadPNGMonochrome(paths[i]);
        if (loaded[i].width <= 0 || loaded[i].height <= 0)
            return false;
        if (loaded[i].width  > maxW) maxW = loaded[i].width;
        if (loaded[i].height > maxH) maxH = loaded[i].height;
    }

    m_dinoW = maxW;
    m_dinoH = maxH;

    int bytesPerRow = (maxW + 7) / 8;
    for (int i = 0; i < 4; i++) {
        SpriteData& src = loaded[i];
        int srcBPR = (src.width + 7) / 8;
        m_dinoSprites[i].width  = maxW;
        m_dinoSprites[i].height = maxH;
        m_dinoSprites[i].data.resize(maxH * bytesPerRow, 0);

        for (int y = 0; y < src.height; y++) {
            for (int x = 0; x < src.width; x++) {
                int srcByte = y * srcBPR + x / 8;
                int srcBit  = 7 - (x % 8);
                if (src.data[srcByte] & (1 << srcBit)) {
                    int dstByte = y * bytesPerRow + x / 8;
                    int dstBit  = 7 - (x % 8);
                    m_dinoSprites[i].data[dstByte] |= (1 << dstBit);
                }
            }
        }
    }

    return true;
}

void DinoGame::reset() {
    m_dinoY = GROUND_Y - m_dinoH;
    m_dinoVelY = 0;
    m_jumping = false;
    m_ducking = false;
    
    m_obstacleX = SCREEN_W + 80;
    m_obstacleType = OBS_CACTUS;
    m_obstacleY = GROUND_Y - 20;
    
    for (int i = 0; i < 3; i++) {
        m_cloudX[i] = 200 + i * 100 + (i * 37) % 60;
        m_cloudY[i] = 25 + (i * 23) % 20;
    }
    
    m_score = 0;
    m_speed = 3;
    m_frameCount = 0;
    m_gameOver = false;
    m_gameStarted = true;
    
    memset(m_prevBuffer, 0x00, sizeof(m_prevBuffer));
}

void DinoGame::jump() {
    if (!m_jumping && !m_gameOver) {
        m_dinoVelY = JUMP_FORCE;
        m_jumping = true;
    } else if (m_gameOver) {
        reset();
    }
}

void DinoGame::autoJump() {
    if (!m_jumping && !m_gameOver) {
        int jumpZone = DINO_X + m_dinoW + 40;
        if (m_obstacleX < jumpZone && m_obstacleX > DINO_X - 10) {
            m_dinoVelY = JUMP_FORCE;
            m_jumping = true;
        }
    } else if (m_gameOver) {
        reset();
    }
}

void DinoGame::update() {
    if (m_gameOver) return;
    
    m_frameCount++;
    
    if (m_frameCount % 10 == 0) {
        m_score++;
    }
    
    if (m_score % 100 == 0 && m_score > 0) {
        m_speed++;
        if (m_speed > 8) m_speed = 8;
    }
    
    if (m_jumping) {
        m_dinoY += m_dinoVelY;
        m_dinoVelY += GRAVITY;
        
        if (m_dinoY >= GROUND_Y - m_dinoH) {
            m_dinoY = GROUND_Y - m_dinoH;
            m_dinoVelY = 0;
            m_jumping = false;
        }
    }
    
    m_obstacleX -= m_speed;
    
    if (m_obstacleX < -BIRD_W) {
        m_obstacleX = SCREEN_W + 80 + (m_frameCount % 80);
        if ((m_frameCount % 5) == 0 && m_score > 50) {
            m_obstacleType = OBS_BIRD;
            m_obstacleY = GROUND_Y - 28 - (m_frameCount % 20);
        } else {
            m_obstacleType = OBS_CACTUS;
            m_obstacleY = GROUND_Y - 20;
        }
    }
    
    for (int i = 0; i < 3; i++) {
        m_cloudX[i] -= 1;
        if (m_cloudX[i] < -CLOUD_W) {
            m_cloudX[i] = SCREEN_W + 20 + (i * 97) % 60;
            m_cloudY[i] = 25 + (i * 23) % 20;
        }
    }
    
    int dinoLeft = DINO_X + 4;
    int dinoRight = DINO_X + m_dinoW - 4;
    int dinoTop = m_dinoY + 4;
    int dinoBottom = m_dinoY + m_dinoH - 4;
    
    int obsLeft = m_obstacleX + 2;
    int obsRight = m_obstacleX + BIRD_W - 2;
    int obsTop = m_obstacleY + 2;
    int obsBottom = m_obstacleY + BIRD_H - 2;
    
    if (dinoRight > obsLeft && dinoLeft < obsRight &&
        dinoBottom > obsTop && dinoTop < obsBottom) {
        m_gameOver = true;
    }
}

void DinoGame::drawPixel(uint8_t* buffer, int x, int y) {
    if (x < 0 || x >= SCREEN_W || y < 0 || y >= SCREEN_H) return;
    
    int mirroredX = SCREEN_W - 1 - x;
    int byteIndex = (mirroredX * SCREEN_H + y) / 8;
    int bitIndex = 7 - (y % 8);
    buffer[byteIndex] |= (1 << bitIndex);
}

void DinoGame::drawSprite(uint8_t* buffer, const uint8_t* sprite, int x, int y) {
    for (int row = 0; row < SPRITE_H; row++) {
        for (int col = 0; col < SPRITE_W; col++) {
            int byteIdx = row * SPRITE_BYTES_PER_ROW + (col / 8);
            int bitIdx = 7 - (col % 8);
            
            if (sprite[byteIdx] & (1 << bitIdx)) {
                drawPixel(buffer, x + col, y + row);
            }
        }
    }
}

void DinoGame::drawSpriteData(uint8_t* buffer, const SpriteData& sprite, int x, int y) {
    int bytesPerRow = (sprite.width + 7) / 8;
    for (int row = 0; row < sprite.height; row++) {
        for (int col = 0; col < sprite.width; col++) {
            int byteIdx = row * bytesPerRow + (col / 8);
            int bitIdx = 7 - (col % 8);
            
            if (sprite.data[byteIdx] & (1 << bitIdx)) {
                drawPixel(buffer, x + col, y + row);
            }
        }
    }
}

void DinoGame::drawDino(uint8_t* buffer, int x, int y) {
    if (m_spritesLoaded) {
        int frame = (m_frameCount / 5) % 4;
        drawSpriteData(buffer, m_dinoSprites[frame], x, y);
        return;
    }

    const uint8_t* sprite;
    if (m_jumping) {
        sprite = sprite_dino_jump;
    } else if (m_ducking) {
        sprite = (m_frameCount % 10 < 5) ? sprite_dino_duck1 : sprite_dino_duck2;
    } else {
        sprite = (m_frameCount % 10 < 5) ? sprite_dino_run1 : sprite_dino_run2;
    }
    drawSprite(buffer, sprite, x, y);
}

void DinoGame::drawObstacle(uint8_t* buffer, int x, int y) {
    if (m_obstacleType == OBS_BIRD) {
        const uint8_t* birdSprite = (m_frameCount % 8 < 4) ? sprite_bird1 : sprite_bird2;
        drawSprite(buffer, birdSprite, x, y);
    } else {
        int trunkW = 6;
        int trunkH = 20;
        
        for (int row = 0; row < trunkH; row++) {
            for (int col = 0; col < trunkW; col++) {
                drawPixel(buffer, x + col, y + row);
            }
        }
        
        for (int row = 0; row < 4; row++) {
            drawPixel(buffer, x - 1, y + 5 + row);
            drawPixel(buffer, x - 2, y + 5 + row);
        }
        for (int col = 0; col < 3; col++) {
            drawPixel(buffer, x - 4 + col, y + 5);
        }
        
        for (int row = 0; row < 4; row++) {
            drawPixel(buffer, x + trunkW, y + 7 + row);
            drawPixel(buffer, x + trunkW + 1, y + 7 + row);
        }
        for (int col = 0; col < 3; col++) {
            drawPixel(buffer, x + trunkW + 1 + col, y + 7);
        }
    }
}

void DinoGame::drawCloud(uint8_t* buffer, int x, int y) {
    drawSprite(buffer, sprite_cloud1, x, y);
}

void DinoGame::drawGround(uint8_t* buffer) {
    for (int x = 0; x < SCREEN_W; x++) {
        drawPixel(buffer, x, GROUND_Y);
        drawPixel(buffer, x, GROUND_Y + 1);
        
        if (x % 4 < 2) {
            drawPixel(buffer, x, GROUND_Y + 3);
        }
    }
}

void DinoGame::drawScore(uint8_t* buffer) {
    FontManager fm;
    fm.setFont(FONT_5x8);
    
    char scoreStr[16];
    snprintf(scoreStr, sizeof(scoreStr), "SCORE: %04d", m_score);
    
    int x = 10;
    for (int i = 0; scoreStr[i] != '\0'; i++) {
        const uint8_t* bitmap = fm.getCharBitmap(scoreStr[i]);
        if (bitmap) {
            for (int col = 0; col < 5; col++) {
                uint8_t byte = bitmap[col];
                for (int row = 0; row < 8; row++) {
                    if ((byte >> row) & 0x01) {
                        drawPixel(buffer, x + col, SKY_Y + row);
                    }
                }
            }
        }
        x += 6;
    }
    
    char hiStr[16];
    snprintf(hiStr, sizeof(hiStr), "HI: %04d", m_score > 100 ? m_score - 100 : 0);
    
    x = SCREEN_W - 80;
    for (int i = 0; hiStr[i] != '\0'; i++) {
        const uint8_t* bitmap = fm.getCharBitmap(hiStr[i]);
        if (bitmap) {
            for (int col = 0; col < 5; col++) {
                uint8_t byte = bitmap[col];
                for (int row = 0; row < 8; row++) {
                    if ((byte >> row) & 0x01) {
                        drawPixel(buffer, x + col, SKY_Y + row);
                    }
                }
            }
        }
        x += 6;
    }
}

void DinoGame::render(uint8_t* buffer) {
    memset(buffer, 0x00, (SCREEN_W * SCREEN_H) / 8);
    
    drawGround(buffer);
    
    for (int i = 0; i < 3; i++) {
        drawCloud(buffer, m_cloudX[i], m_cloudY[i]);
    }
    
    drawDino(buffer, DINO_X, m_dinoY);
    
    drawObstacle(buffer, m_obstacleX, m_obstacleY);
    
    drawScore(buffer);
    
    if (m_gameOver) {
        FontManager fm;
        fm.setFont(FONT_7x8_THICK);
        
        const char* gameOverStr = "GAME OVER";
        int x = (SCREEN_W - 9 * 7) / 2;
        for (int i = 0; gameOverStr[i] != '\0'; i++) {
            const uint8_t* bitmap = fm.getCharBitmap(gameOverStr[i]);
            if (bitmap) {
                for (int col = 0; col < 7; col++) {
                    uint8_t byte = bitmap[col];
                    for (int row = 0; row < 8; row++) {
                        if ((byte >> row) & 0x01) {
                            drawPixel(buffer, x + col, 70 + row);
                        }
                    }
                }
            }
            x += 8;
        }
        
        fm.setFont(FONT_5x8);
        const char* restartStr = "Press button to restart";
        x = (SCREEN_W - 22 * 6) / 2;
        for (int i = 0; restartStr[i] != '\0'; i++) {
            const uint8_t* bitmap = fm.getCharBitmap(restartStr[i]);
            if (bitmap) {
                for (int col = 0; col < 5; col++) {
                    uint8_t byte = bitmap[col];
                    for (int row = 0; row < 8; row++) {
                        if ((byte >> row) & 0x01) {
                            drawPixel(buffer, x + col, 85 + row);
                        }
                    }
                }
            }
            x += 6;
        }
    }
}
