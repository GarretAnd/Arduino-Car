
#include <avr/io.h>
#include "USARTE85.h"   			// UART initializations
#include "ioE85.h"    
#include <avr/delay.h>			// tiny_printf as well as some reading
#include "i2c.h"
#include "motorlib.h"




void initShield(void){		// Initiates the shield 
	
	#define PCA9685_ADDR 0x60 //I2c bus address (shift left 1 bit before sending1)
	#define TWSR_MASK 0b11111100 //Mask for status register (mask out prescaler bits)
	#define ADDRESS_ACK 0x18 //status if code was recuived after SLA+W transmit
	#define   MODE1          0x0
	
	USART_Init();
	printf("Adafruit Motorsheild vs2 test \r\n");
	initI2C();
	i2cStart();
	i2cSend(PCA9685_ADDR << 1);//what
	i2cSend(MODE1);
	i2cSend(0b00100001);     // enable auto increment and oscillatori2cStop(); _delay_us(500);
	_delay_us(500);
	i2cStop();
}


void motorRun( uint16_t speed, uint16_t motor, uint8_t cmd){  // Sets speed of motor plus direction via pwm 
	
	
	i2cStart();
	i2cSend(PCA9685_ADDR << 1);//what does this do
	i2cSend(motor);//pwm8 led 8	 //motor 1
	i2cSend(0b00000000);
	i2cSend(0b00000000);
	i2cSend((uint8_t)speed);
	i2cSend((uint8_t)(speed>>8));
	
	motordirection(cmd);

}

void motordirection(uint8_t cmd){
	
	
	switch(cmd) {
		case 0: //clockwise
			
			i2cSend(0b00000000);
			i2cSend(0b00010000);
			i2cSend(0b00000000);
			i2cSend(0b00000000);
			
			i2cSend(0b00000000); 
			i2cSend(0b00000000);
			i2cSend(0b00000000);
			i2cSend(0b00010000);
		break;
		case 1:
			i2cSend(0b00000000);//coutnerclockwise
			i2cSend(0b00000000);
			i2cSend(0b00000000);
			i2cSend(0b00010000);
			
			i2cSend(0b00000000);
			i2cSend(0b00010000);
			i2cSend(0b00000000);
			i2cSend(0b00000000);
		break;
		case 2:
			i2cSend(0b00000000);//stop
			i2cSend(0b00010000);
			i2cSend(0b00000000);
			i2cSend(0b00000000);
			
			i2cSend(0b00000000); 
			i2cSend(0b00010000);
			i2cSend(0b00000000);
			i2cSend(0b00000000);
		break;
	}
	i2cStop();
}

