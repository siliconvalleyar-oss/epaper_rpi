//////////////////////////////////////////////////////////////////////////////
//     
//          filename            :   gpio.h
//          License             :   GNU 
//          Author              :   Lio
//          Description         :   GPIO wrapper con bcm2835
//
//////////////////////////////////////////////////////////////////////////////

#pragma once
#include <iostream>
#include <string>
#include <string_view>
#include <memory>
#include <vector>
#include <unordered_map>
#include <cstdint>   // para uint16_t

//#define DBG_GPIO

// O mejor aún como constantes:
namespace GPIO {
    constexpr const char* SYSFS_GPIO_PATH  = "/sys/class/gpio";
    constexpr const char* SYSFS_GPIO_VALUE = "/value";

// Direcciones
constexpr std::string_view DIR_IN  = "in";
constexpr std::string_view DIR_OUT = "out";

// Valores
constexpr std::string_view VALUE_HIGH = "1";
constexpr std::string_view VALUE_LOW  = "0";

struct GpioConform_t {
    int                 ID;
    uint16_t            gpio;
    std::string         dir;
    std::string         edge;   // reservado para compatibilidad
    std::string         value;
    bool                status;

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

    int digitalWrite(uint16_t pin, std::string_view st);
    int pinMode(uint16_t number_gpio, std::string_view direction);
    int digitalRead(int gpio);

    int getNextId();
    void printGpios() const;
    void addGpio(uint16_t gpio_pin, std::string dir, std::string edge, std::string value);
    void CloseGpios();
    int gpio_get_fd_to_value(int gpio_num) ;

private:
    bool m_state;

    std::unordered_map<int, GpioConform_t*> gpioById;
    std::unordered_map<uint16_t, GpioConform_t*> gpioByPin;
    std::vector<std::unique_ptr<GpioConform_t>> m_gpio_cfg;

    void updateGpioMaps();
};

} // namespace GPIO
