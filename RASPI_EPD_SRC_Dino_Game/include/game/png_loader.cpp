#include "png_loader.h"
#include <fstream>
#include <vector>
#include "picopng.hpp"

SpriteData loadPNGMonochrome(const std::string& filepath) {
    SpriteData result;

    std::ifstream file(filepath, std::ios::binary | std::ios::ate);
    if (!file) return result;

    std::streamsize size = file.tellg();
    file.seekg(0, std::ios::beg);

    std::vector<unsigned char> buffer(static_cast<size_t>(size));
    if (!file.read(reinterpret_cast<char*>(buffer.data()), size))
        return result;

    std::vector<unsigned char> image;
    unsigned long w, h;
    int error = decodePNG(image, w, h, buffer.data(), buffer.size(), true);
    if (error != 0) return result;

    result.width = static_cast<int>(w);
    result.height = static_cast<int>(h);

    int bytesPerRow = (w + 7) / 8;
    result.data.resize(h * bytesPerRow, 0);

    for (unsigned long y = 0; y < h; y++) {
        for (unsigned long x = 0; x < w; x++) {
            size_t idx = (y * w + x) * 4;
            unsigned char a = image[idx + 3];
            if (a > 128) {
                int byteIdx = y * bytesPerRow + static_cast<int>(x / 8);
                int bitIdx = 7 - static_cast<int>(x % 8);
                result.data[byteIdx] |= (1 << bitIdx);
            }
        }
    }

    return result;
}
