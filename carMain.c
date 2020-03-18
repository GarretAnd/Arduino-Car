#include <avr/io.h>
#include "USARTE85.h"   			// UART initializations
#include "ioE85.h"    
#include <avr/delay.h>	
#include <avr/interrupt.h>  		// Includes Interrupts
#include "i2c.h"
#include "txlib.h"
#include "nrf24l01Plus.h"
#include "rxlib.h"
#include "SteeringLib.h"
#include "motorlib.h"

#define MOTOR1 0x26
#define MOTOR2 0x0E

nRF24L01 *setup_rf(void);
void tx(uint16_t xcoord, uint16_t ycoord, nRF24L01 *rf);
uint16_t * rx(uint16_t returnVal[], nRF24L01 *rf);

volatile uint8_t rf_interrupt = 0;

int main(void) {

	initShield();
	
	nRF24L01 *rf; // struct that holds all of the relevant rf info 
	rf = setup_rf();  // setup chip (pin assignments, interrupt, etc)
	
	uint16_t final[3];
	uint16_t returnVal[3];
	uint16_t *address;
	uint16_t *recievedData;
	
	uint16_t xcoord;
	uint16_t ycoord;
	
	uint8_t direction;
	
	printf("\r\nHello world!\r\n");
	
	while (1) {	
		sei();
		
		recievedData = rx(returnVal, rf);
		
		xcoord = returnVal[0];
		ycoord = returnVal[1];
		
		if (ycoord >= 145) {
			direction = 0;
			ycoord = (((ycoord - 145) * 32) + 500);  
		}
		else if (ycoord <= 125) {
			direction = 1;
			ycoord = (((125 - ycoord) * 28) + 500);
		}
		else if (ycoord < 145 && ycoord > 125) {
			direction = 2;
			ycoord = 0;
		}
		
		/*
			The next problem is determining speed - the y coordinates we receive are between 0 and 256, the stopping value is 125 to 145, or the middle of the joystick. 
			For forward and backwards speed, we want absolute value of the distance from the stopping position 
			(so for backwards direction - any ycoord less than stopping position -, the lower the ycoord the higher the speed,
			while for the front direction - any y coord higher than stopping range -, the high the ycoord, the higher the speed - Now we have a gradient of 0 to 256, 
			but for the motor function we want speed to be between 500-4000. 
			So for each respective range we take a multiplier (32 and 28) so the range of ycoord for each directions corresponds to a range of value between 0-3500 linearly, 
			then add 500 (the minimum speed before motors stall out) so the range inserted into motor function is 500-4000
		*/
		
		motorrun(ycoord, MOTOR1, direction);  //Runs the motor at ycoord given from if statement above, address specific motor, and gives direction of wheel
		motorrun(ycoord, MOTOR2, direction);
		
		steer(xcoord);  //Sets Steering Degree
		
		rf = setup_rf();
		
		_delay_ms(10);
		
		//tx(final[0], final[1], rf);
		
		_delay_ms(10);
		
		
		printf("\r\nRestart Loop\r\n");	
		buttonPress = 1;
		cli();
	}
	
	return 0;	// Never Reached
}

/* nRF24L01 interrupt - raised if transmission successful or if timed out. */
ISR(INT0_vect) {
    rf_interrupt = true;
}

/* You can change the SS and CE pins in the setup below as long as your wiring
   reflects your choice. The SCK, MOSI and MISO pins should remain unchanged. 
   You can also use a different interrupt. */


nRF24L01 *setup_rf(void) {
    nRF24L01 *rf = nRF24L01_init();
    rf->ss.port = &PORTB;
    rf->ss.pin = PB2;
    rf->ce.port = &PORTD;
    rf->ce.pin = PD5;
    rf->sck.port = &PORTB;
    rf->sck.pin = PB5;
    rf->mosi.port = &PORTB;
    rf->mosi.pin = PB3;
    rf->miso.port = &PORTB;
    rf->miso.pin = PB4;
    // interrupt on falling edge of INT0 (PD2)
    EICRA |= (1 << ISC01);
    EIMSK |= (1 << INT0);
    nRF24L01_begin(rf);
    return rf;
}

