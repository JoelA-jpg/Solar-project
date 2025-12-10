#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <p_func.h>

#define F_CPU 8000000UL
//#define PERIOD    20000   // 20 ms total period in µs
#define MIN_PULSE 1500    // 1.5 ms
#define MAX_PULSE 2200    // 2.2 ms

volatile uint16_t pulsewidth = 500; // current high pulse width
volatile uint8_t phase = 0;           // 0 = high, 1 = low
volatile uint16_t PERIOD = 20000 * 1.01;

// Initialize Timer1
void Timer1_init(void)
{
    DDRB |= (1 << PB1);        // PB1 / OC1A as output

    TCCR1A = 0;                // normal port operation
    TCCR1B = (1 << WGM12) | (0 << CS12) | (1 << CS11) | (0 << CS10); // CTC mode, no prescaler

    OCR1A = pulsewidth;       // initial compare value
    TIMSK1 = (1 << OCIE1A);    // enable compare A interrupt
}

static inline void initADC0(void){
    // ADC0 and AVCC as reference
    ADMUX = (1 << REFS0) | (0 << MUX0); // REFS0=1: AVCC, MUX[3:0]=0: ADC0
    ADMUX &= 0xF0; // Clear MUX bits (bits 3:0)
    ADCSRA |= (1 << ADPS1) | (1 << ADPS0); // prescaler = 8
    ADCSRA |= (1 << ADEN); // enable ADC
}

// Update pulse width dynamically
void Update_Pulse(uint16_t pw)
{
    pulsewidth = pw;
}

// Timer1 Compare Match A ISR
ISR(TIMER1_COMPA_vect)
{
    if (phase == 0)
    {
        // End of high pulse -> set pin low
        PORTB &= ~(1 << PB1);
        phase = 1;
        //OCR1A = pulsewidth - 1;
        OCR1A = PERIOD - pulsewidth; // low phase duration 8.14
    }
    else
    {
        // End of low phase -> set pin high
        PORTB |= (1 << PB1);
        phase = 0;
        //OCR1A = (PERIOD - pulsewidth) - 1;
        OCR1A = pulsewidth * 1.01;          // high phase duration
    }
}

int main(void)
{
    Timer1_init();
    initADC0();
    sei(); // enable global interrupts

    //init variables
    uint16_t adc_value = 0;
    uint16_t pw = 500;
    uint16_t K = 1;

    while (1)
    {
        ADCSRA |= (1 << ADSC); 
        loop_until_bit_is_clear(ADCSRA, ADSC); 
        adc_value = ADC; // Read ADC value (0-1023)
        //pw = p_funcy(pw, adc_value, 10*K); // proportional control pw
        pw = p_bangbang(adc_value, 200 , 20); // bang bang control pw
        Update_Pulse(pw); // update high pulse width dynamically
        _delay_ms(20*8.14);
        
    }
}
