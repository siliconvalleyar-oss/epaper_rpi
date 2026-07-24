
//////////////////////////////////////////////////////////////////////////////
//     
//          filename            :   epaper.cpp
//          License             :   GNU 
//          Author              :   Lio
//          Change History      :
//          Processor           :   ARM
//          Hardware            :		
//          Complier            :   ARM
//          Company             :
//          Dependencies        :
//          Description         :
//          brief               :	
//
//////////////////////////////////////////////////////////////////////////////

#include <string>
#include <cstring>
#include <tyme/tyme.h>
#include <epaper/epaper.h>
#include <epaper/boards.h>


//#define DEBUG


namespace EPAPER{


EPD_Driver::EPD_Driver(uint32_t eScreen_EPD, const pins_t board)
    : Gpio_t(true)
    , spi_ptr(std::make_unique<SPI::Spi_t>())
    , pin_cfg_epaper(board)
{
    pdi_cp = (uint16_t)eScreen_EPD;
    pdi_size = (uint16_t)(eScreen_EPD >> 8);

    uint16_t screenSizeV = 0;
    uint16_t screenSizeH = 0;

    switch (pdi_size) {
        case 0x15: // 1.54"
            screenSizeV = 152;
            screenSizeH = 152;
            break;
        case 0x21: // 2.13"
            screenSizeV = 212;
            screenSizeH = 104;
            break;
        case 0x26: // 2.66"
            screenSizeV = 296;
            screenSizeH = 152;
            break;
        case 0x27: // 2.71"
            screenSizeV = 264;
            screenSizeH = 176;
            break;
        case 0x28: // 2.87"
            screenSizeV = 296;
            screenSizeH = 128;
            break;
        case 0x37: // 3.70"
            screenSizeV = 416;
            screenSizeH = 240;
            break;
        case 0x41: // 4.17"
            screenSizeV = 300;
            screenSizeH = 400;
            break;
        case 0x43: // 4.37"
            screenSizeV = 480;
            screenSizeH = 176;
            break;
        default:
            break;
    }

    image_data_size = (uint32_t)screenSizeV * (uint32_t)(screenSizeH / 8);
    memcpy(register_data, register_data_sm, sizeof(register_data_sm));
}



int EPD_Driver::digitalRead(int gpio) {
    return gpio_get_fd_to_value(gpio);
}





void EPD_Driver::COG_initial() {
    // Configurar pines GPIO
    pinMode(pin_cfg_epaper.panelBusy, INPUT);
    pinMode(pin_cfg_epaper.panelDC, OUTPUT);
    digitalWrite(pin_cfg_epaper.panelDC, HIGH);

    pinMode(pin_cfg_epaper.panelReset, OUTPUT);
    digitalWrite(pin_cfg_epaper.panelReset, HIGH);

    pinMode(pin_cfg_epaper.panelCS, OUTPUT);
    digitalWrite(pin_cfg_epaper.panelCS, HIGH);

    if (pin_cfg_epaper.panelON_EXT2 != NOT_CONNECTED) {
        pinMode(pin_cfg_epaper.panelON_EXT2, OUTPUT);
        digitalWrite(pin_cfg_epaper.panelON_EXT2, HIGH);
    }
    if (pin_cfg_epaper.panelSPI43_EXT2 != NOT_CONNECTED) {
        pinMode(pin_cfg_epaper.panelSPI43_EXT2, OUTPUT);
        digitalWrite(pin_cfg_epaper.panelSPI43_EXT2, LOW);
    }

    TYME::delay_ms(5);

    // Secuencia de reset
    reset(5, 5, 10, 5, 5);

    // Soft reset
    softReset();

    // Configuración de temperatura y PSR
    sendIndexData(0xe5, &register_data[2], 1);  // Input Temperature: 25°C
    sendIndexData(0xe0, &register_data[3], 1);  // Active Temperature
    sendIndexData(0x00, &register_data[4], 2);  // PSR
}


void EPD_Driver::sendIndexData(uint8_t index, const uint8_t *data, uint32_t len) {
    // Enviar comando: DC=LOW, CS=LOW → transferir byte → CS=HIGH
    digitalWrite(pin_cfg_epaper.panelDC, LOW);
    digitalWrite(pin_cfg_epaper.panelCS, LOW);
    hV_HAL_SPI_transfer(index);
    digitalWrite(pin_cfg_epaper.panelCS, HIGH);

    // Enviar datos: DC=HIGH, CS=LOW → transferir byte → CS=HIGH entre cada uno
    for (uint32_t i = 0; i < len; i++) {
        digitalWrite(pin_cfg_epaper.panelDC, HIGH);
        digitalWrite(pin_cfg_epaper.panelCS, LOW);
        hV_HAL_SPI_transfer(data[i]);
        digitalWrite(pin_cfg_epaper.panelCS, HIGH);
    }
}



void EPD_Driver::softReset() {
    sendIndexData(0x00, &register_data[1], 1);  // Soft-reset
    uint32_t timeout = 5000;
    while (digitalRead(pin_cfg_epaper.panelBusy) != 1) {
        if (--timeout == 0) {
            std::cerr << "Timeout en softReset" << std::endl;
            break;
        }
        TYME::delay_ms(1);
    }
}


