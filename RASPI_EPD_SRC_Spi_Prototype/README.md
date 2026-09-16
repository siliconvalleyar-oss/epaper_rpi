
ANNOTATE EPAPER:
raspberry pi 2w & pi 4

/******************************************************************
WIRE_GRAY_PIN    <->	Panel_CS    <->		PIN13 - gpio27
WIRE_BLUE_PIN    <->	MOSI		<->    	PIN19 - gpio10
WIRE_BROWN_PIN   <->	SCK    		<->    	PIN23 - gpio11
WIRE_GREEN_PIN   <->	MISO    	<->     PIN21 - gpio9

WIRE_VIOLET_PIN   <->	Flash_CS    <->     PIN15 - gpio22
WIRE_YELLOW_PIN   <->	RESET    	<->     PIN22 - gpio25
WIRE_ORANGE_PIN   <->	D/C    		<->    	PIN24 - gpio8 -ce0
WIRE_RED_PIN 	  <->	BUSY 		<->	    PIN26 - gpio7 -ce1

pixels 266 = 152 pix x 296 pix
total bytes = 5624 bytes
******************************************************************/

others
/******************************************************************
WIRE_GRAY_PIN    <->	Panel_CS    <->	PIN13 - gpio2
WIRE_BLUE_PIN    <->	MOSI		<->    	PIN19 - gpio12
WIRE_BROWN_PIN   <->	SCK    		<->    	PIN23 - gpio14
WIRE_GREEN_PIN   <->	MISO    	<->     PIN21 - gpio13

WIRE_VIOLET_PIN   <->	Flash_CS    <->     PIN15 - gpio3
WIRE_YELLOW_PIN   <->	RESET    	<->     PIN22 - gpio6
WIRE_ORANGE_PIN   <->	D/C    		<->    	PIN24 - gpio10 -ce0
WIRE_RED_PIN 	  <->	BUSY 		<->	    PIN26 - gpio11 -ce1

pixels 266 = 152 pix x 296 pix
total bytes = 5624 bytes
******************************************************************/
corregido 


#define DC pin
#define CS pin


SPITimingFormat(){
uint8_t index=0x09;
uint8_t buffer[]={0x1,0x3c,0x1f,0x5c,0xa3,0xe9,0xbc,0xeb,0xda,0xf1};

DC=0;
CS=0;
spi_send_one_byte(index);
CS=1
DC=1

for(;;){
	delay_ms(1);
	CS=0;
	value_return=spi_send_buffer(buffer);
	CS=1;

	if(value_return==0)exit;
	}
}




#power

powerOnCOGdriver(){
delay_ms(5);//1 ms , diferential original
RES=1;
delay_ms(5); 
RES=0;
delay_ms(10); 
RES=1;
delay_ms(5); 

spi_send(0x00 , 0x0e);
CS, HIGH;
delay_ms(5); //1 ms , diferential original
#end
return ;
}



SetEnvironmentTemperatureAndPSR(){

uint8_t get_data_TSSET;
uint8_t cmd=0xe5;
uint8_t get_PSR;

spi_get(cmd, (*) get_data_TSSET);

spi_send(0xe0,0x02);


	if(EPD_zise > 2.9"){
		exit;
	}

spi_get(0x00,PSR);


}

EPD_266 = 152x296;
white=0;
black=1;


inputImageToTheEPD(cmd , buffer){
DC=0;
CS=0;
spi_send(cmd);
DC=1;
for(;;){
	CS=1;
	delay_ms(1);
	CS=0;
	spi_send_buff(buffer++);

}
CS=1;
delay_ms(1);
CS=0;
DC=0;
}



SendUpdatingCommand(*busy){

// Power on command
spi(0x4);//*2

	while(BUSY);//BUSY=HIGH?

// Display Refresh
spi(0x12);//*2

	while(BUSY);//BUSY=HIGH?

}//end



turnOffDcDc(){

spi(0x2);//*2
 while(BUSY);//BUSY=HIGH?
DC=LOW
CS=0;
BUSY LOW;
delay_ms( 150 );
RES, LOW;
MOSI=0;
SCK=0;

//of /Cut off the vcc vdd of COG
}


void EPD_Driver::COG_initial(){

 BUSY = INPUT ;
DC= OUTPUT ;
DC= HIGH;
RES=OUTPUT 
RES=HIGH
CS= OUTPUT;
CS=HIGH;

}



bee@macbook epaper_me_code % 
