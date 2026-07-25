#pragma once

#include <cstdint>

namespace TYME {

    void delay_ms(uint32_t ms);
    void delay_us(uint32_t us);
    void delay_s(uint32_t s);
    void delay(uint32_t ms);  // alias de delay_ms

} // namespace TYME
