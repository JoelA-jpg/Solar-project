#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <p_func.h>

#define F_CPU 8000000UL
//#define PERIOD    20000   // 20 ms total period in µs
#define MIN_PULSE 1500    // 1.5 ms
#define MAX_PULSE 2200    // 2.2 ms

volatile uint16_t pulsewidth_servo_a = 1000; // current high pulse width
volatile uint16_t pulsewidth_servo_b = 1000;
volatile uint8_t phase_a = 0;           // 0 = high, 1 = low
volatile uint8_t phase_b = 0;           // 0 = high, 1 = low
volatile uint16_t PERIOD = 156;       //8-bit value

// Initialize Timer1
void Timer0_init(void) //8 Bit timer init 2 of them
{
    DDRD |= (1 << PD6);        // PB1 / OC0A as output
    DDRD |= (1 << PD3);        // PB0 / OC0B as output

    TCCR0A = (1 << WGM01) | (0 << WGM00);
    TCCR0B = (0 << WGM02) | (1 << CS02) | (0 << CS01) | (1 << CS00); // CTC mode, no prescaler

    TCCR2A = (1 << WGM21) | (0 << WGM20);
    TCCR2B = (0 << WGM22) | (1 << CS22) | (1 << CS21) | (1 << CS20);

    OCR0A = pulsewidth_servo_a;       // initial compare value
    TIMSK0 = (1 << OCIE0A) | (0 << OCIE0B);

    OCR2B = pulsewidth_servo_b;
    TIMSK2 = (1 << OCIE2A) | (1 << OCIE2B);
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

// Timer0A
ISR(TIMER0_COMPA_vect)
{
    if (phase_a == 0)
    {
        // End of high pulse -> set pin low
        PORTD &= ~(1 << PD6);
        phase_a = 1;
        //OCR1A = pulsewidth - 1;
        OCR0A = PERIOD - pulsewidth_servo_a; // low phase duration 8.14
    }
    else
    {
        // End of low phase -> set pin high
        PORTD |= (1 << PD6);
        phase_a = 0;
        //OCR1A = (PERIOD - pulsewidth) - 1;
        OCR0A = pulsewidth_servo_a;          // high phase duration
    }
}

// Timer0B
ISR(TIMER2_COMPA_vect)
{
    if (phase_b == 0)
    {
        // End of high pulse -> set pin low
        PORTD &= ~(1 << PD3);
        phase_b = 1;
        //OCR1A = pulsewidth - 1;
        OCR2A = PERIOD - pulsewidth_servo_b; // low phase duration 8.14
    }
    else
    {
        // End of low phase -> set pin high
        PORTD |= (1 << PD3);
        phase_b = 0;
        //OCR1A = (PERIOD - pulsewidth) - 1;
        OCR2A = pulsewidth_servo_b;          // high phase duration
    }
}

void Update_Pulse_a(uint16_t pw_func)
{
    pulsewidth_servo_a = (16 * pw_func) / 2200 ;
}

void Update_Pulse_b(uint16_t pw_func)
{
    pulsewidth_servo_b = (16 * pw_func) / 2200 ;
}

int main(void)
{
    Timer0_init();
    initADC0();
    initADC1();
    sei(); // enable global interrupts

    //init variables
    uint16_t adc_value = 0;
    uint16_t pw_a = 16; //16 bin value _ 2200 * 0.007
    uint16_t pw_b = 16;
    uint16_t K = 1;

    while (1)
    {

        while (1)
    {
        ADCSRA |= (1 << ADSC); 
        loop_until_bit_is_clear(ADCSRA, ADSC); 
        adc_value = ADC; // Read ADC value (0-1023)
        pw_a = p_funcy(pw_a, adc_value, K); // proportional control pw
        Update_Pulse_a(pw_a);

        pw_b = p_funcx(pw_b, adc_value, K);
        Update_Pulse_b(pw_b);
        _delay_ms(20*8.14);
        
    }

        /*
        ADCSRA |= (1 << ADSC); 
        loop_until_bit_is_clear(ADCSRA, ADSC); 
        adc_value = ADC; // Read ADC value (0-1023)
        pw = p_funcy(pw, adc_value, K); // proportional control pw
        Update_Pulse(pw); // update high pulse width dynamically
        _delay_ms(20*8.14);
        */

        /*
        {
        for(int i = 0; i < 18; i++){
        Update_Pulse_a((500 + i*100));
        Update_Pulse_b((500 + i*100));
        _delay_ms(200*2);
        }
        }
        */

    }
}
