/********************************************
*
*  Name: Leila Saxby
*  Section: 20261_31395 EE-109
*  Assignment: Lab 0
*
********************************************/
//my modules!
#include "lcd.h" 
#include "adc.h" 
#include "speed.h"
#include "thresh.h"
#include "led.h"
#include "servo.h"
#include "buzzer.h"

#include <avr/io.h> 
#include <util/delay.h> 
#include <avr/interrupt.h> 
#include <stdlib.h> 
#include <stdio.h>

void speedometer_init(void);
void speedometer_update(void);
void thresh_init(void);
void thresh_update(void);
void led_init(void);
void led(void);
void timer2_init(void);
void servo_control(void);
void buzzer_init(void);
void buzzer(void);

#define FOSC 16000000 // Clock frequency
#define BAUD 9600 // Baud rate used
#define MYUBRR (FOSC/16/BAUD-1) // Value for UBRR


//i changed rec_speed from int to int8_t var type
//use extern to use the speed variable, so I can replace it with a recieved value if necessary
extern volatile int16_t speed;
char rec[6];
int rec_speed; //need these signed
volatile int rec_speed_tenths;
volatile int rec_speed_ones;
char trans[8];
_Bool data_start = 0;
_Bool val_data = 0;
uint8_t buff_count;;
uint16_t last_speed = 0; //track if speed changes

void tx_char(char ch)
{
	// Wait for transmitter data register empty
	while ((UCSR0A & (1<<UDRE0)) == 0) {}
	UDR0 = ch;
}

void tx_str(char *str)
{
	snprintf(trans, 6, "%s%u%s", "@", speed, "$");
	for(int i = 0; i != 6; i++) { //using a for loop to parse through the string until the end!	
		tx_char(trans[i]);
	}
}

int main(void) {
   	//initalize lcd and speed module
	timer2_init();
	lcd_init();
	speedometer_init();
	led_init();
	buzzer_init();

	//initialize USART for Rx and Tx
	UBRR0 = MYUBRR; //set UBRR0 value
	UCSR0B |= (1 << TXEN0) | (1 << RXEN0) | (1 << RXCIE0); // Enable RX,TX, and interrupt
	UCSR0C = (3 << UCSZ00); // Async., no parity, 
	// 1 stop bit, 8 data bit
	sei();

	DDRB |= (1 << PB4); //pb4 as output
	PORTB &= ~(1 << PB4);

	
	_delay_ms(50);
	lcd_writecommand(1);
	lcd_moveto(0,3);
	lcd_stringout("Leila Saxby");
	lcd_moveto(1,0);
	lcd_stringout("EE109 Speedometer");
	_delay_ms(1000);
	lcd_writecommand(1);
	
	
	//need to put here so THR shows up
	thresh_init();

    while (1) { // Loop forever
		thresh_update();
		led();
		servo_control();
		speedometer_update();
		if (speed != last_speed) {
			tx_str(trans);
			last_speed = speed;
		}
		if (val_data == 1){
			sscanf(rec, "%d", &rec_speed);
			rec_speed_ones = rec_speed / 10;
			rec_speed_tenths = abs(rec_speed % 10);
			buzzer();
			val_data = 0;
		}
    }
}

ISR(USART_RX_vect) {
	char c = UDR0;
	if(c == '@' || c == '$' || c == '-' || ('0' <= c && c <= '9')){
		if(c == '@'){
			data_start = 1;
			buff_count = 0;
			val_data = 0;
		} else if (data_start) {
			if(c == '$') {
				if(buff_count != 0){
					rec[buff_count] = '\0'; //null terminator, needed for c strings to make sscanf work
					val_data = 1;
					data_start = 0;
				}
			} else {
				//save ONE space for null terminator
				if(buff_count < 5){
					rec[buff_count] = c;
					buff_count++;
				} else {
					data_start = 0;
				}
			}
		}
	} else {
		data_start = 0;
		val_data = 0;
	}
}
    
