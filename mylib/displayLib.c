#include "i2c.h"	

#define HT16K33_ADDR 0x71

void counterDisplay(int16_t speed);

void counterDisplay(int16_t speed) {
	
	int16_t first_digit = 0;		// Creates Values to parse max size variable 
	int16_t second_digit = 0;
	int16_t third_digit = 0;
	int16_t fourth_digit = 0;
	
	uint16_t numberTable[] = {		// Converts value into counter code 
		0x3F, /* 0 */
		0x06, /* 1 */
		0x5B, /* 2 */
		0x4F, /* 3 */
		0x66, /* 4 */
		0x6D, /* 5 */
		0x7D, /* 6 */
		0x07, /* 7 */
		0x7F, /* 8 */
		0x6F, /* 9 */
		0x77, /* a */
		0x7C, /* b */
		0x39, /* C */
		0x5E, /* d */
		0x79, /* E *
		0x71, /* F */
	};
	
	
	i2cStart();					// Starts sending bits to the display 
	i2cSend(HT16K33_ADDR <<1);		
	i2cSend(0x21);
	i2cStop();
	
	i2cStart();
	i2cSend(HT16K33_ADDR <<1);		
	i2cSend(0x81);					//Adjusts Blinking
	i2cStop();

	i2cStart();
	i2cSend(HT16K33_ADDR <<1);
	i2cSend(0xEF);					//Adjusts Dimming
	i2cStop();
	
	i2cStart();
	i2cSend(HT16K33_ADDR <<1);
	i2cSend(0x00);

	
	first_digit = speed/1000;		// Converts values to get digit for that place 
	second_digit = speed/100;
	third_digit = speed/10;
	fourth_digit = speed%10;
	
	first_digit = numberTable[first_digit];		// Sends digit to number table and converts it 
	second_digit = numberTable[second_digit];
	third_digit = numberTable[third_digit];
	fourth_digit = numberTable[fourth_digit];
	
	i2cSend(first_digit);					// Displays these values as needed 
	i2cSend(0b00000000);
	
	i2cSend(second_digit);
	i2cSend(0b00000000);
	
	i2cSend(0b00000000);			//This line controls the Colon
	i2cSend(0b00000000);
	
	i2cSend(third_digit);
	i2cSend(0b00000000);
	
	i2cSend(fourth_digit);
	i2cSend(0b00000000);
	
	i2cStop();
}