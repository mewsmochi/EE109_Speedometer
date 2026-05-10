#include <avr/io.h>
#include <util/delay.h>
#include <stdio.h>
#include <avr/interrupt.h>
#include <avr/eeprom.h>
#include <stdlib.h> 

#include "lcd.h"
#define ADDRESS 69

volatile uint8_t new_state, old_state;
volatile uint8_t changed;       // Flag that count changed
volatile int8_t count = 0;	// Count to display
int8_t thresh = 0;
int8_t saved_thresh;
uint8_t bits, a, b;
char cnt[14];
char init[12];

void thresh_init (void) { //initialization for thresh_update function	
    // Initialize appropriate DDR and PORT registers
    PORTC |= (1 << PC5 | 1 << PC4); // Enable pull-ups on PC5, PC4             // Initialize the LCD

	PCICR |=(1<<PCIE1); // enable PORTC interrupts
	PCMSK1 |= (1<<PCINT13) | (1<<PCINT12); //enable PCINT on PC5 and PC4

    // Determine the intial state
    bits = PINC;
    a = bits & (1 << PC4);
    b = bits & (1 << PC5);

    if (!b && !a)
		old_state = 0;
    else if (!b && a)
		old_state = 1;
    else if (b && !a)
		old_state = 2;
    else
		old_state = 3;

    new_state = old_state;
	saved_thresh = eeprom_read_byte((void *) ADDRESS);
	if (saved_thresh < 0 || saved_thresh > 99){
		thresh = 0;
		snprintf(init, 12, "TH:%02d", 0);
		lcd_moveto(1,11);
		lcd_stringout("INV");
		_delay_ms(700);
		lcd_writecommand(1);
	} else {
		thresh = saved_thresh;
		snprintf(init, 12, "TH:%02d", thresh);
		lcd_moveto(1,11);
	}
	lcd_stringout(init);
	count = thresh;
}

void thresh_update (void) { //function to update speed threshold
	if (changed) { // Did encoder change the count?
		if(count <= 0){
			thresh = 0;
			if(count < 0){
				count = 0;
			}
		} else if (count >= 99) {
			thresh = 99;
			if(count > 99){
				count = 99;
			}
		} else if (thresh != count) {
			thresh = count;
			eeprom_update_byte((void *) ADDRESS, thresh);
		}
		//use padding to clear extra digits when we reach the ones place 
		snprintf(cnt, 12, "TH:%02d", thresh);
		lcd_moveto(1,11);
		lcd_stringout(cnt);    // Print value of count
	}
	changed = 0;
}	



ISR(PCINT1_vect){
	uint8_t bits = PINC;		// Read the two encoder inputs at the same time
	uint8_t a = bits & (1 << PC4);  // a = LSB of state number
	uint8_t b = bits & (1 << PC5);  // b = MSB of state number

	if (old_state == 0) {
	    if (!b && a) {
			new_state = 1;
			count++;
	    }
	    else if (b && !a) {
			new_state = 3;
			count--;
	    }
	}
	else if (old_state == 1) {
	    if (b && a) {
			new_state = 2;
			count++;
	    }
	    else if (!b && !a) {
			new_state = 0;
			count--;
	    }
	}
	else if (old_state == 2) {
	    if (b && !a) {
			new_state = 3;
			count++;
	    }
	    else if (!b && a) {
			new_state = 1;
			count--;
	    }
	}
	else {   // old_state = 3
	    if (!b && !a) {
			new_state = 0;
			count++;
	    }
	    else if (b && a) {
			new_state = 2;
			count--;
	    }
	}

	if (new_state != old_state) {
	    changed = 1;
	    old_state = new_state;
	}
}
