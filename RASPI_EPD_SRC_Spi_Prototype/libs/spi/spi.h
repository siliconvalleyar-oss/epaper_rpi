#pragma once

#include <cstdint>
#include <memory>
#include <linux/spi/spidev.h>

// SPI through /dev/spidev0.0 using ioctl
// NOTE: This is the legacy implementation. Newer versions use bcm2835.

#define SPI_SPEED   400000  // 400 KHz
#define LARGE_SECTOR_SIZE   256

namespace SPI{
  
  struct Spi_t{
    explicit  Spi_t();
    ~Spi_t();

    void init();
    void settings_spi();
    void spi_close();
    const uint8_t Transfer1bytes(const uint8_t cmd);
    const uint8_t Transfer2bytes(const uint16_t address);
    const uint8_t Transfer3bytes(const uint32_t address);
    void printDBGSpi();
    void msj_fail();  
    uint32_t get_spi_speed();
  private:
    uint8_t m_tx_buffer[LARGE_SECTOR_SIZE]{};
    uint8_t m_rx_buffer[LARGE_SECTOR_SIZE]{};
    const uint32_t m_spi_speed{SPI_SPEED};
    int fs{-1};
    int ret{0};
    uint8_t looper{0};
    uint32_t scratch32{0};
    std::unique_ptr<struct spi_ioc_transfer> spi{nullptr};
  };

}//END namespace SPI_H
