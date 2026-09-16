#include <gpio/gpio.h>
#include <bcm2835.h>
#include <iostream>
#include <string>
#include <memory>
#include <vector>

namespace GPIO {

Gpio_t::Gpio_t(bool& status) : m_state(status) {
    if (!bcm2835_init()) {
        std::cerr << "Error iniciando bcm2835" << std::endl;
        m_state = false;
    } else {
        m_state = true;
    }
}

Gpio_t::~Gpio_t() {
    bcm2835_close();
}

int Gpio_t::digitalWrite(uint16_t pin, uint8_t value) {
    if (!m_state) return -1;
    bcm2835_gpio_write(pin, value);
    return 0;
}

int Gpio_t::pinMode(uint16_t pin, uint8_t mode) {
    if (!m_state) return -1;
    bcm2835_gpio_fsel(pin, mode);
    return 0;
}

int Gpio_t::digitalRead(int pin) {
    if (!m_state) return -1;
    return bcm2835_gpio_lev(pin) ? 1 : 0;
}

void Gpio_t::CloseGpios() {
    if (m_state) {
        bcm2835_close();
        m_state = false;
    }
}

void Gpio_t::addGpio(uint16_t gpio_pin, std::string dir, std::string edge, std::string value) {
    int id = getNextId();
    auto gpio = std::make_unique<GpioConform_t>(id, gpio_pin, std::move(dir), std::move(edge), std::move(value), true);
    m_gpio_cfg.push_back(std::move(gpio));
    gpioById[id] = m_gpio_cfg.back().get();
    gpioByPin[gpio_pin] = m_gpio_cfg.back().get();
}

int Gpio_t::getNextId() {
    int max_id = -1;
    for (const auto& gpioPtr : m_gpio_cfg) {
        if (gpioPtr->ID > max_id) {
            max_id = gpioPtr->ID;
        }
    }
    return max_id + 1;
}

void Gpio_t::updateGpioMaps() {
    gpioById.clear();
    gpioByPin.clear();
    for (const auto& gpioPtr : m_gpio_cfg) {
        gpioById[gpioPtr->ID] = gpioPtr.get();
        gpioByPin[gpioPtr->gpio] = gpioPtr.get();
    }
}

void Gpio_t::printGpios() const {
    for (const auto& gpioPtr : m_gpio_cfg) {
        const GpioConform_t& gpio = *gpioPtr;
        std::cout << "ID: " << gpio.ID
                  << ", GPIO: " << gpio.gpio
                  << ", Direction: " << gpio.dir
                  << ", Edge: " << gpio.edge
                  << ", Value: " << gpio.value << "\n";
    }
}

} // namespace GPIO
