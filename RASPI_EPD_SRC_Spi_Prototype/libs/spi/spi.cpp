#include <spi/spi.h>
#include <cstring>
#include <iostream>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/spi/spidev.h>

#define SPI_DEVICE "/dev/spidev0.0"

namespace SPI {

void Spi_t::msj_fail(){
    std::cerr << "Could not open SPI device" << std::endl;
}

void Spi_t::settings_spi(){
    spi->tx_buf = (unsigned long)m_tx_buffer;
    spi->rx_buf = (unsigned long)m_rx_buffer;
    spi->bits_per_word = 0;
    spi->speed_hz = m_spi_speed;
    spi->delay_usecs = 1;
    spi->len = 3;
}

void Spi_t::init(){
    fs = open(SPI_DEVICE, O_RDWR);
    if(fs < 0) {
        msj_fail();
        return;
    }
    ret = ioctl(fs, SPI_IOC_RD_MODE, &scratch32);
    if(ret != 0) { close(fs); fs = -1; return; }
    scratch32 |= SPI_MODE_0;
    ret = ioctl(fs, SPI_IOC_WR_MODE, &scratch32);
    if(ret != 0) { close(fs); fs = -1; return; }
    scratch32 = m_spi_speed;
    ret = ioctl(fs, SPI_IOC_WR_MAX_SPEED_HZ, &scratch32);
    if(ret != 0) { close(fs); fs = -1; return; }
}

const uint8_t Spi_t::Transfer1bytes(const uint8_t cmd){
    if (fs < 0) return 0xFF;
    std::memset(m_rx_buffer, 0xFF, LARGE_SECTOR_SIZE);
    std::memset(m_tx_buffer, 0xFF, LARGE_SECTOR_SIZE);
    std::memset(spi.get(), 0, sizeof(struct spi_ioc_transfer));
    spi->len = 1;
    m_tx_buffer[0] = cmd;
    spi->tx_buf = reinterpret_cast<unsigned long>(m_tx_buffer);
    spi->rx_buf = reinterpret_cast<unsigned long>(m_rx_buffer);
    spi->speed_hz = get_spi_speed();
    spi->bits_per_word = 8;
    spi->cs_change = 0;
    spi->delay_usecs = 0;
    int ret = ioctl(fs, SPI_IOC_MESSAGE(1), spi.get());
    if (ret < 0) return 0xFF;
    return m_rx_buffer[0];
}

const uint8_t Spi_t::Transfer2bytes(const uint16_t cmd){
    if (fs < 0) return 0xFF;
    spi->len = sizeof(cmd);
    m_rx_buffer[0] = m_rx_buffer[1] = 0xFF;
    memcpy(m_tx_buffer, &cmd, sizeof(cmd));
    ioctl(fs, SPI_IOC_MESSAGE(1), spi.get());
    return m_rx_buffer[1];
}

const uint8_t Spi_t::Transfer3bytes(const uint32_t cmd){
    if (fs < 0) return 0xFF;
    spi->len = 3;
    m_rx_buffer[0] = m_rx_buffer[1] = m_rx_buffer[2] = 0xFF;
    memcpy(m_tx_buffer, &cmd, 3);
    ioctl(fs, SPI_IOC_MESSAGE(1), spi.get());
    return m_rx_buffer[2];
}

void Spi_t::spi_close(){
    if(fs >= 0) close(fs);
    fs = -1;
}

Spi_t::Spi_t()
    : m_tx_buffer{}
    , m_rx_buffer{}
    , m_spi_speed{SPI_SPEED}
    , spi{std::make_unique<struct spi_ioc_transfer>()} 
{
    settings_spi();
    init();
}

Spi_t::~Spi_t(){
    spi_close();
    // NOTA: No llamar a exit() aqui - el destructor debe ser seguro
}

uint32_t Spi_t::get_spi_speed(){
    return m_spi_speed;
}

}
