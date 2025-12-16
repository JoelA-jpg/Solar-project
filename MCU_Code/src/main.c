
#define F_CPU 8000000UL

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <p_func.h>


//#define PERIOD    20000   // 20 ms total period in µs
#define MIN_PULSE 1500    // 1.5 ms
#define MAX_PULSE 2200    // 2.2 ms

volatile uint16_t pulsewidth = 50; // current high pulse width
volatile uint8_t phase = 0;           // 0 = high, 1 = low
volatile uint16_t PERIOD = 80;// * 1.01;

// Initialize Timer1
void Timer1_init(void)
{
    DDRB |= (1 << PB1);        // PB1 / OC1A as output

    TCCR1A = 0;                // normal port operation
    TCCR1B = (1 << WGM12) | (0 << CS12) | (0 << CS11) | (1 << CS10); // CTC mode, no prescaler

    OCR1A = pulsewidth;       // initial compare value
    TIMSK1 = (1 << OCIE1A);    // enable compare A interrupt
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
        // End of high pulse → set pin low
        PORTB &= ~(1 << PB1);
        phase = 1;
        //OCR1A = pulsewidth - 1;
        OCR1A = PERIOD - pulsewidth; // low phase duration 8.14
    }
    else
    {
        // End of low phase → set pin high
        PORTB |= (1 << PB1);
        phase = 0;
        //OCR1A = (PERIOD - pulsewidth) - 1;
        OCR1A = pulsewidth * 1.01;          // high phase duration
    }
}

int main(void)
{
    Timer1_init();
    sei(); // enable global interrupts

    uint16_t pw = 30;

    while (1)
    {
        // Sweep pulse from MIN_PULSE to MAX_PULSE

        for(int i = 1; i < 5; i++){
        Update_Pulse(0 + pw*1); // update high pulse width dynamically
        _delay_ms(3000);
        }
    }
}