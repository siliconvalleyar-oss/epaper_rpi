#include "EPD_Driver.h"
#include <unistd.h>

static void delay_ms(uint32_t ms) {
    usleep(ms * 1000);
}

EPD_Driver::EPD_Driver(eScreen_EPD_t eScreen_EPD, pins_t board)
{
	spi_basic = board;

	pdi_cp = (uint16_t) eScreen_EPD;
	pdi_size = (uint16_t) (eScreen_EPD >> 8);

	uint16_t _screenSizeV = 0;
	uint16_t _screenSizeH = 0;

	switch (pdi_size)
	{
		case 0x15: _screenSizeV = 152; _screenSizeH = 152; break;
		case 0x21: _screenSizeV = 212; _screenSizeH = 104; break;
		case 0x26: _screenSizeV = 296; _screenSizeH = 152; break;
		case 0x27: _screenSizeV = 264; _screenSizeH = 176; break;
		case 0x28: _screenSizeV = 296; _screenSizeH = 128; break;
		case 0x37: _screenSizeV = 416; _screenSizeH = 240; break;
		case 0x41: _screenSizeV = 300; _screenSizeH = 400; break;
		case 0x43: _screenSizeV = 480; _screenSizeH = 176; break;
		default: break;
	}

	image_data_size = (uint32_t) _screenSizeV * (uint32_t) (_screenSizeH / 8);

	memcpy(register_data, register_data_sm, sizeof(register_data_sm));
}

void EPD_Driver::COG_initial()
{
	bcm2835_gpio_fsel(spi_basic.panelBusy, BCM2835_GPIO_FSEL_INPT);
	bcm2835_gpio_fsel(spi_basic.panelDC, BCM2835_GPIO_FSEL_OUTP);
	bcm2835_gpio_write(spi_basic.panelDC, HIGH);

	bcm2835_gpio_fsel(spi_basic.panelReset, BCM2835_GPIO_FSEL_OUTP);
	bcm2835_gpio_write(spi_basic.panelReset, HIGH);

	bcm2835_gpio_fsel(spi_basic.panelCS, BCM2835_GPIO_FSEL_OUTP);
	bcm2835_gpio_write(spi_basic.panelCS, HIGH);

	if (spi_basic.panelON_EXT2 != 0xff)
	{
		bcm2835_gpio_fsel(spi_basic.panelON_EXT2, BCM2835_GPIO_FSEL_OUTP);
		bcm2835_gpio_write(spi_basic.panelON_EXT2, HIGH);
	}

	if (spi_basic.panelSPI43_EXT2 != 0xff)
	{
		bcm2835_gpio_fsel(spi_basic.panelSPI43_EXT2, BCM2835_GPIO_FSEL_OUTP);
		bcm2835_gpio_write(spi_basic.panelSPI43_EXT2, LOW);
	}

	delay_ms(5);

	reset(5, 5, 10, 5, 5);

	softReset();

	sendIndexData( 0xe5, &register_data[2], 1 );
	sendIndexData( 0xe0, &register_data[3], 1 );
	sendIndexData( 0x00, &register_data[4], 2 );
}

void EPD_Driver::COG_powerOff()
{
	sendIndexData( 0x02, &register_data[0], 0 );

	while ( bcm2835_gpio_lev(spi_basic.panelBusy) != HIGH );

	bcm2835_gpio_write(spi_basic.panelDC, LOW);
	bcm2835_gpio_write(spi_basic.panelCS, LOW);

	delay_ms(150);

	bcm2835_gpio_write(spi_basic.panelReset, LOW);
}

void EPD_Driver::globalUpdate(const uint8_t * data1s, const uint8_t * data2s)
{
	sendIndexData(0x10, data1s, image_data_size);

	static const uint8_t zeros[5624] = {0};
	(void)data2s;
	sendIndexData(0x13, zeros, image_data_size);

	DCDC_powerOn();
	displayRefresh();
}

void EPD_Driver::sendIndexData( uint8_t index, const uint8_t *data, uint32_t len )
{
	bcm2835_gpio_write(spi_basic.panelDC, LOW);
	bcm2835_gpio_write(spi_basic.panelCS, LOW);

	bcm2835_spi_transfer(index);

	bcm2835_gpio_write(spi_basic.panelCS, HIGH);

	for ( uint32_t i = 0; i < len; i++ )
	{
		bcm2835_gpio_write(spi_basic.panelDC, HIGH);
		bcm2835_gpio_write(spi_basic.panelCS, LOW);

		bcm2835_spi_transfer(data[i]);

		bcm2835_gpio_write(spi_basic.panelCS, HIGH);
	}
}

void EPD_Driver::softReset()
{
	sendIndexData( 0x00, &register_data[1], 1 );
	while( bcm2835_gpio_lev(spi_basic.panelBusy) != HIGH );
}

void EPD_Driver::displayRefresh()
{
	sendIndexData( 0x12, &register_data[0], 0 );
	while( bcm2835_gpio_lev(spi_basic.panelBusy) != HIGH );
}

void EPD_Driver::reset(uint32_t ms1, uint32_t ms2, uint32_t ms3, uint32_t ms4, uint32_t ms5)
{
	delay_ms(ms1);
	bcm2835_gpio_write(spi_basic.panelReset, HIGH);
	delay_ms(ms2);
	bcm2835_gpio_write(spi_basic.panelReset, LOW);
	delay_ms(ms3);
	bcm2835_gpio_write(spi_basic.panelReset, HIGH);
	delay_ms(ms4);
	bcm2835_gpio_write(spi_basic.panelCS, HIGH);
	delay_ms(ms5);
}

void EPD_Driver::DCDC_powerOn()
{
	uint8_t dummy = 0x00;
	sendIndexData( 0x04, &dummy, 0 );
	while( bcm2835_gpio_lev(spi_basic.panelBusy) != HIGH );
}
