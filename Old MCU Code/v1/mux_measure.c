#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#define F_CPU 8000000UL

uint8_t measurements[8];
uint8_t truth_table[8] = {
    0b00000000,
    0b00000001,
    0b00000010,
    0b00000011,
    0b00000100,
    0b00000101,
    0b00000110,
    0b00000111
}; //PB0 = A, PB1 = B, PB2 = C

void init(){
    DDRB |= (1 << PB0) | (1 << PB1) | (1 << PB2); //Designate as output
    PORTB |= (0 << PB0) | (0 << PB1) | (0 << PB2); //Status of output pin
}

void adc_init(void)
{
    // Reference = AVcc (5V)
    ADMUX = (1 << REFS0);

    // ADC Enable + Prescaler 128 -> 16MHz/128 = 125kHz
    ADCSRA = (1 << ADEN) | (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0);
}

uint16_t adc_read(uint8_t channel)
{
    // Select ADC channel (0–7)
    ADMUX = (ADMUX & 0xF0) | (channel & 0x0F);

    // Start conversion
    ADCSRA |= (1 << ADSC);

    // Wait for conversion to finish
    while (ADCSRA & (1 << ADSC));

    // Read result
    return ADC;
}

void mux_measure(){
    for(int i = 0; i < 9; i++){
        PORTB = truth_table[i];

        if(i < 4){
            measurements[i] = adc_read(0); //Strom matning
        }
        else if(i > 4){
            measurements[i] = adc_read(1); //Spanningsmatning
        }
    }
}

int main(void)
{
    while (1)
    {
        mux_measure();
    }
}