void tx(uint16_t xcoord, uint16_t ycoord, nRF24L01 *rf) {
	
/* This first section contains setups for use on the transmission side */
    uint8_t to_address[6] = "david"; // packet address - FIVE characters    
    nRF24L01Message msg; // for sending message
	
/* Be sure to use the same address on the receiver as you defined here. */

/* Now things specific to this example */
	printf("\r\nWelcome to the TX Lib");
	printf("\r\nThe 16 bit Xcoord is: %u", xcoord);
	printf("\r\nThe 16 bit Ycoord is: %u", ycoord);
	
	uint8_t xcoord8bit = xcoord/4;
	uint8_t ycoord8bit = ycoord/4;
	
	printf("\r\nThe 8-bit Xcoord is: %u", xcoord8bit);
	printf("\r\nThe 8-bit Ycoord is: %u", ycoord8bit);

    typedef enum {SEND, WAIT, CHECK, NEW} state_t; // for state machine example
	state_t state;
	printf("Welcome to the RF Message Transmitter!\n\r");
    uint8_t xvalue = xcoord8bit;  // assemble some data - maybe coming from the joystick?
    uint8_t yvalue = ycoord8bit; // You choose format of your own data - up to 32 bytes total
    uint8_t other[10] = "Secret :)"; 
    assembleMessage(&msg, xvalue, yvalue, other);      
    state = SEND;
    
    while (state != NEW) {
       switch (state) {
       		case SEND: // loads nRF registers and initiates transmission
       			nRF24L01_transmit(rf, to_address, &msg); 
       			printf("Message queued\n\r");			
       			state = WAIT;
       		break;
       		case WAIT: // wait for interrupt which tells you TX done
       			printf("transmitting..."); 
       			if (rf_interrupt) {
       				rf_interrupt = 0;
       				state = CHECK;
       			}
       		break;
       		case CHECK: // interrupt has happend - check if ACK was received
           		if (nRF24L01_transmit_success(rf) != 0) { // something went wrong 
                	printf("\n\rTransmission not successful.\n\r");
                	nRF24L01_flush_transmit_message(rf); 
            	} else { // transmission complete, ACK received
        			printf("\n\rMessage successfully transmitted. \n\r");
       			}
       			state = NEW; // ready for more
			case NEW:
				state = NEW; //Never Reached
       		break;		
    	}
    }
}

void initPinChangeInterrupt(void) {
  /* set pin-change interrupt for B-pins */
	PCICR |= (1 << PCIE2);  //Go to notes from 1/27 to see why this E2 is used, it varies per bank
	PCMSK2 |= (1 << PD7);	//Specificed to pin 7

  /* set mask to look for PB4 */

  /* set (global) interrupt enable bit */
}

void SleepModeInit(void) {
	SMCR = 0b00000110; //Power Save Mode (7.2) in data sheets
}

ISR(PCINT2_vect) {
  /* set flag to notify main */
  	SMCR &= ~(0x01);
	if (buttonPress == 0) {
		buttonPress = 1;
	}
	else {
		buttonPress = 0;
	}
}

uint16_t * rx(uint16_t returnVal[], nRF24L01 *rf) {
/* This first section contains setups for use on the receiver side */
    uint8_t address[6] = "david"; // packet address 
    nRF24L01Message msg; // holds received message
/* Be sure to use the same address here as you defined on the transmiter. */

/* Now things specific to this example */
    printf("Welcome to the RF message receiver! \n\r");

    uint8_t xvalue; // to hold transmitted data - maybe coming from the joystick? 
    uint8_t yvalue; // You choose format of your own data - up to 32 bytes total
    uint8_t other[10];
	uint8_t counter = 0;
    
    while (counter <= 30) {
    	nRF24L01_listen(rf, 0, address); // put chip in listen mode
        if (rf_interrupt) {  // packet received
            rf_interrupt = 0;
            if (nRF24L01_data_received(rf)) {
                nRF24L01_read_received_data(rf, &msg); //extract msg
                disAssembleMessage(&msg, &xvalue, &yvalue, other); //interpret
                printf("x = %u, y = %u, other = %s\r\n", xvalue, yvalue, other);
				returnVal[0] = xvalue;
				returnVal[1] = yvalue;
				returnVal[2] = other;
            }
        }
		printf("\r\nThe Counter is at: %u", counter);
		counter++;
    }
	printf("\r\nI'm Done.");
}