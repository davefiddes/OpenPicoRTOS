#include "gpio-sio.h"
#include "picoRTOS.h"

struct GPIO_SIO {
    volatile uint32_t GPIO_IN;
    volatile uint32_t GPIO_HI_IN;
    uint32_t RESERVED0;
    volatile uint32_t GPIO_OUT;
    volatile uint32_t GPIO_OUT_SET;
    volatile uint32_t GPIO_OUT_CLR;
    volatile uint32_t GPIO_OUT_XOR;
    volatile uint32_t GPIO_OE;
    volatile uint32_t GPIO_OE_SET;
    volatile uint32_t GPIO_OE_CLR;
    volatile uint32_t GPIO_OE_XOR;
    volatile uint32_t GPIO_HI_OUT;
    volatile uint32_t GPIO_HI_OUT_SET;
    volatile uint32_t GPIO_HI_OUT_CLR;
    volatile uint32_t GPIO_HI_OUT_XOR;
    volatile uint32_t GPIO_HI_OE;
    volatile uint32_t GPIO_HI_OE_SET;
    volatile uint32_t GPIO_HI_OE_CLR;
    volatile uint32_t GPIO_HI_OE_XOR;
};

/* Function: gpio_sio_init
 * Initializes a GPIO
 *
 * Parameters:
 *  ctx - The GPIO to init
 *  base - The GPIO block base address
 *  pin - The GPIO pin number on that block
 *  dir - The GPIO direction
 *
 * Returns:
 * 0 if success, -errno otherwise
 */
int gpio_sio_init(struct gpio *ctx, struct GPIO_SIO *base, size_t pin, gpio_sio_dir_t dir)
{
    if (!picoRTOS_assert(pin < (size_t)GPIO_SIO_PIN_COUNT)) return -EINVAL;
    if (!picoRTOS_assert(dir < GPIO_SIO_DIR_COUNT)) return -EINVAL;

    ctx->base = base;
    ctx->mask = (uint32_t)(1 << pin);

    if (dir == GPIO_SIO_DIR_OUTPUT)
        ctx->base->GPIO_OE_SET = ctx->mask;
    else
        ctx->base->GPIO_OE_CLR = ctx->mask;

    return 0;
}

void gpio_write(struct gpio *ctx, bool value)
{
    if (value) ctx->base->GPIO_OUT_SET = ctx->mask;
    else ctx->base->GPIO_OUT_CLR = ctx->mask;
}

bool gpio_read(struct gpio *ctx)
{
    return (ctx->base->GPIO_IN & ctx->mask) != 0;
}

void gpio_toggle(struct gpio *ctx)
{
    ctx->base->GPIO_OUT_XOR = ctx->mask;
}
