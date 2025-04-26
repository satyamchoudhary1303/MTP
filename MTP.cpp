#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"

#define ENCODER_A 2
#define ENCODER_B 3
#define PULSES_PER_REV 1024
#define QUADRATURE_FACTOR 4

#define LCD_RS 6
#define LCD_EN 7
#define LCD_D4 8
#define LCD_D5 9
#define LCD_D6 10
#define LCD_D7 11

volatile int position = 0;

void lcd_toggle_enable() {
    sleep_us(1);
    gpio_put(LCD_EN, 1);
    sleep_us(1);
    gpio_put(LCD_EN, 0);
    sleep_us(100);
}

void lcd_send_nibble(uint8_t nibble) {
    gpio_put(LCD_D4, (nibble >> 0) & 1);
    gpio_put(LCD_D5, (nibble >> 1) & 1);
    gpio_put(LCD_D6, (nibble >> 2) & 1);
    gpio_put(LCD_D7, (nibble >> 3) & 1);
    lcd_toggle_enable();
}

void lcd_send_byte(uint8_t byte, bool is_data) {
    gpio_put(LCD_RS, is_data);
    lcd_send_nibble(byte >> 4);
    lcd_send_nibble(byte & 0x0F);
    sleep_ms(1);
}

void lcd_init() {
    gpio_init(LCD_RS); gpio_set_dir(LCD_RS, GPIO_OUT);
    gpio_init(LCD_EN); gpio_set_dir(LCD_EN, GPIO_OUT);
    gpio_init(LCD_D4); gpio_set_dir(LCD_D4, GPIO_OUT);
    gpio_init(LCD_D5); gpio_set_dir(LCD_D5, GPIO_OUT);
    gpio_init(LCD_D6); gpio_set_dir(LCD_D6, GPIO_OUT);
    gpio_init(LCD_D7); gpio_set_dir(LCD_D7, GPIO_OUT);

    sleep_ms(50);
    lcd_send_nibble(0x03); sleep_ms(5);
    lcd_send_nibble(0x03); sleep_ms(5);
    lcd_send_nibble(0x03); sleep_ms(5);
    lcd_send_nibble(0x02);

    lcd_send_byte(0x28, false);
    lcd_send_byte(0x0C, false);
    lcd_send_byte(0x06, false);
    lcd_send_byte(0x01, false);
    sleep_ms(2);
}

void lcd_set_cursor(uint8_t col, uint8_t row) {
    const uint8_t offsets[] = {0x00, 0x40};
    lcd_send_byte(0x80 | (col + offsets[row]), false);
}

void lcd_print(const char *str) {
    while (*str) {
        lcd_send_byte(*str++, true);
    }
}

void encoder_callback(uint gpio, uint32_t events) {
    int A = gpio_get(ENCODER_A);
    int B = gpio_get(ENCODER_B);

    if (gpio == ENCODER_A) {
        position += (A == B) ? 1 : -1;
    } else {
        position += (A != B) ? 1 : -1;
    }

    if (position < -PULSES_PER_REV * QUADRATURE_FACTOR / 2)
        position += PULSES_PER_REV * QUADRATURE_FACTOR;
    if (position > PULSES_PER_REV * QUADRATURE_FACTOR / 2)
        position -= PULSES_PER_REV * QUADRATURE_FACTOR;
}

int main() {
    stdio_init_all();
    lcd_init();

    gpio_init(ENCODER_A); gpio_set_dir(ENCODER_A, GPIO_IN); gpio_pull_up(ENCODER_A);
    gpio_init(ENCODER_B); gpio_set_dir(ENCODER_B, GPIO_IN); gpio_pull_up(ENCODER_B);

    gpio_set_irq_enabled_with_callback(ENCODER_A, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true, &encoder_callback);
    gpio_set_irq_enabled_with_callback(ENCODER_B, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true, &encoder_callback);

    while (true) {
        float angle = ((position / (float)(PULSES_PER_REV * QUADRATURE_FACTOR)) * 360.0);
        if (angle >= 180.0) angle -= 360.0;
        if (angle < -180.0) angle += 360.0;

        char msg[17];
        snprintf(msg, sizeof(msg), "Angle: %7.4f", angle);  // 4 decimal places
        printf("%s\n", msg);

        lcd_send_byte(0x01, false);  // Clear LCD
        lcd_set_cursor(0, 0);
        lcd_print(msg);

        sleep_ms(5);
    }
}
