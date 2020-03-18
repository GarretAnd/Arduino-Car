#include <avr/io.h>
#include "ioE85.h"      	// tiny_printf as well as some reading

#define SERVO_MIN 24     // Sets min and max range of servo 
#define SERVO_MAX 34       
#define SERVO_MID 29      

void initTimer0Servo(void); 	
void setPulseLength(uint16_t pulse);
void steer(uint16_t degree);

void steer(uint16_t degree) {		// Takes value in as degree 

	uint16_t finalValue = 0;

	finalValue = (((degree*(SERVO_MAX-SERVO_MIN))/256) + SERVO_MIN);  // Converts value to a percentage of the range 	
	
	if ((finalValue >= SERVO_MIN)  && (finalValue <= SERVO_MAX)) {
		setPulseLength(finalValue);
	}
	else {									// if greater than or less than max resets it otherwise lets it pass
		if (finalValue < SERVO_MIN) {
			setPulseLength(SERVO_MIN);
			finalValue = SERVO_MIN;
		}
		else {
			setPulseLength(SERVO_MAX);
			finalValue = SERVO_MAX;
		}
	}
	
}

void initTimer0Servo(void) {   // Initializes timer for servo 
   
   ICR1 = 40000;
   TCCR0A |= (1<<COM0A1);
   TCCR0A |= ((1<<WGM01)|(1<<WGM00));
   TCCR0B |= ((1<<CS02)|(1<<CS00));
   DDRD |= (1<<DDD6);
   
   /*
   initTimer1Servo ***Remember to Change this in the header too!
   ICR1 = 40000;

  TCCR1A |= (1<<WGM11)|(1<<COM1A1);
  TCCR1B |= (1<<WGM13)|(1<<WGM12)|(1<<CS11);
  DDRB |= (1<<DDB1);
   
   */
   
}

void setPulseLength(uint16_t value) {
/* Change the pulse length 
   Print out the value, in ms, of the new pulse. */

	OCR0A = value;
	
	/*
		OCR1A = value;
	*/
}
