#include "gpio.h"

#define GPIOA_BASE  0x48000000U
#define GPIOA_MODER (*(volatile uint32_t *)(GPIOA_BASE + 0x00U)) // Configuración de modo
#define GPIOA_ODR   (*(volatile uint32_t *)(GPIOA_BASE + 0x14U)) // Data de salida
#define LD2_PIN     5U                                         // Pin PA5 (LED)

// --- Definiciones de registros para Button B1 (Ver RM0351) ------------------
#define GPIOC_BASE  0x48000800U
#define GPIOC_MODER (*(volatile uint32_t *)(GPIOC_BASE + 0x00U)) // Configuración de modo
#define GPIOC_IDR   (*(volatile uint32_t *)(GPIOC_BASE + 0x10U)) // Data de entrada
#define B1_PIN      13U                                        // Pin PC13 (Button)
void init_led(void);
{
    GPIOA_MODER &= ~(3 << (LD2_PIN*2));           // Limpia bits
    GPIOA_MODER |=  (1 << (LD2_PIN*2));           // Configura como salida
}

void init_button(void);
{
    GPIOC_MODER &= ~(3 << (B1_PIN*2));            // PC13 como entrada (00)

}
void set_led(void);
{
    GPIOA_ODR |= (1 << LD2_PIN);              // Encender LED
}
void clear_led(void);
{
    GPIOA_ODR &= ~(1 << LD2_PIN);             // Apagar LED
}
uint8_t read_button(void);
{
   // Leer estado del botón PC13 (botón presionado = 0, no presionado = 1)
    if (!(GPIOC_IDR & (1 << B1_PIN)) == 0 ) {
        return 1;
    }
    return 0;
}