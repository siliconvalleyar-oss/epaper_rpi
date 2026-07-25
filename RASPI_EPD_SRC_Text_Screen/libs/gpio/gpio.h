#pragma once
#include <cstdint>
#include <string>
#include <string_view>
#include <memory>
#include <vector>
#include <unordered_map>

namespace GPIO {

struct GpioConform_t {
    int         ID;
    uint16_t    gpio;
    std::string dir;
    std::string edge;
    std::string value;
    bool        status;

    GpioConform_t() = default;
    GpioConform_t(int id, uint16_t g, std::string d, std::string e, std::string v, bool s)
        : ID(id), gpio(g), dir(std::move(d)), edge(std::move(e)), value(std::move(v)), status(s) {}

    GpioConform_t(GpioConform_t&&) noexcept = default;
    GpioConform_t& operator=(GpioConform_t&&) noexcept = default;
    GpioConform_t(const GpioConform_t&) = delete;
    GpioConform_t& operator=(const GpioConform_t&) = delete;
};

class Gpio_t {
public:
    explicit Gpio_t(bool& status);
    ~Gpio_t();

    int  digitalWrite(uint16_t pin, uint8_t value);
    int  pinMode(uint16_t pin, uint8_t mode);
    int  digitalRead(int pin);
    void CloseGpios();

    int  getNextId();
    void printGpios() const;
    void addGpio(uint16_t gpio_pin, std::string dir, std::string edge, std::string value);

private:
    bool m_state;
    std::unordered_map<int, GpioConform_t*> gpioById;
    std::unordered_map<uint16_t, GpioConform_t*> gpioByPin;
    std::vector<std::unique_ptr<GpioConform_t>> m_gpio_cfg;
    void updateGpioMaps();
};

} // namespace GPIO
