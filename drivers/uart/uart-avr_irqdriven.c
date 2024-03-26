#include "uart-avr_irqdriven.h"

#include "picoRTOS.h"
#include "picoRTOS_device.h"

#include <stdint.h>

struct USART_AVR {
    volatile uint8_t UCSRnA;
    volatile uint8_t UCSRnB;
    volatile uint8_t UCSRnC;
    volatile uint8_t RESERVED;
    volatile uint16_t UBRRn;
    volatile uint8_t UDRn;
};

#define UCSRnA_RXC  (1 << 7)
#define UCSRnA_TXC  (1 << 6)
#define UCSRnA_UDRE (1 << 5)
#define UCSRnA_FE   (1 << 4)
#define UCSRnA_DOR  (1 << 3)
#define UCSRnA_UPE  (1 << 2)
#define UCSRnA_U2X  (1 << 1)
#define UCSRnA_MPCM (1 << 0)

#define UCSRnB_RXCIE (1 << 7)
#define UCSRnB_TXCIE (1 << 6)
#define UCSRnB_UDRIE (1 << 5)
#define UCSRnB_RXEN  (1 << 4)
#define UCSRnB_TXEN  (1 << 3)
#define UCSRnB_UCSZ2 (1 << 2)
#define UCSRnB_RXD8  (1 << 1)
#define UCSRnB_TXD8  (1 << 0)

#define UCSRnC_UMSEL_M  0x3u
#define UCSRnC_UMSEL(x) (((x) & UCSRnC_UMSEL_M) << 6)
#define UCSRnC_UPM_M    0x3u
#define UCSRnC_UPM(x)   (((x) & UCSRnC_UPM_M) << 4)
#define UCSRnC_USBS     (1 << 3)
#define UCSRnC_UCSZ_M   0x3u
#define UCSRnC_UCSZ(x)  (((x) & UCSRnC_UCSZ_M) << 1)
#define UCSRnC_UCPOL    (1 << 0)

/* UART */

static void uart_rx_isr(void *priv)
{
    int deadlock = CONFIG_DEADLOCK_COUNT;
    struct uart *ctx = (struct uart*)priv;

    picoRTOS_assert(ctx->rx_buf != NULL, return );

    while ((ctx->base->UCSRnA & UCSRnA_RXC) != 0 &&
           deadlock-- != 0) {
        /* store data if possible */
        if (fifo_head_is_writable(&ctx->rx_fifo)) {
            ctx->rx_buf[ctx->rx_fifo.w] = (char)ctx->base->UDRn;
            fifo_head_push(&ctx->rx_fifo);
        }else{ /* drop */
            /*@i@*/ (void)ctx->base->UDRn;
            break;
        }
    }

    /* check */
    picoRTOS_assert_void(deadlock != -1);
}

static void uart_udre_isr(void *priv)
{
    struct uart *ctx = (struct uart*)priv;

    picoRTOS_assert(ctx->tx_buf != NULL, return );

    do{
        if (fifo_head_is_readable(&ctx->tx_fifo)) {
            fifo_head_pop(&ctx->tx_fifo);
            ctx->base->UDRn = (uint8_t)ctx->tx_buf[ctx->tx_fifo.r];
        }else{
            /* disable */
            ctx->base->UCSRnB &= ~UCSRnB_UDRIE;
            break;
        }
    } while((ctx->base->UCSRnA & UCSRnA_UDRE) != 0);
}

int uart_avr_init(struct uart *ctx, int base, clock_id_t clkid)
{
    ctx->base = (struct USART_AVR*)base;
    ctx->clkid = clkid;
    ctx->mode = UART_AVR_MODE_NORMAL;
    ctx->rx_buf = NULL;
    ctx->tx_buf = NULL;

    /* turn on */
    ctx->base->UCSRnA = (uint8_t)UCSRnA_U2X;
    ctx->base->UCSRnB = (uint8_t)(UCSRnB_TXEN | UCSRnB_RXEN);

    return 0;
}

