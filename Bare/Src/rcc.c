#include "rcc.h"

#define RCC_BASE    0x40021000U
#define RCC_AHB2ENR (*(volatile uint32_t *)(RCC_BASE  + 0x4CU)) // Habilita GPIOA clock

void rcc_init(void){
RCC_AHB2ENR |= (1 << 0);                      // Habilita reloj GPIOA
RCC_AHB2ENR |= (1 << 2);     
}