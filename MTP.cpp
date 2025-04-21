#include <stdio.h>
#include <pico/stdlib.h>
#include <hardware/irq.h>
#include <hardware/gpio.h>

#define ENCODER_A 2  // Encoder Channel A
#define ENCODER_B 3  // Encoder Channel B
#define PULSES_PER_REV 1024  // Encoder resolution
#define QUADRATURE_FACTOR 4  // 4x decoding for better accuracy

volatile int position = 0;  // Encoder position count

void encoder_callback(uint gpio, uint32_t events) {
    int A = gpio_get(ENCODER_A);
    int B = gpio_get(ENCODER_B);

    if (gpio == ENCODER_A) {
        if (A == B) position++;  // Clockwise
        else position--;  // Counterclockwise
    } else if (gpio == ENCODER_B) {
        if (A != B) position++;  // Clockwise
        else position--;  // Counterclockwise
    }

    // Wrap position to keep angle in -180 to 180 range
    if (position < -PULSES_PER_REV * QUADRATURE_FACTOR / 2) {
        position += PULSES_PER_REV * QUADRATURE_FACTOR;
    }
    if (position > PULSES_PER_REV * QUADRATURE_FACTOR / 2) {
        position -= PULSES_PER_REV * QUADRATURE_FACTOR;
    }
}

int main() {
    stdio_init_all();

    gpio_init(ENCODER_A);
    gpio_init(ENCODER_B);
    
    gpio_set_dir(ENCODER_A, GPIO_IN);
    gpio_set_dir(ENCODER_B, GPIO_IN);

    gpio_pull_up(ENCODER_A);
    gpio_pull_up(ENCODER_B);

    // Enable interrupts for both A & B channels
    gpio_set_irq_enabled_with_callback(ENCODER_A, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true, &encoder_callback);
    gpio_set_irq_enabled_with_callback(ENCODER_B, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true, &encoder_callback);

    while (1) {
        float angle = ((position / (float)(PULSES_PER_REV * QUADRATURE_FACTOR)) * 360.0);

        // Convert to -180 to +180 range
        if (angle > 180.0) angle -= 360.0;
        if (angle < -180.0) angle += 360.0;

        printf("Angle: %.2f degrees\n", angle);
        sleep_ms(5);
    }
}
