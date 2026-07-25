//////////////////////////////////////////////////////////////////////////////
//     
//          filename            :   dino_game.cpp
//          Description         :   Dino jump game implementation
//          License             :   GNU 
//          Author              :   Lio
//          Hardware            :   Raspberry Pi Zero 2W + e-Paper 2.66"
//     
//////////////////////////////////////////////////////////////////////////////

#include "dino_game.h"
#include <fonts/fonts_manager.h>
#include <cstring>

// Dino sprite (16x18 pixels) - T-Rex standing
// Row format: 2 bytes per row, MSB first
static const uint8_t dino_sprite[] = {
    0x00, 0x00,  // row 0:  ................
    0x00, 0x00,  // row 1:  ................
    0x00, 0xE0,  // row 2:  .......###......
    0x01, 0xF0,  // row 3:  ......#####.....
    0x01, 0xF0,  // row 4:  ......#####.....
    0x01, 0xF0,  // row 5:  ......#####.....
    0x00, 0xE0,  // row 6:  .......###......
    0x00, 0x40,  // row 7:  .......#........
    0x00, 0xE0,  // row 8:  .......###......
    0x01, 0xF0,  // row 9:  ......#####.....
    0x03, 0xF8,  // row 10: ..##########....
    0x07, 0xFC,  // row 11: ..############..
    0x0F, 0xFE,  // row 12: ..##############.
    0x0F, 0xFE,  // row 13: ..##############.
    0x07, 0xFC,  // row 14: ..############..
    0x03, 0xF8,  // row 15: ..##########....
    0x00, 0x00,  // row 16: ................
    0x00, 0x00,  // row 17: ................
};

// Dino running frame 1 (left leg forward)
static const uint8_t dino_run1[] = {
    0x00, 0x00,  // row 0:  ................
    0x00, 0x00,  // row 1:  ................
    0x00, 0xE0,  // row 2:  .......###......
    0x01, 0xF0,  // row 3:  ......#####.....
    0x01, 0xF0,  // row 4:  ......#####.....
    0x01, 0xF0,  // row 5:  ......#####.....
    0x00, 0xE0,  // row 6:  .......###......
    0x00, 0x40,  // row 7:  .......#........
    0x00, 0xE0,  // row 8:  .......###......
    0x01, 0xF0,  // row 9:  ......#####.....
    0x03, 0xF8,  // row 10: ..##########....
    0x07, 0xFC,  // row 11: ..############..
    0x0F, 0xFE,  // row 12: ..##############.
    0x0F, 0xFE,  // row 13: ..##############.
    0x07, 0xFC,  // row 14: ..############..
    0x02, 0x10,  // row 15: ......#....#....
    0x01, 0x20,  // row 16: .......#..#.....
    0x00, 0x00,  // row 17: ................
};

// Dino running frame 2 (right leg forward)
static const uint8_t dino_run2[] = {
    0x00, 0x00,  // row 0:  ................
    0x00, 0x00,  // row 1:  ................
    0x00, 0xE0,  // row 2:  .......###......
    0x01, 0xF0,  // row 3:  ......#####.....
    0x01, 0xF0,  // row 4:  ......#####.....
    0x01, 0xF0,  // row 5:  ......#####.....
    0x00, 0xE0,  // row 6:  .......###......
    0x00, 0x40,  // row 7:  .......#........
    0x00, 0xE0,  // row 8:  .......###......
    0x01, 0xF0,  // row 9:  ......#####.....
    0x03, 0xF8,  // row 10: ..##########....
    0x07, 0xFC,  // row 11: ..############..
    0x0F, 0xFE,  // row 12: ..##############.
    0x0F, 0xFE,  // row 13: ..##############.
    0x07, 0xFC,  // row 14: ..############..
    0x01, 0x20,  // row 15: .......#..#.....
    0x02, 0x10,  // row 16: ......#....#....
    0x00, 0x00,  // row 17: ................
};

DinoGame::DinoGame() {
    reset();
}

void DinoGame::reset() {
    m_dinoY = GROUND_Y - DINO_H;
    m_dinoVelY = 0;
    m_jumping = false;
    m_duckFrame = false;
    
    m_cactusX = SCREEN_W + 50;
    m_cactusType = 0;
    
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
    // Auto-jump when cactus is within jump range
    if (!m_jumping && !m_gameOver) {
        int jumpZone = DINO_X + DINO_W + 40;  // Jump when cactus is 40px ahead
        if (m_cactusX < jumpZone && m_cactusX > DINO_X - 20) {
            m_dinoVelY = JUMP_FORCE;
            m_jumping = true;
        }
    } else if (m_gameOver) {
        // Auto-restart after game over
        reset();
    }
}

