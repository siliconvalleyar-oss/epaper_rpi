#pragma once
#include <cstdint>
#include <string>
#include <vector>

struct SpriteData {
    int width{0};
    int height{0};
    std::vector<uint8_t> data;
};

SpriteData loadPNGMonochrome(const std::string& filepath);
