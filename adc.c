#include <avr/io.h>

#include "adc.h"
#define MASKBITS 0x0f

void adc_init(void)
{
    // Initialize the ADC
    ADMUX |= ((1 << REFS0) | (1 << ADLAR));
    ADMUX &= ~(1 << REFS1);
    
    ADCSRA |= ((1 << ADEN) | (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0));
    ADCSRA &= ~((1 << ADSC) | (1 << ADATE) | (1 << ADIF) | (1 << ADIE));

}

uint8_t adc_sample(uint8_t channel)
{
    // Set ADC input mux bits to 'channel' value
    ADMUX &= ~MASKBITS;
    ADMUX |= ((channel & MASKBITS));
   
    // Convert an analog input and return the 8-bit result
    ADCSRA |= (1 << ADSC);
    while (1){
        if((ADCSRA & (1 << ADSC)) == 0){
            break;
        }
    }
    unsigned char result = ADCH;
    return result; 
}
