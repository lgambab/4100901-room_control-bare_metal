# 6. UART.md
> **Referencia:**
> Zhu, Yifen. *Embedded Systems with ARM Cortex-M Microcontrollers in Assembly Language and C*, 2nd Edition, 2022. ***Capítulo 22 recomendado para esta sección***

## 1. Objetivo

Este documento guía la implementación de comunicación UART en **C puro** usando **typedef struct** para mapear los periféricos. **Este es un ejercicio hands-on donde debes implementar la funcionalidad UART desde cero siguiendo las instrucciones.**

---

## 2. Preparación del Proyecto

1. **Crear el archivo de cabecera**: en `Inc/`, crea un archivo llamado `uart.h`.
2. **Crear el archivo de código**: en `Src/`, crea un archivo llamado `uart.c`.
3. **Incluir en la compilación**: copia la línea `${CMAKE_CURRENT_SOURCE_DIR}/Src/uart.c` en `cmake/vscode_generated.cmake`.
4. **Implementar el código**: usa el código de las secciones 3 y 4 como referencia para implementar tu librería.
5. **Integrar en main.c**: modifica `Src/main.c` para incluir la inicialización UART y funcionalidad de eco.

---

## 3. Código de referencia para implementar (`Inc/uart.h`)

En el archivo `Inc/uart.h` e implementa el siguiente código:

```c
// uart.h
#include <stdint.h>
#include "gpio.h"  // Librería GPIO existente
#include "rcc.h"   // Librería RCC existente

// USART2
typedef struct {
    volatile uint32_t CR1;
    volatile uint32_t CR2;
    volatile uint32_t CR3;
    volatile uint32_t BRR;
    volatile uint32_t GTPR;
    volatile uint32_t RTOR;
    volatile uint32_t RQR;
    volatile uint32_t ISR;
    volatile uint32_t ICR;
    volatile uint32_t RDR;
    volatile uint32_t TDR;
} USART_Typedef_t;
#define USART2 ((USART_Typedef_t *)0x40004400U)

// Constantes
#define BAUD_RATE     115200U
#define HSI_FREQ      4000000U

// Prototipos de funciones
void init_gpio_uart(void);
void init_uart(void);
void uart_send(char c);
char uart_receive(void);
void uart_send_string(const char *str);
```

## 4. Código de referencia para implementar (`Src/uart.c`)

En el archivo `Src/uart.c` e implementa el siguiente código:

```c
#include "uart.h"

void init_gpio_uart(void) {
    // PA2->TX AF7, PA3->RX AF7
    RCC->AHB2ENR |= (1 << 0);  // Enable GPIOA
    GPIOA->MODER &= ~((3U<<4)|(3U<<6));
    GPIOA->MODER |=  ((2U<<4)|(2U<<6));
    GPIOA->AFRL &= ~((0xFU<<8)|(0xFU<<12));
    GPIOA->AFRL |=  ((7U<<8)|(7U<<12));  // AF7 = USART2
}

void init_uart(void) {
    RCC->APB1ENR1 |= (1 << 17);  // Enable USART2
    USART2->BRR = (HSI_FREQ + (BAUD_RATE/2)) / BAUD_RATE;
    USART2->CR1 = (1 << 3) | (1 << 2);  // TE | RE
    USART2->CR1 |= (1 << 0);            // UE
}

void uart_send(char c) {
    while (!(USART2->ISR & (1 << 7)));   // TXE
    USART2->TDR = (uint8_t)c;
}

char uart_receive(void) {
    while (!(USART2->ISR & (1 << 5)));   // RXNE
    return (char)(USART2->RDR & 0xFF);
}

void uart_send_string(const char *str) {
    while (*str) {
        uart_send(*str++);
    }
}
```

## 5. Integración en `Src/main.c`

Modifica `Src/main.c` para incluir la inicialización UART y enviar un mensaje de inicio. El bucle principal debe incluir el eco UART.

**INSTRUCCIONES:**
1. Agrega el include de `uart.h`
2. Declara las variables necesarias para el buffer de recepción
3. Llama las funciones de inicialización UART
4. Implementa el polling para recepción UART en el bucle principal

**Código de referencia para implementar:**