/* Function: uart_avr_setup_irqdriven
 * Activates IRQ-driven options for the UART
 *
 * Parameters:
 *  ctx - The UART to setup
 *  settings - The UART IRQ setup parameters (see .h)
 *
 * Returns:
 * -errno if errno, 0 otherwise
 */
int uart_avr_setup_irqdriven(struct uart *ctx, struct uart_avr_irqdriven_settings *settings)
{
    picoRTOS_assert(settings->irq_rx <= (picoRTOS_irq_t)DEVICE_INTERRUPT_VECTOR_COUNT, return -EINVAL);
    picoRTOS_assert(settings->irq_udre <= (picoRTOS_irq_t)DEVICE_INTERRUPT_VECTOR_COUNT, return -EINVAL);
    picoRTOS_assert((settings->rx_mask & (settings->rx_mask + 1)) == 0, return -EINVAL);
    picoRTOS_assert((settings->tx_mask & (settings->tx_mask + 1)) == 0, return -EINVAL);

    fifo_head_init(&ctx->rx_fifo, settings->rx_mask);
    fifo_head_init(&ctx->tx_fifo, settings->tx_mask);
    ctx->rx_buf = settings->rx_buf;
    ctx->tx_buf = settings->tx_buf;

    /* register interrupts */
    picoRTOS_register_interrupt(settings->irq_rx, uart_rx_isr, ctx);
    picoRTOS_register_interrupt(settings->irq_udre, uart_udre_isr, ctx);

    ctx->base->UCSRnB |= UCSRnB_RXCIE;
    ctx->mode = UART_AVR_MODE_IRQDRIVEN;

    return 0;
}

static int set_baudrate(struct uart *ctx, unsigned long baud)
{
    picoRTOS_assert(baud > 0, return -EINVAL);

    clock_freq_t freq = clock_get_freq(ctx->clkid);
    unsigned long ubrr = (unsigned long)freq / 8ul / baud - 1;

    picoRTOS_assert(freq > 0, return -EIO);

    ctx->base->UBRRn = (uint16_t)ubrr;

    /* TODO: check freq match */
    return 0;
}

static int set_cs(struct uart *ctx, size_t cs)
{
    picoRTOS_assert(cs >= (size_t)UART_AVR_CS_MIN, return -EINVAL);
    picoRTOS_assert(cs <= (size_t)UART_AVR_CS_MAX, return -EINVAL);

    /* no support for 9bit yet */
    ctx->base->UCSRnB &= ~UCSRnB_UCSZ2;
    ctx->base->UCSRnC &= ~UCSRnC_UCSZ(UCSRnC_UCSZ_M);

    switch (cs) {
    case 5: break;
    case 6: ctx->base->UCSRnC |= UCSRnC_UCSZ(1); break;
    case 7: ctx->base->UCSRnC |= UCSRnC_UCSZ(2); break;
    case 8: ctx->base->UCSRnC |= UCSRnC_UCSZ(3); break;
    default:
        picoRTOS_break();
        /*@notreached@*/
        return -EIO;
    }

    return 0;
}

static int set_parity(struct uart *ctx, uart_par_t par)
{
    picoRTOS_assert(par != UART_PAR_IGNORE, return -EINVAL);
    picoRTOS_assert(par < UART_PAR_COUNT, return -EINVAL);

    /* disable */
    ctx->base->UCSRnC &= ~UCSRnC_UPM(UCSRnC_UPM_M);
    if (par == UART_PAR_NONE)
        return 0;

    if (par == UART_PAR_ODD) ctx->base->UCSRnC |= UCSRnC_UPM(3);
    else ctx->base->UCSRnC |= UCSRnC_UPM(2);

    return 0;
}

static int set_cstopb(struct uart *ctx, uart_cstopb_t cstopb)
{
    picoRTOS_assert(cstopb != UART_CSTOPB_IGNORE, return -EINVAL);
    picoRTOS_assert(cstopb < UART_CSTOPB_COUNT, return -EINVAL);

    if (cstopb == UART_CSTOPB_2BIT) ctx->base->UCSRnC |= UCSRnC_USBS;
    else ctx->base->UCSRnC &= ~UCSRnC_USBS;

    return 0;
}

