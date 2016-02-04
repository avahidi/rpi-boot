
#include "defs.h"

#define LED_BIT 47
#define BLINK_DELAY (1UL << 22)

/*
 * GPIO
 */

#define _gpio ((volatile struct gpio *) GPIO_BASE)
#define GPIO_SEL_INPUT 0
#define GPIO_SEL_OUTPUT 1


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

/*
 * MAILBOX
 */

#define _mbox ((volatile uint32_t *) MBOX_BASE)

void mbox_write(int core, int box, uint32_t val)
{
    _mbox[core * 4 + box] = val;
}

uint32_t mbox_read(int core, int box)
{
    return _mbox[(4 + core) * 4 + box];
}

void mbox_clear(int core, int box)
{
    _mbox[(4 + core) * 4 + box] = -1;
}


/*
 *
 */

void delay(uint32_t n)
{
    while(n--)
        __asm__ volatile("nop");
}



void main()
{
    /* if this is the first CPU, just wake up the second CPU and return */
    if(__cpuid() == 0) {
        mbox_write(1, 3, (uint32_t) &__reset);
        __sev();
        return;
    }

    /* second CPU will do the blinking ... */
    gpio_sel(LED_BIT, GPIO_SEL_OUTPUT);
    for(;;) {
        gpio_set(LED_BIT, 0);
        delay(BLINK_DELAY);

        gpio_set(LED_BIT, 1);
        delay(BLINK_DELAY);
    }
}
