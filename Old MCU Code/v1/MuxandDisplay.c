#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include "font.h"

#include <stdlib.h> //String formatting stuff

#define F_CPU 8000000UL

#define CS   PB2
#define DC   PB1
#define RST  PB0

#define CS_LOW()   (PORTB &= ~(1<<CS))
#define CS_HIGH()  (PORTB |=  (1<<CS))
#define DC_LOW()   (PORTB &= ~(1<<DC))
#define DC_HIGH()  (PORTB |=  (1<<DC))
#define RST_LOW()  (PORTB &= ~(1<<RST))
#define RST_HIGH() (PORTB |=  (1<<RST))

float measurements[8] = {
    0,
    1,
    2,
    3,
    4,
    5,
    6,
    7
};
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

void init(){
    DDRD |= (1 << PD0) | (1 << PD1) | (1 << PD2); //Designate as output
    PORTD |= (0 << PD0) | (0 << PD1) | (0 << PD2); //Status of output pin
}

void adc_init(void)
{
    // Reference = AVcc (5V)
    ADMUX = (1 << REFS0);

    // ADC Enable + Prescaler 128 -> 16MHz/128 = 125kHz
    ADCSRA = (1 << ADEN) | (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0);
}

float adc_read(uint8_t channel)
{
    // Select ADC channel (0–7)
    ADMUX = (ADMUX & 0xF0) | (channel & 0x0F);

    // Start conversion
    ADCSRA |= (1 << ADSC);

    // Wait for conversion to finish
    while (ADCSRA & (1 << ADSC));

    // Read result
    float raw = ADC;
    return (5.0 / 1023.0) * raw;
}

void mux_measure(){
    for(int i = 0; i < 4; i++){
        PORTD = truth_table[i];
        _delay_ms(3000 * 8);

        if(i < 4){
            measurements[i] = adc_read(0); //Strom matning
        }
        else if(i > 4){
            measurements[i] = adc_read(1); //Spanningsmatning
        }
    }
}


/*
Display pins
LED = 5V
SCK = PB5
SDA = PB3
A0 = PB1
RESET = PB0
CS = PB2
GND = GND
VCC = 5V
*/
int main(void)
{

    DDRB |= (1<<DC)|(1<<RST); // Outputs for DC, RST
    SPI_init();
    ST7735_init();
    adc_init();

    for(uint8_t y=0; y<160; y++)
        for(uint8_t x=0; x<128; x++)
            drawPixel(x,y,BLACK);

    drawString(10, 20, "Init complete", WHITE, BLACK);

    while (1)
    {
    mux_measure();        
    for(uint8_t y=0; y<160; y++)
        for(uint8_t x=0; x<128; x++)
            drawPixel(x,y,BLACK);

    for (int i = 0; i < 4; i++){
        char buffer[10];
        dtostrf(measurements[i], 4, 2, buffer);
        drawString(10, 20+(10*i), buffer, WHITE, BLACK);
    }

    }
}
