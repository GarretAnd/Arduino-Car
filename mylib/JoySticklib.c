#include <avr/io.h>			// All the port definitions are here
#include <util/delay.h> 	// Enables delay statements 
#include "USARTE85.h"   	// UART initializations
#include "ioE85.h"      	// tiny_printf as well as some reading
#include <avr/interrupt.h>  // Includes Interrupts 
#include "i2c.h"			//Include I2C Functions

uint16_t * joystickValues(uint16_t final[]);
void ADC_Init (void);
uint16_t gitADCValueC(void);

uint16_t * joystickValues(uint16_t final[]) {		// Gets values from joystick 
	
	uint16_t Jswag = 0;
	uint16_t Rswag = 0;
	
	int counter = 0;
	
	Jswag = gitADCValueC();		// Gets various values from joystick 
	Rswag = gitADCValueC();
	
	final[0] = Jswag;			// Puts these values into array 
	final[1] = Rswag;
}

void ADC_Init (void) {
	ADMUX |= ((1<<REFS0));									//Gets Reference Voltage to Vcc (5V)
	ADCSRA |= ((1<<ADEN)|(1<<ADPS0)|(1<<ADPS1)|(1<<ADPS2));	// ADEN Enables ADC Converstion and next 3 slows down clock (sets prescalers)
}

uint16_t gitADCValueC(void) {								
	uint16_t yefri;
	ADCSRA |= (1<<ADSC);
	while((ADCSRA & (1<<ADSC)) != 0) {	} 					//Waits for conversion to finish (is 1 while waiting to finish)
	yefri = ADC;
	ADMUX ^= ((1 << MUX0));					//Switches from temp to light every other time (starts at light)(A0 to A1)
	_delay_ms(10);
	//Toggles the voltage reference to 1.1V for light in second statement
	return(yefri);											//We all love returning Yefri
}
