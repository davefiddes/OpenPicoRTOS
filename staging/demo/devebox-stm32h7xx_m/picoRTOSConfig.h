#ifndef PICORTOSCONFIG_H
#define PICORTOSCONFIG_H

/* CLOCKS */
// #define CONFIG_SYSCLK_HZ 16000000
// #define CONFIG_SYSCLK_HZ 25000000
// #define CONFIG_SYSCLK_HZ 128000000
#define CONFIG_SYSCLK_HZ 200000000
// #define CONFIG_SYSCLK_HZ 400000000
#define CONFIG_TICK_HZ   1000

/* TASKS */
#define CONFIG_TASK_COUNT 6

/* STACK */
#define CONFIG_DEFAULT_STACK_COUNT 128

/* DEADLOCK */
#define CONFIG_DEADLOCK_COUNT      CONFIG_TICK_HZ

#endif
