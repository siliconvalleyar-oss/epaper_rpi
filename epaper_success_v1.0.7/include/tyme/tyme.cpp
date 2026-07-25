#include "tyme.h"
#include <unistd.h>

namespace TYME {

void delay_ms(uint32_t ms) {
    usleep(ms * 1000);
}

void delay_us(uint32_t us) {
    usleep(us);
}

void delay_s(uint32_t s) {
    sleep(s);
}

void delay(uint32_t ms) {
    delay_ms(ms);
}

} // namespace TYME
