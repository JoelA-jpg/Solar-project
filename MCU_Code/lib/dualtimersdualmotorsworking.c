#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <p_func.h>

#define F_CPU 8000000UL
//#define PERIOD    20000   // 20 ms total period in µs
#define MIN_PULSE 1500    // 1.5 ms
#define MAX_PULSE 2200    // 2.2 ms

volatile uint16_t pulsewidth1 = 1500; // current high pulse width
volatile uint16_t pulsewidth0 = 1500;
volatile uint8_t phase0 = 0;
volatile uint8_t phase1 = 0;  
volatile uint32_t PERIOD0 = 156;        // 0 = high, 1 = low
volatile uint16_t PERIOD1 = 20000 * 1.01;


// Initialize Timer1
void Timer0_init(void){
    DDRD |= (1 << PD6);        // PD6 / OC0A as output

    TCCR0A = (1 << WGM01) | (0 << WGM00);
    TCCR0B = (0 << WGM02) | (1 << CS02) | (0 << CS01) | (1 << CS00); // CTC mode, no prescaler

    OCR0A = pulsewidth0;       // initial compare value
    TIMSK0 = (1 << OCIE0A);    // enable compare A interrupt
}

// Initialize Timer1
void Timer1_init(void)
{
    DDRB |= (1 << PB1);        // PB1 / OC1A as output

    TCCR1A = 0;                // normal port operation
    TCCR1B = (1 << WGM12) | (0 << CS12) | (1 << CS11) | (0 << CS10); // CTC mode, no prescaler

    OCR1A = pulsewidth1;       // initial compare value
    TIMSK1 = (1 << OCIE1A);    // enable compare A interrupt
}

static inline void initADC0(void){
    // ADC0 and AVCC as reference
    ADMUX = (1 << REFS0) | (0 << MUX0); // REFS0=1: AVCC, MUX[3:0]=0: ADC0
    ADMUX &= 0xF0; // Clear MUX bits (bits 3:0)
    ADCSRA |= (1 << ADPS1) | (1 << ADPS0); // prescaler = 8
    ADCSRA |= (1 << ADEN); // enable ADC
}

static inline void initADC1(void){
    // ADC1 and AVCC as reference
    ADMUX = (1 << REFS0) | (1 << MUX0); // REFS0=1: AVCC, MUX[3:0]=1: ADC1
    ADMUX &= 0xF1; // Clear MUX bits except MUX0
    ADCSRA |= (1 << ADPS1) | (1 << ADPS0); // prescaler = 8
    ADCSRA |= (1 << ADEN); // enable ADC
}

// Update pulse width dynamically
void Update_Pulse1(uint16_t pw)
{
    pulsewidth1 = pw;
}

void Update_Pulse0(uint16_t pw)
{
    pulsewidth0 = (16*pw)/2200;
}

//ISR for Timer0 Compare Match A
ISR(TIMER0_COMPA_vect)
{
    if (phase0 == 0)
    {
        // End of high pulse -> set pin low
        PORTD &= ~(1 << PD6);
        phase0 = 1;
        //OCR1A = pulsewidth - 1;
        OCR0A = PERIOD0 - pulsewidth0; // low phase duration 8.14
    }
    else
    {
        // End of low phase -> set pin high
        PORTD |= (1 << PD6);
        phase0 = 0;
        //OCR1A = (PERIOD - pulsewidth) - 1;
        OCR0A = pulsewidth0;          // high phase duration
    }
}

// Timer1 Compare Match A ISR
ISR(TIMER1_COMPA_vect)
{
    if (phase1 == 0)
    {
        // End of high pulse -> set pin low
        PORTB &= ~(1 << PB1);
        phase1 = 1;
        //OCR1A = pulsewidth - 1;
        OCR1A = PERIOD1 - pulsewidth1; // low phase duration 8.14
    }
    else
    {
        // End of low phase -> set pin high
        PORTB |= (1 << PB1);
        phase1 = 0;
        //OCR1A = (PERIOD - pulsewidth) - 1;
        OCR1A = pulsewidth1 * 1.01;          // high phase duration
    }
}

int main(void)
{
    Timer0_init();
    Timer1_init();
    initADC0();
    initADC1();
    sei(); // enable global interrupts

    //init variables
    uint16_t adc_value0 = 0;
    uint16_t adc_value1 = 0;
    uint16_t pw0 = 1500;
    uint16_t pw1 = 1500;
    uint16_t K = 1;

    while (1)
    {
        //ADC 0
        ADMUX = (ADMUX & 0xF0) | 0x01;   // select ADC1
        ADCSRA |= (1 << ADSC); 
        loop_until_bit_is_clear(ADCSRA, ADSC); 
        adc_value1 = ADC; // Read ADC value (0-1023)
        pw1 = p_funcy(pw1, adc_value1, 4*K); // proportional control pw
        Update_Pulse1(pw1); // update high pulse width dynamically

        ADMUX = (ADMUX & 0xF0) | 0x00;   // select ADC0
        ADCSRA |= (1 << ADSC);
        loop_until_bit_is_clear(ADCSRA, ADSC);
        adc_value0 = ADC; // Read ADC value (0-1023)
        pw0 = p_bangbang(adc_value0, 250 , 150); // bang bang control pw
        Update_Pulse0(pw0); // update high pulse width dynamically
        _delay_ms(20*8.14);
        
    }
}
