#include <avr/io.h>
#include <util/delay.h>
#include <stdio.h>
#include <avr/interrupt.h>
#include <stdlib.h>

#include "lcd.h"

void timer0_init(void);
void play_tone(uint16_t, uint16_t);
volatile uint16_t buzz_count;   // How many transitions the ISR has to do
volatile uint8_t buzzing;       // Flag that tone is being played

extern int rec_speed;
extern int8_t thresh;

#define HI 2000   // 2 kHz
#define MED 1000  // 1 kHz
#define LOW 500 //500 Hz
#define LENGTH_NEG 450
#define LENGTH_POS 250

void buzzer_init(void){
	// Initialize appropriate DDR and PORT registers
    DDRB |= (1 << PB5);         // Set PORTB bit for buzzer output
    timer0_init();  // Inititialize TIMER0 for tones
	TCCR0B &= ~((1<<CS02) | (1<<CS01) | (1<<CS00)); // timer off			// start timer one off
	buzzing = 0;             
}

void buzzer(void) {
	// See if button was pressed to play the sequence
	if (rec_speed <= thresh*10) { //ascendd
		play_tone(LOW, LENGTH_NEG);
		play_tone(MED, LENGTH_NEG);
		play_tone(HI, LENGTH_NEG);
	} else if (rec_speed > thresh*10) { //descend
		play_tone(HI, LENGTH_POS);
		play_tone(MED, LENGTH_POS);
		play_tone(LOW, LENGTH_POS);
	}
}


/*
  play_tone - Plays a single tone on the buzzer at frequency "freq" and
  for "length" milliseconds
*/
void play_tone(uint16_t freq, uint16_t length)
{
    while (buzzing == 1) {}

    // Calculate OCR0A for desired frequency
    uint32_t ocr = (F_CPU / (2UL * 8 * freq)) - 1;
    OCR0A = (uint8_t)ocr;

    // num toggles
    buzz_count = (2UL * freq * length) / 1000;

    TCNT0 = 0;

    // Start timer
    TCCR0B |= (1 << CS01);

    buzzing = 1;
}

void timer0_init(void)
{
    // Add code to initialize TIMER0
    DDRB |= (1 << DDB5);
    TCCR0A &= ~(1 << WGM00);  // set to CTC mode
	TCCR0A |= (1 << WGM01);
	TCCR0B &= ~(1 << WGM02);
	TIMSK0 |= (1 << OCIE0A); //enable interrupts
	TCCR0B &= ~(1<<CS02 | 1<<CS01 | 1<< CS00); //no prescalar --> set in isr later -->timr off

}


/*
  ISR for TIMER1.  Used for buzzer output delays
*/
ISR(TIMER0_COMPA_vect)
{
    // Add lines for generating the output signal
	if (buzz_count == 0){
		TCCR0B &= ~((1<<CS02) | (1<<CS01) | (1<<CS00));
		buzzing = 0;
	} else {
		PORTB ^= (1 << PB5);
		buzz_count--;
	}

}
