/********************************************
 *
 *  Name: Leila saxby
 *  Email: lsaxby@usc.edu
 *  Section: 12:30F
 *  Assignment: Final Project: Speedometer
 *
 ********************************************/

#include <avr/io.h> 
#include <util/delay.h> 
#include <avr/interrupt.h> 
#include <stdlib.h> 
#include <stdio.h>

#include "lcd.h" 
#include "adc.h" 
#include "thresh.h"
#define MAX_COUNT 50000

enum states { STOP, MEASURE1, MEASURE1_DONE, STOPWATCH, STOPWATCH_DONE, MEASURE2, MEASURE2_DONE };
enum displays { LOCAL, REMOTE };
volatile uint16_t pulse_count = 0; //store timer1count
volatile uint8_t edge = 1; //1 = expecting rising, 0 = expecting falling
uint8_t last_pin = 0; //tracking last pin value to compare to current
uint16_t d1 = 0; //first distance
uint16_t d2 = 0; //second distance
volatile uint16_t elapsed_time = 0; //stopwatch variable
int16_t speed = 0; //speed variable --> global so it can be used in other files
int16_t speed_ones;
int16_t speed_tenths;
//set initial state
uint8_t state = STOP;
uint8_t display_mode = LOCAL;
//extern "rec_speed_tenths" var and "rec_speed_ones" var to display received speed
extern volatile int8_t rec_speed_ones;
extern volatile int8_t rec_speed_tenths;
uint8_t select_pressed = 0; //select button flashes HORRENDOUSLY without
_Bool trigger_refresh = 0; //for if elapsed_time or invalid measure reading!

//store button adc value to determine left vs right
unsigned char adc_button;
char result1[12];
char result2[12];
char result3[12];
char speed_final_t[25];
char remote_speed[25];
void timer1_init_measure(void);
void timer1_init_stopwatch(void);


void speedometer_init(void){
    // Initialize the LCD 
    lcd_init();

    // Initialize the ADC 
    adc_init();
    // Initialize TIMER1 Measure mode
    timer1_init_measure();

    //initialize pins
    //set PD2 as input
    DDRD &= ~(1 << PD2);
    PORTD &= ~(1 << PD2); //no pull-up
    
    //set PD3 as output
    DDRD |= (1 << PD3);

    // Enable interrupts
    PCICR |= (1 << PCIE2); //Pin CHange Interrupt Control register --> turn on Port D interrupts
    PCMSK2 |= (1 << PCINT18); //turn on SPECIFICALLY PD2 interrupt
    sei(); //enable interrupts

    //determine what the "current" last pin is
    if (PIND & (1 << PD2)) {
        last_pin = 1;
    } else {
        last_pin = 0;
    }
}

void speedometer_update (void) {
    adc_button = adc_sample(0);
    //choose whether to display the recieved speed or local speed
    if (185 <= adc_button && adc_button <= 225){
        if(!select_pressed){
            if(display_mode == LOCAL){
                display_mode = REMOTE;
            } else if (display_mode == REMOTE){
                display_mode = LOCAL;
            }
        }
        _delay_ms(200);
    } else {
            select_pressed = 0;
    }
    //enter first measure 
    if (135 <= adc_button && adc_button <= 175) {
        if (state == STOP || state == MEASURE2_DONE) {
            elapsed_time = 0;
            d1 = 0;
            d2 = 0;
            lcd_writecommand(1);
            thresh_init();
            state = MEASURE1;
            speed = 0;
        }
    } else if (0 <= adc_button && adc_button <= 20) {
        state = STOP;
        if(d1 != 0 && d2 != 0 && elapsed_time != 0){
            if(d2 > d1){
                speed = ((d2 - d1) * 10) / elapsed_time;
                speed_ones = speed / 10;
                speed_tenths = speed % 10;
            } else {
                speed = (((d1 - d2) * 10) / elapsed_time) * -1;
                speed_ones = speed / 10;
                speed_tenths = speed % 10;
            }
        }
    }
    if(trigger_refresh){
        lcd_writecommand(1);
        thresh_init();
        state = STOP;
        trigger_refresh = 0;
    }
    
    if (state == MEASURE1 || state == MEASURE2) {
        PORTD |= (1 << PD3);
        _delay_us(10);
        PORTD &= ~(1 << PD3);
        _delay_ms(60);
    }
    
    //if measured D1
    if (state == MEASURE1_DONE) {
        d1 = (pulse_count * 10) / 116;
        //loop back for continuos measurement IF stop is not pressed
        lcd_moveto(0,0);
        uint16_t d1_ones = d1 / 10;
        uint16_t d1_tenths = d1 % 10;
        snprintf(result1, 12, "%u.%u", d1_ones, d1_tenths);
        lcd_stringout(result1);
    }

    //if measured D2
    if (state == MEASURE2_DONE) {
        d2 = (pulse_count * 10) / 116; //mm
        //loop back for continuos measurement IF stop is not pressed
        lcd_moveto(0,7);
        uint16_t d2_ones = d2 / 10;
        uint16_t d2_tenths = d2 % 10;
        snprintf(result3, 12, "%u.%u", d2_ones, d2_tenths);
        lcd_stringout(result3);
    }

    if (state == STOPWATCH) {
        uint16_t seconds = elapsed_time / 10;
        uint16_t tenths = elapsed_time % 10;
        snprintf(result2, 12, "%u.%u", seconds, tenths);
        lcd_moveto(0,13);
        lcd_stringout(result2);
    }

    if(state == STOP && d1 != 0 && elapsed_time == 0){ //if stop pressed and distance1 is done measuring
        state = STOPWATCH;
        timer1_init_stopwatch();
    }
    if(state == STOP && elapsed_time != 0 && d2 == 0){
        timer1_init_measure();
        state = MEASURE2;
    }
    if(display_mode == LOCAL){
        snprintf(speed_final_t, 25, "%d.%dcm/s", speed_ones, abs(speed_tenths));
        lcd_moveto(1,0);
        lcd_stringout(speed_final_t);

    } else {
        if(rec_speed_ones > 0){
            snprintf(remote_speed, 25, "r%d.%dcm/s", rec_speed_ones, rec_speed_tenths);
            lcd_moveto(1,0);
            lcd_stringout(remote_speed);
        }
    }
}



