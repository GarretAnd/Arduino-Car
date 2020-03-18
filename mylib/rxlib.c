#include <avr/io.h>
#include <avr/interrupt.h>
#include "nrf24l01Plus.h"
#include "USARTE85.h"
#include "ioE85.h"

void disAssembleMessage(nRF24L01Message *msg, uint8_t *x, uint8_t *y, uint8_t *other);

/* The format in which you store your data in the array msg->data is entirely up to you.
   You can store (and send) up to 32 bytes, via data[0], data[1], ..., data[31] in one
   packet and simply have to make sure the program on the receiving end understands 
   your data formatting choice. */
void disAssembleMessage(nRF24L01Message *msg, uint8_t *x, uint8_t *y, uint8_t *other) {  // Assembles message function from sample
	*x = msg->data[0];
	*y = msg->data[1];
	for (uint8_t i=0; i<10; i++) {
		other[i] = msg->data[i+2]; 
	}
}