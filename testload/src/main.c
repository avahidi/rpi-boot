
#include "defs.h"

#define _gpio ((volatile struct gpio *) GPIO_BASE)
#define GPIO_SEL_INPUT 0
#define GPIO_SEL_OUTPUT 1

#define LED_BIT 47
#define BLINK_DELAY (1UL << 22)

struct gpio {
    uint32_t sel[6];
    uint32_t unused0;
    uint32_t set[2];
    uint32_t unused1;
    uint32_t clr[2];
    /* ... */
};


void gpio_sel(int pin, int func)
{
    const int index = pin / 10;
    /* const int shift = (pin % 10) * 3; */
    const int shift = (pin - index * 10) * 3;

    _gpio->sel[index] = (func << shift) | (_gpio->sel[index] & ~(7 << shift));
}


void gpio_set(int pin, int val)
{
    const int index = pin >> 5;
    const int shift = pin & 31;

    if(val)
        _gpio->set[index] = 1UL << shift;
    else
        _gpio->clr[index] = 1UL << shift;
}



void delay(uint32_t n)
{
    while(n--)
        __asm__ volatile("nop");
}



void main()
{
    uint32_t dtime = BLINK_DELAY / 8;

    gpio_sel(LED_BIT, GPIO_SEL_OUTPUT);

    for(;;) {
        gpio_set(LED_BIT, 0);
        delay(BLINK_DELAY);

        gpio_set(LED_BIT, 1);
        delay(BLINK_DELAY);
    }
}
