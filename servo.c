/********************************************
 *
 *  Name: Leila saxby
 *  Email: lsaxby@usc.edu
 *  Section: 12:30F
 *  Assignment: Lab 7 - ADC and PWM
 *
 ********************************************/

#include <avr/io.h>
#include <util/delay.h>
#include <stdlib.h>
#include <stdio.h>

#include "lcd.h"
#include "adc.h"
#define MIN 12 //((PW(ms) * clk_f)/(prescalar * 1000)) - 1
#define MAX 35
extern volatile uint16_t elapsed_time; //grab elapsed time for comparison
uint16_t last_time = 0; //track if time changed

void servo_control(void)
{
    if(last_time != elapsed_time){ //check if elapsed_time changed
            // division by 100 was forcing rounding --> adding 50 forced to round up
            OCR2A = 35 - ((23 * elapsed_time + 50) / 100);
            last_time = elapsed_time;
    }
}

/*
  timer2_init - Initialize Timer/Counter2 for Fast PWM
*/
void timer2_init(void)
{
    // Add code to initialize TIMER2
    DDRB |= (1 << DDB3);
    TCCR2A |= (0b11 << WGM20);  // Fast PWM mode, modulus = 256
    TCCR2A |= (0b10 << COM2A0); // Turn D11 on at 0x00 and off at OCR2A
    OCR2A = 35;      // Initial pulse width (calculate this)
    TCCR2B |= (0b111 << CS20);  // Prescaler = 1024 for 16ms period
}