    void EPD_Driver::reset(uint32_t ms1, uint32_t ms2, uint32_t ms3, uint32_t ms4, uint32_t ms5)
    {
    	// note: group delays into one array
    	TYME::delay_ms(ms1);    /// 5 msec 
        digitalWrite(pin_cfg_epaper.panelReset, HIGH); // RES# = 1
        TYME::delay_ms(ms2);    /// 5 msec 
        digitalWrite(pin_cfg_epaper.panelReset, LOW);
        TYME::delay_ms(ms3);    /// 10 msec 
        digitalWrite(pin_cfg_epaper.panelReset, HIGH);
        // comando reset
        TYME::delay_ms(ms4);    /// 5 msec 
        digitalWrite(pin_cfg_epaper.panelCS, HIGH); // CS# = 1
        TYME::delay_ms(ms5);    /// 5 msec 
    }


void EPD_Driver::DCDC_powerOn() {
    uint8_t dummy = 0;
    sendIndexData(0x04, &dummy, 0);  // Power on — solo comando
    uint32_t timeout = 10000;
    while (digitalRead(pin_cfg_epaper.panelBusy) != 1) {
        if (--timeout == 0) {
            std::cerr << "Timeout en DCDC_powerOn" << std::endl;
            break;
        }
        TYME::delay_ms(1);
    }
}


void EPD_Driver::displayRefresh() {
    uint8_t dummy = 0;
    sendIndexData(0x12, &dummy, 0);  // Display Refresh — solo comando
    uint32_t timeout = 60000;
    while (digitalRead(pin_cfg_epaper.panelBusy) != 1) {
        if (--timeout == 0) {
            std::cerr << "Timeout en displayRefresh" << std::endl;
            break;
        }
        TYME::delay_ms(1);
    }
}

void EPD_Driver::globalUpdate(const uint8_t *data1s, const uint8_t *data2s) {
    // Enviar primer frame (canal negro)
    sendIndexData(0x10, data1s, image_data_size);

    // Enviar segundo frame (canal rojo)
    sendIndexData(0x13, data2s, image_data_size);

    // Encender DC/DC y refrescar
    DCDC_powerOn();
    displayRefresh();
}


    // CoG shutdown function
    //		Shuts down the CoG and DC/DC circuit after all update functions
    //		- INPUT:
    //			- none but requires global variables on SPI pinout and config register data
void EPD_Driver::COG_powerOff() {
    sendIndexData(0x02, &register_data[0], 0);  // Turn off DC/DC

    uint32_t timeout = 5000;
    while (digitalRead(pin_cfg_epaper.panelBusy) != 1) {
        if (--timeout == 0) {
            std::cerr << "Timeout en COG_powerOff" << std::endl;
            break;
        }
        TYME::delay_ms(1);
    }

    digitalWrite(pin_cfg_epaper.panelDC, LOW);
    digitalWrite(pin_cfg_epaper.panelCS, LOW);
    // NOTA: BUSY es entrada — NO se debe escribir
    TYME::delay_ms(150);
    digitalWrite(pin_cfg_epaper.panelReset, LOW);
}

uint8_t EPD_Driver::hV_HAL_SPI_transfer(uint8_t command) {
    return spi_ptr->Transfer1bytes(command);
}

}