```c
#include "gpio.h"
#include "systick.h"
#include "rcc.h"
#include "uart.h"  // Agregar esta línea

static volatile uint32_t ms_counter = 17;
static char rx_buffer[256];
static uint8_t rx_index = 0;

// --- Programa principal ------------------------------------------------------
int main(void)
{
    rcc_init();
    init_gpio(GPIOA, 5, 0x01, 0x00, 0x01, 0x00, 0x00);
    init_gpio(GPIOC, 13, 0x00, 0x00, 0x01, 0x01, 0x00);
    init_systick();
    init_gpio_uart();  // Agregar inicialización GPIO para UART
    init_uart();       // Agregar inicialización UART
    
    uart_send_string("Sistema Inicializado!\r\n");  // Enviar mensaje de inicio
    
    while (1) {
        if (read_gpio(GPIOC, 13) != 0) { // Botón presionado
            ms_counter = 0;   // reiniciar el contador de milisegundos
            set_gpio(GPIOA, 5);        // Encender LED
        }
        
        if (ms_counter >= 3000) { // Si han pasado 3 segundos o más, apagar LED
            clear_gpio(GPIOA, 5);             // Apagar LED
        }

        // Polling UART receive
        if (USART2->ISR & (1 << 5)) {  // RXNE
            char c = (char)(USART2->RDR & 0xFF);
            if (rx_index < sizeof(rx_buffer) - 1) {
                rx_buffer[rx_index++] = c;
                if (c == '\r' || c == '\n') {
                    rx_buffer[rx_index] = '\0';  // Null terminate
                    uart_send_string("Recibido: ");
                    uart_send_string(rx_buffer);
                    uart_send_string("\r\n");
                    rx_index = 0;
                }
            }
        }
    }
}

// --- Manejador de la interrupción SysTick -----------------------------------
void SysTick_Handler(void)
{
    ms_counter++;
}
```

---

## 6. Explicación de funciones

| Función               | Descripción                                                                  |
| --------------------- | ---------------------------------------------------------------------------- |
| `init_gpio_uart()`    | Configura PA2/PA3 como AF7 para USART2.                                      |
| `init_uart()`         | Habilita RCC APB1 USART2, configura BRR, enciende UE/TE/RE.                  |
| `uart_send(char)`     | Espera TXE y escribe en TDR.                                                 |
| `uart_receive(void)`  | Espera RXNE y lee RDR.                                                       |
| `uart_send_string()`  | Envía una cadena completa carácter por carácter.                             |

---

## 7. Ejercicios Adicionales

Una vez que hayas implementado la funcionalidad básica UART:

1. **Prueba diferentes baudrates**: Cambia la configuración de tasa de baudios en `init_uart()` y prueba diferentes valores (ej. 9600, 19200).
2. **Mejora la recepción**: La implementación actual ya maneja líneas completas (hasta '\r' o '\n') y envía el mensaje: **"Recibido: \<línea>\r\n"**.
3. **Análisis técnico**: Documenta pros/cons de polling vs interrupciones para UART.

## 8. Verificación de Funcionamiento

Para verificar que tu implementación funciona correctamente:

1. **Compilación**: El proyecto debe compilar sin errores
2. **Funcionalidad**:
   - Al iniciar, debe enviar "Sistema Inicializado!\r\n"
   - Debe hacer eco de los mensajes recibidos con el formato "Recibido: \<mensaje>\r\n"
   - El LED y botón deben seguir funcionando normalmente

## 9. Funcionalidad UART operativa

   **Pros de Polling:**
   - Simple de implementar.
   - No requiere configuración de interrupciones.
   - Menos overhead de context switching.

   **Cons de Polling:**
   - Consume CPU continuamente verificando el estado.
   - No eficiente para aplicaciones con múltiples tareas.
   - Puede perder datos si el polling no es lo suficientemente frecuente.

   **Pros de Interrupciones:**
   - CPU libre para otras tareas hasta que llegue el dato.
   - Respuesta inmediata a eventos.
   - Mejor para sistemas en tiempo real.

   **Cons de Interrupciones:**
   - Mayor complejidad en el código.
   - Riesgo de anidamiento de interrupciones.
   - Overhead de entrada/salida de ISR.

---

**Siguiente guía:**
NVIC: [NVIC.md](7_NVIC.md)

