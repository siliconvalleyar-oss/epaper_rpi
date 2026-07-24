#include <chrono>
#include <thread>
#include <tyme/tyme.h>

namespace TYME {

    void delay_ms(uint32_t ms) {
        std::this_thread::sleep_for(std::chrono::milliseconds(ms));
    }

    void delay_us(uint32_t us) {
        std::this_thread::sleep_for(std::chrono::microseconds(us));
    }

    void delay_s(uint32_t s) {
        std::this_thread::sleep_for(std::chrono::seconds(s));
    }

    void delay(uint32_t ms) {
        delay_ms(ms);
    }

}
