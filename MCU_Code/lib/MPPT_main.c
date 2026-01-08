
#define F_CPU 8000000UL

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <p_func.h>
#include <font.h>


//#define PERIOD    20000   // 20 ms total period in µs
#define MIN_PULSE 1500    // 1.5 ms
#define MAX_PULSE 2200    // 2.2 ms
#define V_CONV 20.0f/1023.0f
#define I_CONV (5.0f/1023.0f*0.202f/4.56f)
#define CS   PB2
#define DC   PB1
#define RST  PB0

#define CS_LOW()   (PORTB &= ~(1<<CS))
#define CS_HIGH()  (PORTB |=  (1<<CS))
#define DC_LOW()   (PORTB &= ~(1<<DC))
#define DC_HIGH()  (PORTB |=  (1<<DC))
#define RST_LOW()  (PORTB &= ~(1<<RST))
#define RST_HIGH() (PORTB |=  (1<<RST))


volatile uint16_t pulsewidth = 50; // current high pulse width
volatile uint8_t phase = 0;           // 0 = high, 1 = low
volatile uint16_t PERIOD = 80;// * 1.01;
volatile float V = 0.0;
volatile float P = 0.0;
volatile float I = 0.0;
volatile float V_old = 0.0;
volatile float P_old = 0.0;
volatile float I_old = 0.0;


void SPI_init() {
    DDRB |= (1<<PB3)|(1<<PB5)|(1<<CS); // MOSI, SCK, CS
    SPCR = (1<<SPE)|(1<<MSTR)|(1<<SPR0); // SPI enable, master, fosc/16
}

void SPI_send(uint8_t d) {
    SPDR = d;
    while (!(SPSR & (1<<SPIF)));
}

// -------------------------------------------------------
// TFT low-level
// -------------------------------------------------------
void cmd(uint8_t c) {
    DC_LOW();
    CS_LOW();
    SPI_send(c);
    CS_HIGH();
}

void data(uint8_t d) {
    DC_HIGH();
    CS_LOW();
    SPI_send(d);
    CS_HIGH();
}

// -------------------------------------------------------
// ADDRESS WINDOW — Red tab offsets: X+2, Y+1
// -------------------------------------------------------
void setAddr(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1) {
    cmd(0x2A); 
    data(0x00); data(x0 + 0);
    data(0x00); data(x1 + 0);

    cmd(0x2B);
    data(0x00); data(y0 + 0);
    data(0x00); data(y1 + 0);

    cmd(0x2C);
}

void drawPixel(uint8_t x, uint8_t y, uint16_t color) {
    setAddr(x, y, x, y);
    data(color >> 8);
    data(color & 0xFF);
}

void fillScreen(uint16_t color)
{
    uint32_t pixels = 128UL * 160UL;

    // Set full screen window
    cmd(0x2A);            // Column addr set
    data(0x00); data(0);
    data(0x00); data(127);

    cmd(0x2B);            // Row addr set
    data(0x00); data(0);
    data(0x00); data(159);

    cmd(0x2C);            // Memory write

    DC_HIGH();
    CS_LOW();

    uint8_t hi = color >> 8;
    uint8_t lo = color & 0xFF;

    while (pixels--) {
        SPI_send(hi);
        SPI_send(lo);
    }

    CS_HIGH();
}

// -------------------------------------------------------
// ST7735 RED TAB initialization (100% correct)
// -------------------------------------------------------
void ST7735_init() {
    RST_LOW();
    _delay_ms(50);
    RST_HIGH();
    _delay_ms(150);

    cmd(0x01); _delay_ms(150); // SW reset
    cmd(0x11); _delay_ms(150); // Sleep out

    cmd(0x3A); data(0x05);     // 16-bit color
    cmd(0x36); data(0xC8);     // MADCTL: row/col order for red tab

    // Frame rate control
    cmd(0xB1); data(0x01); data(0x2C); data(0x2D);
    cmd(0xB2); data(0x01); data(0x2C); data(0x2D);
    cmd(0xB3); data(0x01); data(0x2C); data(0x2D); data(0x01); data(0x2C); data(0x2D);

    cmd(0xB4); data(0x07);      // Inversion control

    // Power sequence
    cmd(0xC0); data(0xA2); data(0x02); data(0x84);
    cmd(0xC1); data(0xC5);
    cmd(0xC2); data(0x0A); data(0x00);
    cmd(0xC3); data(0x8A); data(0x2A);
    cmd(0xC4); data(0x8A); data(0xEE);
    cmd(0xC5); data(0x0E);

    // Gamma correction
    cmd(0xE0);
    data(0x02); data(0x1C); data(0x07); data(0x12); data(0x37); data(0x32);
    data(0x29); data(0x2D); data(0x29); data(0x25); data(0x2B); data(0x39);
    data(0x00); data(0x01); data(0x03); data(0x10);

    cmd(0xE1);
    data(0x03); data(0x1D); data(0x07); data(0x06); data(0x2E); data(0x2C);
    data(0x29); data(0x2D); data(0x2E); data(0x2E); data(0x37); data(0x3F);
    data(0x00); data(0x00); data(0x02); data(0x10);

    cmd(0x13); // Normal display mode
    cmd(0x29); // Display on
}

