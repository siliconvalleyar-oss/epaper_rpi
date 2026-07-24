#include <iostream>
#include <cstring>
#include <unistd.h>
#include <bcm2835.h>
#include "src/EPD_Driver.h"

int main()
{
	if (!bcm2835_init())
	{
		std::cerr << "ERROR: bcm2835_init failed" << std::endl;
		return 1;
	}

	bcm2835_spi_begin();
	bcm2835_spi_setBitOrder(BCM2835_SPI_BIT_ORDER_MSBFIRST);
	bcm2835_spi_setDataMode(BCM2835_SPI_MODE0);
	bcm2835_spi_setClockDivider(BCM2835_SPI_CLOCK_DIVIDER_256);

	std::cout << "EPD v1.4.0 - Restored from working 1.0.1 driver" << std::endl;
	std::cout << "SPI clock divider: 256 (~976 KHz)" << std::endl;

	EPD_Driver driver(eScreen_EPD_266, boardRaspberryPiZero2W_EXT3);

	std::cout << "COG init..." << std::endl;
	driver.COG_initial();
	std::cout << "  COG ready" << std::endl;

	uint8_t *black = new uint8_t[5624];
	uint8_t *white = new uint8_t[5624];
	memset(black, 0x00, 5624);
	memset(white, 0xff, 5624);

	std::cout << "Sending BLACK image..." << std::endl;
	driver.globalUpdate(black, NULL);
	std::cout << "  Done" << std::endl;

	usleep(3000000);

	std::cout << "Sending WHITE image..." << std::endl;
	driver.globalUpdate(white, NULL);
	std::cout << "  Done" << std::endl;

	usleep(3000000);

	std::cout << "Power off..." << std::endl;
	driver.COG_powerOff();

	delete[] black;
	delete[] white;

	bcm2835_close();

	std::cout << "Complete" << std::endl;
	return 0;
}
