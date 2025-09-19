#define F_CPU 1000000UL

#include <avr/io.h>
#include <util/delay.h>

int main(void) {
    // Set PB5 (digital pin 13) as output
    DDRB |= (1 << DDB5);

    while (1) {
        // Toggle LED
        PORTB ^= (1 << PORTB5);
        
        // Delay ~500 ms
        _delay_ms(500);
    }

    return 0;
}