// -------------------------------------------------------
// TEXT DRAWING
// -------------------------------------------------------
void drawChar(uint8_t x, uint8_t y, char ch, uint16_t color, uint16_t bg) {
    if (ch < 32 || ch > 126) return;
    const uint8_t *bitmap = font5x7[ch - 32];

    for (uint8_t col = 0; col < 5; col++) {
        uint8_t line = bitmap[col];
        for (uint8_t row = 0; row < 7; row++) {
            if (line & 0x01)
                drawPixel(x + col, y + row, color);
            else
                drawPixel(x + col, y + row, bg);
            line >>= 1;
        }
    }
}

void drawString(uint8_t x, uint8_t y, const char *s, uint16_t color, uint16_t bg) {
    while (*s) {
        drawChar(x, y, *s, color, bg);
        x += 6;
        s++;
    }
}

// -------------------------------------------------------
// COLORS (RGB565)
// -------------------------------------------------------
#define RED     0xF800
#define GREEN   0x07E0
#define BLUE    0x001F
#define WHITE   0xFFFF
#define BLACK   0x0000
#define YELLOW  0xFFE0












void adc_init(void)
{
    // Reference = AVcc (5V)
    ADMUX = (1 << REFS0);

    // ADC Enable + Prescaler 128 -> 16MHz/128 = 125kHz
    ADCSRA = (1 << ADEN) | (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0);
}


void adc_read()
{
    uint16_t adc_raw;
    P_old = P;
    I_old = I;
    V_old = V;    
    // Select ADC channel (0–7)
    ADMUX = (ADMUX & 0xF0) | (2 & 0x0F);
    // Start conversion
    ADCSRA |= (1 << ADSC);
    // Wait for conversion to finish
    while (ADCSRA & (1 << ADSC));
    // Read result
    adc_raw = ADC;
    V=V_CONV * (float)adc_raw; //voltage in volts


    ADMUX = (ADMUX & 0xF0) | (1 & 0x0F);
    // Start conversion
    ADCSRA |= (1 << ADSC);
    // Wait for conversion to finish
    while (ADCSRA & (1 << ADSC));
    // Read result
    adc_raw = ADC;
    I=I_CONV * (float)adc_raw; //current
    P=V*I; 
}


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
    DDRB |= (1<<DC)|(1<<RST); 
    sei();
    pwm0_init_100kHz();
    adc_init();
    SPI_init();
    ST7735_init();
    fillScreen(BLACK);
    drawString(10, 20, "Init complete", WHITE, BLACK);
    uint16_t i= 0;

    while (1)
    {
        // simple sweep: 10% -> 90% -> 10%
        /*
        for (uint16_t d = 60; d <= 20; d += 2) {
            pwm0_set_duty_percent(d);
            for (volatile uint32_t i=0; i<40000; i++) { } // crude delay
        }
        for (int d = 80; d >= 20; d -= 2) {
            pwm0_set_duty_percent((uint16_t)d);
            for (volatile uint32_t i=0; i<40000; i++) { }
        }
        */   
        
        adc_read();
        pulsewidth = MPPT_pulse(P_old, P, I_old, I, V_old, V, pulsewidth);
        pwm0_set_duty_percent(pulsewidth);
        _delay_ms(20*8);
        //pwm1_set_duty_percent(70);


        if(i==10){
            fillScreen(BLACK);
            /*       
            for(uint8_t y=0; y<160; y++)
                for(uint8_t x=0; x<128; x++)
                    drawPixel(x,y,BLACK);
            */
            char buffer[10];
            dtostrf(P, 8, 6, buffer);
            drawString(10, 20+(10*1), buffer, WHITE, BLACK);
            dtostrf(I, 8, 6, buffer);
            drawString(10, 20+(10*2), buffer, WHITE, BLACK);
            dtostrf(V, 8, 5, buffer);
            drawString(10, 20+(10*3), buffer, WHITE, BLACK);
            i=0;
        }
        i++;
    }
}