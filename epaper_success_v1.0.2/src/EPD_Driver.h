#ifndef EPD_DRIVER_H
#define EPD_DRIVER_H

#include <cstdint>
#include <cstring>
#include <bcm2835.h>
#include "EPD_Configuration.h"

#ifndef HIGH
#define HIGH 0x1
#endif
#ifndef LOW
#define LOW 0x0
#endif

class EPD_Driver
{
  public:
	EPD_Driver(eScreen_EPD_t eScreen_EPD, pins_t board);

	void COG_initial();
	void COG_powerOff();
	void globalUpdate(const uint8_t *data1s, const uint8_t *data2s);

  protected:
	void sendIndexData(uint8_t index, const uint8_t *data, uint32_t len);
	void softReset();
	void displayRefresh();
	void reset(uint32_t ms1, uint32_t ms2, uint32_t ms3, uint32_t ms4, uint32_t ms5);
	void DCDC_powerOn();

	pins_t spi_basic;
	uint16_t pdi_size;
	uint16_t pdi_cp;
	uint32_t image_data_size;
	uint8_t register_data[6] = {};
};

#endif
