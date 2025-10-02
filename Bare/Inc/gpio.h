// gpio.h
#include <stdint.h>

typedef struct {
    volatile uint32_t MODER; // Modo de configuración
    volatile uint32_t TYPER; // Tipo de salida
    volatile uint32_t SPEEDR; // Velocidad de salida
    volatile uint32_t PUPDR; // Pull-up/Pull-down
    volatile uint32_t IDR; // Data de entrada
    volatile uint32_t ODR; // Data de salida

} GPIO_TypeDef_t; 

void init_led(void);
void init_button(void);

void set_led(void);
void clear_led(void);
uint8_t read_button(void);