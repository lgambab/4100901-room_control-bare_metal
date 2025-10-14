#include "rcc.h"
#define RCC_APB2ENR_SYSCFGEN_Pos    (0U)
#define RCC_APB2ENR_SYSCFGEN_Msk    (0x1UL << RCC_APB2ENR_SYSCFGEN_Pos)
#define RCC_APB2ENR_SYSCFGEN        RCC_APB2ENR_SYSCFGEN_Msk
void rcc_init(void)
{
    RCC->AHB2ENR |= (1 << 0);                      // Habilita reloj GPIOA
    RCC->AHB2ENR |= (1 << 2);                      // Habilita reloj GPIOC
}
void rcc_syscfg_clock_enable(void) {
    RCC->APB2ENR |= RCC_APB2ENR_SYSCFGEN; // Habilita el reloj para SYSCFG
}
void rcc_tim3_clock_enable(void) {
    // TIM3EN está en el bit 1 del registro APB1ENR1
    RCC->APB1ENR1 |= (1U << 1); // Habilita el reloj para TIM3
}