#include "gpio.h"
#define GPIOA_BASE  0x48000000U
#define GPIOA     ((GPIO_TypeDef_t *) GPIOA_BASE) 
//#define GPIOA_MODER (*(volatile uint32_t *)(GPIOA_BASE + 0x00U)) // Configuración de modo
//#define GPIOA_ODR   (*(volatile uint32_t *)(GPIOA_BASE + 0x14U)) // Data de salida
#define LD2_PIN     5U                                         // Pin PA5 (LED)

#define GPIOC_BASE  0x48000800U
#define GPIOC     ((GPIO_TypeDef_t *) GPIOC_BASE)
//#define GPIOC_MODER (*(volatile uint32_t *)(GPIOC_BASE + 0x00U)) // Configuración de modo
//#define GPIOC_IDR   (*(volatile uint32_t *)(GPIOC_BASE + 0x10U)) // Data de entrada
#define B1_PIN      13U                                        // Pin PC13 (Botón)

void init_led(void)
{
    GPIOA->MODER &= ~(3 << (LD2_PIN*2)); // PA5 como salida
    GPIOA->MODER |=  (1 << (LD2_PIN*2));  
}

void init_button(void)
{
    GPIOC->MODER &= ~(3 << (B1_PIN*2)); // PC13 como entrada
}

void set_led(void)
{
    GPIOA->ODR |= (1 << LD2_PIN); // Enciende LED
}

void clear_led(void)
{
    GPIOA->ODR &= ~(1 << LD2_PIN); // Apaga LED
}

uint8_t read_button(void)
{
    if ((GPIOC->IDR & (1 << B1_PIN))==0){
        return 1; // Botón no presionado
    } 
    return 0; // Botón presionado
}