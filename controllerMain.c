#include <avr/io.h>
#include "USARTE85.h"   			// UART initializations
#include "ioE85.h"    
#include <avr/delay.h>	
#include <avr/interrupt.h>  		// Includes Interrupts
#include "i2c.h"
#include "JoySticklib.h"
#include "displayLib.h"
#include "txlib.h"
#include <avr/sleep.h>
#include "nrf24l01Plus.h"
#include "rxlib.h"

nRF24L01 *setup_rf(void);
void tx(uint16_t xcoord, uint16_t ycoord, nRF24L01 *rf);
void SleepModeInit(void);
void initPinChangeInterrupt(void);
uint16_t * rx(uint16_t returnVal[], nRF24L01 *rf);

volatile uint8_t rf_interrupt = 0;
volatile int buttonPress = 0;

int main(void) {
	
	SleepModeInit();
	initPinChangeInterrupt();
	USART_Init(); 					// Enables Printing 
	initI2C();
	ADC_Init();						// Enables ADC conversion
	
	nRF24L01 *rf; // struct that holds all of the relevant rf info 
	rf = setup_rf();  // setup chip (pin assignments, interrupt, etc)
	
	/* configure button */
	PORTD |= (1<<PORTD7);
	DDRD &= ~(1 << DDD7);
	
	uint16_t final[3];
	uint16_t returnVal[3];
	uint16_t *address;
	uint16_t *recievedData;
	
	printf("\r\nHello world!\r\n");
	
	while (1) {	
		sei();
		if (buttonPress == 0) {  //Need to put the car motors stopped
			SMCR |= 0x01;  //Sets Sleep Enabled Bit
			printf("\r\nI'm going to bed\r\n");
			sleep_cpu();			
		}
		
		address = joystickValues(final);
		
		printf("\n\rX: %u", final[0]);
		printf("\n\rY: %u", final[1]);
		
		tx(final[0], final[1], rf);
		
		_delay_ms(10);
		
		recievedData = rx(returnVal, rf);
		
		rf = setup_rf();
		
		_delay_ms(10);
		
		printf("\r\nReturn Value X: %u", returnVal[0]);
		printf("\r\nReturn Value Y: %u", returnVal[1]);
		printf("\r\nReturn Value Other: %u", returnVal[2]);
		
		counterDisplay(returnVal[1]);	
		
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