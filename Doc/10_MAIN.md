# 10. MAIN.md

## 1. Objetivo

Este documento guía la integración de todos los módulos en `main.c`, configurando el sistema para usar interrupciones en lugar de polling, y coordinando la inicialización de periféricos y la lógica de aplicación.

---

## 2. Preparación del Proyecto

1. **Modificar `Src/main.c`**: Actualiza el archivo principal para incluir inicializaciones de nuevos módulos e implementar el bucle principal con interrupciones.
2. **Asegurar includes**: Incluye todos los headers necesarios.
3. **Implementar ISRs**: Modifica los archivos de drivers para llamar a las funciones de `room_control` desde las ISRs.

---

## 3. Código de referencia para implementar (`Src/main.c`)

Modifica `Src/main.c` para integrar todos los módulos:

```c
#include "gpio.h"
#include "systick.h"
#include "rcc.h"
#include "uart.h"
#include "nvic.h"
#include "tim.h"
#include "room_control.h"

// Flags para eventos
volatile uint8_t button_event = 0;
volatile char uart_event_char = 0;

// Contador de milisegundos del sistema
volatile uint32_t system_ms_counter = 0;

// Función local para inicializar periféricos
static void peripherals_init(void)
{
    // Inicialización del sistema
    rcc_init();

    // Configuración de GPIOs
    gpio_init_pin(GPIOA, 5, GPIO_MODE_OUTPUT, GPIO_OTYPE_PP, GPIO_OSPEED_LOW, GPIO_PUPD_NONE);  // LED externo
    gpio_init_pin(GPIOB, 3, GPIO_MODE_OUTPUT, GPIO_OTYPE_PP, GPIO_OSPEED_LOW, GPIO_PUPD_NONE);  // LD2 (heartbeat)

    // Inicialización de periféricos
    systick_init();
    uart_init();  // Asumiendo función unificada
    nvic_exti_pc13_button_enable();
    nvic_usart2_irq_enable();
    tim3_ch1_pwm_init(1000);  // 1 kHz PWM
}

int main(void)
{
    peripherals_init();
    room_control_app_init();
    uart_send_string("Sistema de Control de Sala Inicializado!\r\n");

    // Bucle principal: procesa eventos
    while (1) {
        if (button_event) {
            button_event = 0;
            room_control_on_button_press();
        }
        if (uart_event_char) {
            char c = uart_event_char;
            uart_event_char = 0;
            room_control_on_uart_receive(c);
        }
        // Llamar a la función de actualización periódica
        room_control_update();
    }
}

// Manejador de SysTick
void SysTick_Handler(void)
{
    system_ms_counter++;
}

// Manejadores de interrupciones
void EXTI15_10_IRQHandler(void)
{
    // Limpiar flag de interrupción
    if (EXTI->PR1 & (1 << 13)) {
        EXTI->PR1 |= (1 << 13);  // Clear pending
        button_event = 1;
    }
}

void USART2_IRQHandler(void)
{
    // Verificar si es recepción
    if (USART2->ISR & (1 << 5)) {  // RXNE
        uart_event_char = (char)(USART2->RDR & 0xFF);
    }
}
```


---

## 4. Explicación de cambios

| Sección | Descripción |
| ------- | ----------- |
| main | Llama a inicializaciones y entra en bucle para pasar eventos. |
| Bucle principal | Pasa eventos a room_control. |
| peripherals_init | Inicializa todos los periféricos. |
| SysTick_Handler | Maneja tareas periódicas. |

---

## 5. Verificación de Funcionamiento

- **Compilación**: Sin errores.
- **Funcionalidad**: Sistema responde a botón y UART.
- **Integración**: Eventos pasan correctamente a room_control.

---

**Siguiente guía:**
User Manual: [USER_MANUAL.md](11_USER_MANUAL.md)