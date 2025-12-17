
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
static inline void pwm0_init_100kHz(void)
{
    DDRD |= (1 << PD5); // PD5 = OC0B

    // Fast PWM, TOP = OCR0A (Mode 7)
    TCCR0A = (1 << COM0B1) | (1 << WGM01) | (1 << WGM00);
    TCCR0B = (1 << WGM02)  | (1 << CS00);   // prescaler=1

    OCR0A = 79;   // TOP -> 100 kHz at 8 MHz
    OCR0B = 40;   // duty (0..79)
}

// duty in percent: 0..100
static inline void pwm0_set_duty_percent(uint8_t percent)
{
    if (percent > 100) percent = 100;

    uint16_t top = OCR0A;                    // 79
    uint16_t val = ((uint32_t)top * percent) / 100;

    OCR0B = (uint8_t)val;                    // set duty on OC0B
}

int main(void)
{
    pwm0_init_100kHz();

    while (1)
    {
        // simple sweep: 10% -> 90% -> 10%
        for (uint16_t d = 0; d <= 100; d += 2) {
            pwm0_set_duty_percent(d);
            for (volatile uint32_t i=0; i<40000; i++) { } // crude delay
        }
        for (int d = 100; d >= 0; d -= 2) {
            pwm0_set_duty_percent((uint16_t)d);
            for (volatile uint32_t i=0; i<40000; i++) { }
        }
        //pwm1_set_duty_percent(70);
    }
}