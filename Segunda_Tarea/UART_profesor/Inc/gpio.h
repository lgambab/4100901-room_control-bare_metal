// gpio.h
#ifndef GPIO_H
#define GPIO_H
#include <stdint.h>

typedef struct {
    volatile uint32_t MODER;
    volatile uint32_t TYPER;
    volatile uint32_t SPEEDR;
    volatile uint32_t PUPDR;
    volatile uint32_t IDR;
    volatile uint32_t ODR;
    volatile uint32_t BSRR;
    volatile uint32_t LCKR;
    volatile uint32_t AFRL;
    volatile uint32_t AFRH;
    volatile uint32_t BRR;
    volatile uint32_t ASCR;

} GPIO_Typedef_t;


#define GPIOA_BASE  0x48000000U
#define GPIOA       ((GPIO_Typedef_t *) GPIOA_BASE)

#define GPIOB_BASE  0x48000400U
#define GPIOB       ((GPIO_Typedef_t *) GPIOB_BASE)

#define GPIOC_BASE  0x48000800U
#define GPIOC       ((GPIO_Typedef_t *) GPIOC_BASE)

#define GPIOD_BASE  0x48000C00U
#define GPIOD       ((GPIO_Typedef_t *) GPIOD_BASE)

#define GPIOE_BASE  0x48001000U
#define GPIOE       ((GPIO_Typedef_t *) GPIOE_BASE)

#define GPIOF_BASE  0x48001400U
#define GPIOF       ((GPIO_Typedef_t *) GPIOF_BASE)

#define GPIOG_BASE  0x48001800U
#define GPIOG       ((GPIO_Typedef_t *) GPIOG_BASE)

#define GPIOH_BASE  0x48001C00U
#define GPIOH       ((GPIO_Typedef_t *) GPIOH_BASE)

// (agregamos diferentes modos para teminar de completar la libreria)Modos GPIO (Input, Output, Alternate Function, Analog)
#define GPIO_MODE_INPUT     0x00U
#define GPIO_MODE_OUT  PUT    0x01U
#define GPIO_MODE_AF        0x02U
#define GPIO_MODE_ANALOG    0x03U

// Output Type (Push-pull / Open-drain)
#define GPIO_TYPE_PP        0x00U
#define GPIO_TYPE_OD        0x01U

// Output Speed
#define GPIO_SPEED_LOW      0x00U
#define GPIO_SPEED_MEDIUM   0x01U
#define GPIO_SPEED_HIGH     0x02U
#define GPIO_SPEED_VERY_HIGH 0x03U

// Pull-up / Pull-down
#define GPIO_PUPD_NONE      0x00U
#define GPIO_PUPD_PU        0x01U
#define GPIO_PUPD_PD        0x02U
#define LD2_PIN     5U                                         // Pin PA5 (LED)
#define B1_PIN      13U   

void init_gpio(GPIO_Typedef_t * GPIO, uint8_t pin, uint8_t mode, uint8_t type, uint8_t speed, uint8_t pupd, uint8_t initial_value, uint8_t af_num);
// Modificamos el prototipo para reflejar la necesidad del AF
void set_gpio(GPIO_Typedef_t * GPIO, uint8_t pin);
void clear_gpio(GPIO_Typedef_t * GPIO, uint8_t pin);
uint8_t read_gpio(GPIO_Typedef_t * GPIO, uint8_t pin);
void gpio_setup_pin(GPIO_Typedef_t *GPIOx, uint8_t pin, uint8_t mode, uint8_t af);
#endif