void DinoGame::update() {
    if (m_gameOver) return;
    
    m_frameCount++;
    
    // Update score
    if (m_frameCount % 10 == 0) {
        m_score++;
    }
    
    // Increase speed over time
    if (m_score % 100 == 0 && m_score > 0) {
        m_speed++;
        if (m_speed > 8) m_speed = 8;
    }
    
    // Update dino physics
    if (m_jumping) {
        m_dinoY += m_dinoVelY;
        m_dinoVelY += GRAVITY;
        
        // Land on ground
        if (m_dinoY >= GROUND_Y - DINO_H) {
            m_dinoY = GROUND_Y - DINO_H;
            m_dinoVelY = 0;
            m_jumping = false;
        }
    }
    
    // Update cactus
    m_cactusX -= m_speed;
    
    // Spawn new cactus when off screen
    if (m_cactusX < -CACTUS_W) {
        m_cactusX = SCREEN_W + 50 + (m_frameCount % 100);
        m_cactusType = m_frameCount % 3;
    }
    
    // Collision detection
    int dinoLeft = DINO_X;
    int dinoRight = DINO_X + DINO_W - 4;
    int dinoTop = m_dinoY + 2;
    int dinoBottom = m_dinoY + DINO_H - 2;
    
    int cactusLeft = m_cactusX + 2;
    int cactusRight = m_cactusX + CACTUS_W - 2;
    int cactusTop = GROUND_Y - CACTUS_H;
    int cactusBottom = GROUND_Y;
    
    if (dinoRight > cactusLeft && dinoLeft < cactusRight &&
        dinoBottom > cactusTop && dinoTop < cactusBottom) {
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

void DinoGame::drawDino(uint8_t* buffer, int x, int y) {
    // Alternate between running frames based on frame count
    const uint8_t* sprite;
    if (m_jumping) {
        sprite = dino_sprite;  // Standing when jumping
    } else if (m_frameCount % 10 < 5) {
        sprite = dino_run1;
    } else {
        sprite = dino_run2;
    }
    
    // Sprite is 16 pixels wide, 2 bytes per row, 18 rows
    for (int row = 0; row < DINO_H; row++) {
        for (int col = 0; col < DINO_W; col++) {
            int byteIdx = row * 2 + (col / 8);
            int bitIdx = 7 - (col % 8);
            
            if (sprite[byteIdx] & (1 << bitIdx)) {
                drawPixel(buffer, x + col, y + row);
            }
        }
    }
}

void DinoGame::drawCactus(uint8_t* buffer, int x, int y) {
    // Cactus: taller trunk with arms
    int trunkW = 6;
    int trunkH = CACTUS_H;
    
    // Draw trunk
    for (int row = 0; row < trunkH; row++) {
        for (int col = 0; col < trunkW; col++) {
            drawPixel(buffer, x + col, y + row);
        }
    }
    
    // Draw left arm (goes up then left)
    for (int row = 0; row < 4; row++) {
        drawPixel(buffer, x - 1, y + 5 + row);
        drawPixel(buffer, x - 2, y + 5 + row);
    }
    for (int col = 0; col < 3; col++) {
        drawPixel(buffer, x - 4 + col, y + 5);
    }
    
    // Draw right arm (goes up then right)
    for (int row = 0; row < 4; row++) {
        drawPixel(buffer, x + trunkW, y + 7 + row);
        drawPixel(buffer, x + trunkW + 1, y + 7 + row);
    }
    for (int col = 0; col < 3; col++) {
        drawPixel(buffer, x + trunkW + 1 + col, y + 7);
    }
}

void DinoGame::drawGround(uint8_t* buffer) {
    // Draw ground line
    for (int x = 0; x < SCREEN_W; x++) {
        drawPixel(buffer, x, GROUND_Y);
        if (x % 3 == 0) {
            drawPixel(buffer, x, GROUND_Y + 2);
        }
    }
}

void DinoGame::drawScore(uint8_t* buffer) {
    FontManager fm;
    fm.setFont(FONT_5x8);
    
    // Draw "SCORE: XXXX"
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
    
    // Draw "HI: XXXX" on the right
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
    // Clear buffer
    memset(buffer, 0x00, (SCREEN_W * SCREEN_H) / 8);
    
    // Draw ground
    drawGround(buffer);
    
    // Draw dino
    drawDino(buffer, DINO_X, m_dinoY);
    
    // Draw cactus
    drawCactus(buffer, m_cactusX, GROUND_Y - CACTUS_H);
    
    // Draw score
    drawScore(buffer);
    
    // Draw game over text
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
    
    // Draw clouds (decorative)
    for (int i = 0; i < 3; i++) {
        int cx = 50 + i * 90;
        int cy = 35 + (i % 2) * 10;
        for (int w = 0; w < 20; w++) {
            drawPixel(buffer, cx + w, cy);
            if (w > 3 && w < 17) {
                drawPixel(buffer, cx + w, cy - 2);
            }
        }
    }
}