/* HAL */

int uart_setup(struct uart *ctx, const struct uart_settings *settings)
{
    int res;

    /* baudrate */
    if (settings->baudrate != 0 &&
        (res = set_baudrate(ctx, settings->baudrate)) < 0)
        return res;

    /* cs */
    if (settings->cs != 0 &&
        (res = set_cs(ctx, settings->cs)) < 0)
        return res;

    /* parity */
    if (settings->par != UART_PAR_IGNORE &&
        (res = set_parity(ctx, settings->par)) < 0)
        return res;

    /* cstopb */
    if (settings->cstopb != UART_CSTOPB_IGNORE &&
        (res = set_cstopb(ctx, settings->cstopb)) < 0)
        return res;

    return 0;
}

static int uart_write_irqdriven(struct uart *ctx, const char *buf, size_t n)
{
    picoRTOS_assert(n > 0, return -EINVAL);
    picoRTOS_assert(ctx->tx_buf != NULL, return -EIO);

    int sent = 0;

    while ((size_t)sent != n) {
        if (fifo_head_is_writable(&ctx->tx_fifo)) {
            ctx->tx_buf[ctx->tx_fifo.w] = buf[sent++];
            fifo_head_push(&ctx->tx_fifo);
        }else
            break;
    }

    if (sent == 0)
        return -EAGAIN;

    /* trigger udre isr */
    ctx->base->UCSRnB |= UCSRnB_UDRIE;
    return sent;
}

static int uart_read_irqdriven(struct uart *ctx, char *buf, size_t n)
{
    picoRTOS_assert(n > 0, return -EINVAL);
    picoRTOS_assert(ctx->rx_buf != NULL, return -EIO);

    int recv = 0;

    while ((size_t)recv != n) {
        if (fifo_head_is_readable(&ctx->rx_fifo)) {
            fifo_head_pop(&ctx->rx_fifo);
            buf[recv++] = ctx->rx_buf[ctx->rx_fifo.r];
        }else
            break;
    }

    if (recv == 0)
        return -EAGAIN;

    return recv;
}

static int uart_write_normal(struct uart *ctx, const char *buf, size_t n)
{
    picoRTOS_assert(n > 0, return -EINVAL);

    size_t nwritten = 0;

    while (nwritten != n) {

        if ((ctx->base->UCSRnA & UCSRnA_UDRE) == (uint8_t)0)
            break;

        ctx->base->UDRn = (uint8_t)*buf++;
        nwritten++;
    }

    if (nwritten == 0)
        return -EAGAIN;

    return (int)nwritten;
}

static int uart_read_normal(struct uart *ctx, char *buf, size_t n)
{
    picoRTOS_assert(n > 0, return -EINVAL);

    size_t nread = 0;

    while (nread != n) {

        if ((ctx->base->UCSRnA & UCSRnA_RXC) == 0)
            break;

        *buf++ = (char)ctx->base->UDRn;
        nread++;
    }

    if (nread == 0)
        return -EAGAIN;

    return (int)nread;
}

int uart_write(struct uart *ctx, const char *buf, size_t n)
{
    switch (ctx->mode) {
    case UART_AVR_MODE_NORMAL: return uart_write_normal(ctx, buf, n);
    case UART_AVR_MODE_IRQDRIVEN: return uart_write_irqdriven(ctx, buf, n);
    default: break;
    }

    picoRTOS_break();
    /*@notreached@*/ return -EIO;
}

int uart_read(struct uart *ctx, char *buf, size_t n)
{
    switch (ctx->mode) {
    case UART_AVR_MODE_NORMAL: return uart_read_normal(ctx, buf, n);
    case UART_AVR_MODE_IRQDRIVEN: return uart_read_irqdriven(ctx, buf, n);
    default: break;
    }

    picoRTOS_break();
    /*@notreached@*/ return -EIO;
}
