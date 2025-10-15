#include "gpio.h"                                     // Pin PC13 (Button)


void init_gpio(GPIO_Typedef_t * GPIO, uint8_t pin, uint8_t mode, uint8_t type, uint8_t speed, uint8_t pupd, uint8_t initial_value, uint8_t af_num)
{
    GPIO->MODER &= ~(3 << (pin*2));           // Limpia bits
    GPIO->MODER |=  (mode << (pin*2));           // Configura modo

    GPIO->TYPER &= ~(1 << pin);           // Limpia bits
    GPIO->TYPER |=  (type << pin);

    GPIO->SPEEDR &= ~(3 << (pin*2));
    GPIO->SPEEDR |= (speed << (pin*2));
    
    GPIO->PUPDR &= ~(3 << (pin*2));
    GPIO->PUPDR |= (pupd << (pin*2));

    GPIO->ODR &= ~(1 << pin);
    GPIO->ODR |= (initial_value << pin);
     // *** NUEVA LÓGICA PARA CONFIGURAR LA FUNCIÓN ALTERNATIVA (AF) ***
    if (mode == GPIO_MODE_AF) {
        if (pin < 8) { // Pines del 0 al 7 usan AFRL
            GPIO->AFRL &= ~(0xFU << (pin * 4));           // Limpia los 4 bits del AF para el pin
            GPIO->AFRL |= ((uint32_t)af_num << (pin * 4)); // Configura el número de AF
        } else { // Pines del 8 al 15 usan AFRH
            GPIO->AFRH &= ~(0xFU << ((pin - 8) * 4));           // Limpia los 4 bits del AF para el pin (ajustando el índice)
            GPIO->AFRH |= ((uint32_t)af_num << ((pin - 8) * 4)); // Configura el número de AF
        }
    }
}

void set_gpio(GPIO_Typedef_t * GPIO, uint8_t pin)
{
    GPIO->ODR |= (1 << pin);
}

void clear_gpio(GPIO_Typedef_t * GPIO, uint8_t pin)
{
    GPIO->ODR &= ~(1 << pin);
}


uint8_t read_gpio(GPIO_Typedef_t * GPIO, uint8_t pin)
{
    // Leer estado del botón PC13 (botón presionado = 0, no presionado = 1)
    if ((GPIO->IDR & (1 << pin)) == 0) { // pressed
        return 1;
    }
    return 0;
}
