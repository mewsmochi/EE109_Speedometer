/********************************************
*
*  Name: Leila Saxby
*  Section: 20261_31395 EE-109
*  Assignment: Lab 0
*
********************************************/

#include <avr/io.h> 
#include <util/delay.h> 
#include <avr/interrupt.h> 
#include <stdlib.h> 
#include <stdio.h>

//using "extern" to acces variables that exist elsewhere already -> i want speed and the threshold for comparison
extern int16_t speed;
extern int8_t thresh;

void led_init(void){
	//initalize PC1 , PC2, and PC3 as outputs
	//PC1 - R, PC2 - G, PC3 - B
	DDRC |= (1 << PC1 | 1 << PC2 | 1 << PC3);
	//set to blue
	PORTC |= (1 << PC1 | 1 << PC2 | 1 << PC3);
	PORTC &= ~(1 << PC3);
	PORTC |= ( 1 << PC1 | 1 << PC2);
}

void led(void) {
    if (speed == 0) { //turn on blue
		PORTC |= (1 << PC1 | 1 << PC2 | 1 << PC3);
		PORTC &= ~(1 << PC3);
	} else if (abs(speed) <= thresh*10) { //be green
		PORTC |= (1 << PC1 | 1 << PC2 | 1 << PC3);
		PORTC &= ~(1 << PC2);
	} else if (abs(speed) > thresh*10) { //be red
		PORTC |= (1 << PC1 | 1 << PC2 | 1 << PC3);
		PORTC &= ~(1 << PC1);
	}
}
    
