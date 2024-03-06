#ifndef CLOCK_STM32F10XXX_H
#define CLOCK_STM32F10XXX_H

#include "clock.h"

#define CLOCK_STM32F10XXX_SYSCLK   CLOCK_SYSCLK
#define CLOCK_STM32F10XXX_FLITFCLK (clock_id_t)1
#define CLOCK_STM32F10XXX_USBCLK   (clock_id_t)2
#define CLOCK_STM32F10XXX_I2S3CLK  (clock_id_t)3
#define CLOCK_STM32F10XXX_I2S2CLK  (clock_id_t)4
#define CLOCK_STM32F10XXX_SDIOCLK  (clock_id_t)5
#define CLOCK_STM32F10XXX_FSMCCLK  (clock_id_t)6
#define CLOCK_STM32F10XXX_HCLK     (clock_id_t)7
#define CLOCK_STM32F10XXX_FCLK     (clock_id_t)8
#define CLOCK_STM32F10XXX_PCLK1    (clock_id_t)9
#define CLOCK_STM32F10XXX_TIMXCLK  (clock_id_t)10   /* 2-7, 12-14 */
#define CLOCK_STM32F10XXX_PCLK2    (clock_id_t)11
#define CLOCK_STM32F10XXX_TIMxCLK  (clock_id_t)12   /* 1, 8-11 */
#define CLOCK_STM32F10XXX_ADCCLK   (clock_id_t)13
#define CLOCK_STM32F10XXX_RTCCLK   (clock_id_t)14
#define CLOCK_STM32F10XXX_IWDGCLK  (clock_id_t)15

typedef enum {
    CLOCK_STM32F10XXX_SYSCLK_HSI    = 0,
    CLOCK_STM32F10XXX_SYSCLK_HSE    = 1,
    CLOCK_STM32F10XXX_SYSCLK_PLL    = 2,
    CLOCK_STM32F10XXX_SYSCLK_COUNT,
} clock_stm32f10xxx_sysclk_t;

typedef enum {
    CLOCK_STM32F10XXX_PLLSRC_HSI_DIV2   = 0,
    CLOCK_STM32F10XXX_PLLSRC_HSE        = 1,
    CLOCK_STM32F10XXX_PLLSRC_HSE_DIV2   = 2,
    CLOCK_STM32F10XXX_PLLSRC_COUNT,
} clock_stm32f10xxx_pllsrc_t;

struct clock_settings {
    /* clock sources */
    clock_stm32f10xxx_sysclk_t sysclk;
    clock_stm32f10xxx_pllsrc_t pllsrc;
    /* constants */
    unsigned long hse;  /* 3-25 HSE OSC. 0: disable */
    unsigned long pll;  /* request pll freq. 0: disable */
    /* prescalers */
    unsigned ahb_prescaler;
    unsigned apb1_prescaler;
    unsigned apb2_prescaler;
    unsigned adc_prescaler;
};

int clock_stm32f10xxx_init(struct clock_settings *settings);

typedef enum {
    /* AHB */
    CLOCK_STM32F10XXX_CLK_DMA1      = 0x0,
    CLOCK_STM32F10XXX_CLK_DMA2      = 0x1,
    CLOCK_STM32F10XXX_CLK_SRAM      = 0x2,
    CLOCK_STM32F10XXX_CLK_FLTIF     = 0x4,
    CLOCK_STM32F10XXX_CLK_CRC       = 0x6,
    CLOCK_STM32F10XXX_CLK_FSMC      = 0x8,
    CLOCK_STM32F10XXX_CLK_SDIO      = 0x10,
    /* APB2 */
    CLOCK_STM32F10XXX_CLK_AFIO      = 0x20,
    CLOCK_STM32F10XXX_CLK_IOPA      = 0x22,
    CLOCK_STM32F10XXX_CLK_IOPB      = 0x23,
    CLOCK_STM32F10XXX_CLK_IOPC      = 0x24,
    CLOCK_STM32F10XXX_CLK_IOPD      = 0x25,
    CLOCK_STM32F10XXX_CLK_IOPE      = 0x26,
    CLOCK_STM32F10XXX_CLK_IOPF      = 0x27,
    CLOCK_STM32F10XXX_CLK_IOPG      = 0x28,
    CLOCK_STM32F10XXX_CLK_ADC1      = 0x29,
    CLOCK_STM32F10XXX_CLK_ADC2      = 0x2a,
    CLOCK_STM32F10XXX_CLK_TIM1      = 0x2b,
    CLOCK_STM32F10XXX_CLK_SPI1      = 0x2c,
    CLOCK_STM32F10XXX_CLK_TIM8      = 0x2d,
    CLOCK_STM32F10XXX_CLK_USART1    = 0x2e,
    CLOCK_STM32F10XXX_CLK_ADC3      = 0x2f,
    CLOCK_STM32F10XXX_CLK_TIM9      = 0x33,
    CLOCK_STM32F10XXX_CLK_TIM10     = 0x34,
    CLOCK_STM32F10XXX_CLK_TIM11     = 0x35,
    /* APB1 */
    CLOCK_STM32F10XXX_CLK_TIM2      = 0x40,
    CLOCK_STM32F10XXX_CLK_TIM3      = 0x41,
    CLOCK_STM32F10XXX_CLK_TIM4      = 0x42,
    CLOCK_STM32F10XXX_CLK_TIM5      = 0x43,
    CLOCK_STM32F10XXX_CLK_TIM6      = 0x44,
    CLOCK_STM32F10XXX_CLK_TIM7      = 0x45,
    CLOCK_STM32F10XXX_CLK_TIM12     = 0x46,
    CLOCK_STM32F10XXX_CLK_TIM13     = 0x47,
    CLOCK_STM32F10XXX_CLK_TIM14     = 0x48,
    CLOCK_STM32F10XXX_CLK_WWDG      = 0x4b,
    CLOCK_STM32F10XXX_CLK_SPI2      = 0x4e,
    CLOCK_STM32F10XXX_CLK_SPI3      = 0x4f,
    CLOCK_STM32F10XXX_CLK_UXART2    = 0x51,
    CLOCK_STM32F10XXX_CLK_USART3    = 0x52,
    CLOCK_STM32F10XXX_CLK_UART4     = 0x53,
    CLOCK_STM32F10XXX_CLK_UART5     = 0x54,
    CLOCK_STM32F10XXX_CLK_I2C1      = 0x55,
    CLOCK_STM32F10XXX_CLK_I2C2      = 0x56,
    CLOCK_STM32F10XXX_CLK_USB       = 0x57,
    CLOCK_STM32F10XXX_CLK_CAN       = 0x59,
    CLOCK_STM32F10XXX_CLK_BKP       = 0x5b,
    CLOCK_STM32F10XXX_CLK_PWR       = 0x5c,
    CLOCK_STM32F10XXX_CLK_DAC       = 0x5d,
    /* The End */
    CLOCK_STM32F10XXX_CLK_COUNT,
} clock_stm32f10xxx_clk_t;

int clock_stm32f10xxx_enable(clock_stm32f10xxx_clk_t clk);
int clock_stm32f10xxx_disable(clock_stm32f10xxx_clk_t clk);

/* Implements:
 * clock_freq_t clock_get_freq(clock_id_t clkid);
 */

#endif
