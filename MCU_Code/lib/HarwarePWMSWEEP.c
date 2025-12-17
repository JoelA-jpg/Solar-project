
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
static inline void pwm1_init_100kHz(void)
{
    // OC1A (PB1) output
    DDRB |= (1 << PB1);

    // Fast PWM, TOP = ICR1 (Mode 14: WGM13:0 = 1110)
    // Non-inverting on OC1A: clear on compare match, set at BOTTOM
    TCCR1A = (1 << COM1A1) | (1 << WGM11);
    TCCR1B = (1 << WGM13) | (1 << WGM12) | (1 << CS10); // prescaler = 1

    ICR1  = 79;     // TOP for 20 kHz at 8 MHz
    OCR1A = 20;     // ~50% duty to start
}

static inline void pwm1_set_duty_percent(uint16_t permille)
{
    // permille: 0..1000
    // OCR1A = duty*(TOP+1)
    uint32_t top_plus_1 = (uint32_t)ICR1 + 1;
    uint32_t val = (top_plus_1 * permille) / 100;
    if (val > top_plus_1) val = top_plus_1;
    OCR1A = (uint16_t)val;
}

int main(void)
{
    pwm1_init_100kHz();

    while (1)
    {
        // simple sweep: 10% -> 90% -> 10%
        for (uint16_t d = 30; d <= 60; d += 2) {
            pwm1_set_duty_percent(d);
            for (volatile uint32_t i=0; i<40000; i++) { } // crude delay
        }
        for (int d = 60; d >= 30; d -= 2) {
            pwm1_set_duty_percent((uint16_t)d);
            for (volatile uint32_t i=0; i<40000; i++) { }
        }
    }
}