void timer1_init_measure() { 
    // Add lines to initialize TIMER1 
    TCCR1A = 0; // clear timer1 settings (mainly CS__ prescaler settings) 
    TCCR1B = 0; //code starts running with the timer stopped 
    TCNT1 = 0; //set counter value to 0 

    OCR1A = MAX_COUNT;  // max allowed count
    TIMSK1 |= (1 << OCIE1A);    // enable compare match interrupt
}

void timer1_init_stopwatch(void)
{
    TCCR1A = 0; // clear timer1 settings (mainly CS__ prescaler settings) 
    TCCR1B = 0; //code starts running with the timer stopped 
    TCNT1 = 0; //set counter value to 0     
    
    TCCR1B |= (1 << WGM12);     // Set for CTC mode using OCR1A for the modulus
    TIMSK1 |= (1 << OCIE1A);    // Enable CTC interrupt
	//desired interval = 0.1s 
	// 16MHz * 0.1 = 1.6m --> too big!
	//prescalar = 64
	//OCR1A = (0.1)*(16E6[Hz])/64 = 25,000
    TCCR1B |= (1<<CS11) | (1<<CS10); //start timer, set prescalar to 64 (011)
    OCR1A = 25000;                    // Set the counter modulus correctly
}

//triggers if any enabled pin on PortD is triggered. 
ISR(PCINT2_vect) { 
    if (state != MEASURE1 && state != MEASURE2){
        return;
    }
    uint8_t current; //tracking current pin value to compare to last_pin
    //stores current pin value
    if (PIND & (1 << PD2)) {
        current = 1;
    } else {
        current = 0;
    }
    // 0 to 1 --> rising edge
    if (last_pin == 0 && current == 1) {
        TCNT1 = 0;
        TCCR1B = (1 << CS11);  // start timer
    }

    // 1 to 0 --> falling edge
    else if (last_pin == 1 && current == 0) {
        TCCR1B = 0;            // stop timer
        pulse_count = TCNT1;
        if(state == MEASURE1){
            state = MEASURE1_DONE;
        } else if (state == MEASURE2){
            state = MEASURE2_DONE;
        }
    }

    last_pin = current;
}

ISR(TIMER1_COMPA_vect)
{
    if (state == STOPWATCH){
        elapsed_time++;
        if (elapsed_time >= 100){ //if we reach ten seconds, return to STOP state!
            d1 = 0;
            speed = 0; //set speed to 0 for led JUST in case
            elapsed_time = 0;
            trigger_refresh = 1;
            return;
        }
    } else if (state == MEASURE1 || state == MEASURE2) {
        TCCR1B = 0;         // stop timer
        pulse_count = 0;    // disregard invalid measurement      
        last_pin = 0; // reset state
        speed = 0; //set speed to 0 for led JUST in case
        trigger_refresh = 1;
    }
}
