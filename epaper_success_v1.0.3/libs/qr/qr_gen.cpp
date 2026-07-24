#include <qr/qr_gen.h>
#include <iostream>

namespace QR{        

    void Qr_gen_t::setPixel(int x, int y, bool isBlack) {
        if (x < 0 || x >= IMAGE_WIDTH || y < 0 || y >= IMAGE_HEIGHT) return;
        int byteIndex = (y * BYTES_PER_ROW) + (x / 8);
        int bitIndex = 7 - (x % 8);
        if (isBlack) {
            imageBuffer[byteIndex] |= (1 << bitIndex);
        } else {
            imageBuffer[byteIndex] &= ~(1 << bitIndex);
        }
    }

    void Qr_gen_t::drawQRCode(const char* data, int scaleFactor) {
        QRcode* qrcode = QRcode_encodeString(data, 0, QR_ECLEVEL_L, QR_MODE_8, 1);
        if (!qrcode) {
            std::cerr << "Error al generar el código QR." << std::endl;
            return;
        }
        
        int qrSize = qrcode->width;
        int scaledQrSize = qrSize * scaleFactor;
        int xOffset = (IMAGE_WIDTH - scaledQrSize) / 2;
        int yOffset = (IMAGE_HEIGHT - scaledQrSize) / 2;
        
        for (int y = 0; y < qrSize; y++) {
            for (int x = 0; x < qrSize; x++) {
                bool isBlack = qrcode->data[y * qrSize + x] & 0x01;
                for (int dy = 0; dy < scaleFactor; ++dy) {
                    for (int dx = 0; dx < scaleFactor; ++dx) {
                        setPixel(x * scaleFactor + dx + xOffset, y * scaleFactor + dy + yOffset, isBlack);
                    }
                }
            }
        }
        
        QRcode_free(qrcode);
    }

    int Qr_gen_t::qr_generator() {
        const char* networkData = "WIFI:T:WPA;S:SSID;P:Password;;";
        std::memset(imageBuffer, 0x00, sizeof(imageBuffer));
        int scaleFactor = SCALE;
        drawQRCode(networkData, scaleFactor);
        return 0;
    }

}
