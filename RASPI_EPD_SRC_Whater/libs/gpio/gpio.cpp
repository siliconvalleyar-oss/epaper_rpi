//////////////////////////////////////////////////////////////////////////////
//     
//          filename            :   gpio_bcm2835.cpp
//          License             :   GNU 
//          Author              :   Lio
//          Description         :   Versión usando librería bcm2835
//
//////////////////////////////////////////////////////////////////////////////

#include <gpio/gpio.h>
#include <bcm2835.h>
#include <iostream>
#include <memory>
#include <vector>
#include <string>
#include <fcntl.h>    // O_RDONLY, O_NONBLOCK, open
#include <unistd.h>   // close, read, write
#include <sys/types.h>
#include <sys/stat.h>
#include <cstdio>





namespace GPIO {

Gpio_t::Gpio_t(bool& status)
    : m_state(status) 
{
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

int Gpio_t::digitalWrite(uint16_t pin, std::string_view st) {
    if (!m_state) return -1;
    if (st == VALUE_HIGH) {
        bcm2835_gpio_write(pin, HIGH);
    } else {
        bcm2835_gpio_write(pin, LOW);
    }
    return 0;
}

int Gpio_t::pinMode(uint16_t pin, std::string_view dir) {
    if (!m_state) return -1;

    if (dir == DIR_OUT) {
        bcm2835_gpio_fsel(pin, BCM2835_GPIO_FSEL_OUTP);
    } else if (dir == DIR_IN) {
        bcm2835_gpio_fsel(pin, BCM2835_GPIO_FSEL_INPT);
    } else {
        std::cerr << "Dirección GPIO inválida: " << dir << std::endl;
        return -1;
    }
    return 0;
}

int Gpio_t::digitalRead(int pin) {
    if (!m_state) return -1;
    uint8_t val = bcm2835_gpio_lev(pin);
    return val ? 1 : 0;
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

    // Configurar GPIO con bcm2835
    pinMode(gpio_pin, gpioByPin[gpio_pin]->dir);
    digitalWrite(gpio_pin, gpioByPin[gpio_pin]->value);
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
        gpioById[gpioPtr->ID]   = gpioPtr.get();
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


/*
int Gpio_t::gpio_get_fd_to_value(int gpio_num) {
    std::string path = std::string(SYSFS_GPIO_PATH) + "/gpio" + std::to_string(gpio_num) + std::string(SYSFS_GPIO_VALUE);
    int fd = open(path.c_str(), O_RDONLY | O_NONBLOCK);
    if (fd < 0) {
        perror("gpio/get-value");
        return -1;
    }
    return fd;
}
*/

/*
int Gpio_t::gpio_get_fd_to_value(int gpio_num) {
    std::string path = std::string(SYSFS_GPIO_PATH) 
                     + "/gpio" + std::to_string(gpio_num) 
                     + std::string(SYSFS_GPIO_VALUE);

    int fd = open(path.c_str(), O_RDONLY | O_NONBLOCK);
    if (fd < 0) {
        perror("gpio/get-value");
        return -1;
    }
    return fd;
}


*/



int Gpio_t::gpio_get_fd_to_value(int gpio_num) {
    std::string path = std::string(SYSFS_GPIO_PATH)
                     + "/gpio" + std::to_string(gpio_num)
                     + std::string(SYSFS_GPIO_VALUE);

    int fd = open(path.c_str(), O_RDONLY | O_NONBLOCK);
    if (fd < 0) {
        perror("gpio/get-value");
        return -1;
    }

    char buf[2] = {0};               // buffer para leer "0\n" o "1\n"
    ssize_t n = read(fd, buf, 1);    // leer un solo carácter
    close(fd);                        // cerrar descriptor inmediatamente

    if (n <= 0) {
        perror("gpio/read-value");
        return -1;
    }

    return (buf[0] == '0') ? 0 : 1;  // convertir a valor digital
}








} // namespace GPIO
