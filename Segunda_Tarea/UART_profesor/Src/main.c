#include "gpio.h"
#include "systick.h"
#include "rcc.h"
#include "uart.h"
#include "nvic.h"

// --- Variables Globales ------------------------------------------------------
static volatile uint32_t ms_counter = 0; // Contador para el SysTick
static char rx_buffer[256];             // Buffer para caracteres UART recibidos por ISR
static uint8_t rx_index = 0;             // Índice del buffer de recepción
static volatile uint8_t uart_new_line_received = 0; // Bandera para nueva línea UART

// --- Programa Principal ------------------------------------------------------
int main(void)
{
    // 1. Inicialización de periféricos
    rcc_init();                                     // Relojes (GPIOA, GPIOC)
    init_gpio(GPIOA, 5, 0x01, 0x00, 0x01, 0x00, 0x00); // LED PA5 (Output)
    init_gpio(GPIOC, 13, 0x00, 0x00, 0x01, 0x01, 0x00); // Botón PC13 (Input)
    init_systick();                                 // SysTick para ms_counter
    init_gpio_uart();                               // GPIOs para UART (PA2, PA3)
    init_uart();                                    // Configuración básica de UART2

    // 2. Configuración de Interrupciones (NVIC y EXTI)
    nvic_exti_pc13_button_enable();                 // Habilitar IRQ EXTI13 para botón
    nvic_usart2_irq_enable();                       // Habilitar IRQ RXNE para UART2

    // 3. Configuración de Prioridades de Interrupción (para anidamiento)
    // Botón (EXTI15_10_IRQn) prioridad 0 (más alta)
    NVIC->IP[EXTI15_10_IRQn] = 0x00;
    // UART2 (USART2_IRQn) prioridad 1 (más baja que botón)
    NVIC->IP[USART2_IRQn]    = 0x10; // (0x10 es el bit 4, el valor real de prioridad)

    // 4. Mensaje de inicio
    uart_send_string("Sistema Inicializado (NVIC/EXTI/Prioridades)!\r\n");

    // 5. Bucle Principal (Superloop)
    while (1) {
        // Apagar LED después de 3 segundos (SysTick actualiza ms_counter)
        if (ms_counter >= 3000) {
            clear_gpio(GPIOA, 5);
        }

        // Procesar línea UART recibida por ISR (cuando la bandera lo indique)
        if (uart_new_line_received) {
            uart_send_string("Recibido (IRQ): ");
            uart_send_string(rx_buffer); // Contenido del buffer de la ISR
            uart_send_string("\r\n");
            
            uart_new_line_received = 0; // Limpiar bandera
            rx_index = 0;               // Resetear índice para próxima línea
        }
    }
}

// --- Manejadores de Interrupción ---------------------------------------------

// ISR de SysTick (cada ms)
void SysTick_Handler(void)
{
    ms_counter++;
}

// ISR para EXTI13 (Botón PC13)
void EXTI15_10_IRQHandler(void) {
    if ((EXTI->PR1 & (1U << 13)) != 0) { // Si la interrupción es por EXTI13
        ms_counter = 0;             // Reiniciar contador al presionar
        set_gpio(GPIOA, 5);         // Encender LED
        uart_send_string("Button EXTI IRQ START!\r\n"); // Log de inicio IRQ botón

        // Retraso artificial para demostrar anidamiento (NO PARA PRODUCCIÓN)
        for(volatile int i=0; i<200000; i++);

        uart_send_string("Button EXTI IRQ END!\r\n");   // Log de fin IRQ botón
        EXTI->PR1 |= (1U << 13);    // Limpiar flag de interrupción EXTI13
    }
}

// ISR para USART2 (Recepción RXNE)
void USART2_IRQHandler(void) {
    if ((USART2->ISR & (1U << 5)) != 0) { // Si la interrupción es por RXNE
        char received_data = (char)(USART2->RDR & 0xFF); // Leer byte para limpiar flag

        set_gpio(GPIOA, 5); // Encender LED brevemente al recibir

        // Almacenar en buffer y verificar fin de línea
        if (rx_index < sizeof(rx_buffer) - 1) {
            rx_buffer[rx_index++] = received_data;
            if (received_data == '\r' || received_data == '\n') {
                rx_buffer[rx_index] = '\0'; // Null-terminate
                uart_new_line_received = 1; // Indicar a main que hay una línea completa
            }
        } else { // Desbordamiento de buffer
            rx_index = 0; // Resetear
            uart_new_line_received = 0;
            // uart_send_string("UART Buffer Full!\r\n"); // No usar en ISR larga
        }
    }
}
/*Polling:
Pros:
Simplicidad: Es muy fácil de implementar. No requiere configurar la NVIC ni escribir manejadores de interrupción.
Control directo: El programador tiene control explícito sobre cuándo y cómo se revisa el estado de la UART.
Baja latencia (en sistemas simples): Si la MCU no tiene mucho que hacer, el polling puede responder rápidamente a eventos UART.
Menos sobrecarga de contexto: No hay cambios de contexto del procesador como en las interrupciones.
Contras:
Desperdicio de ciclos de CPU: La CPU pasa tiempo activamente verificando el registro ISR (en un bucle while(1)) incluso cuando no hay datos. En un sistema complejo, esto puede ser una gran ineficiencia.
Latencia variable/No determinista (en sistemas complejos): Si el main loop tiene otras tareas largas, la CPU podría tardar en revisar la UART, lo que lleva a un retardo en el procesamiento de los datos recibidos o, peor aún, a la pérdida de datos si la siguiente transmisión llega antes de que el byte actual sea leído (especialmente si no usas un buffer).
Inadecuado para altas velocidades de datos: A altas tasas de baudios, el polling podría no ser lo suficientemente rápido para leer cada byte, provocando desbordamientos de hardware (OVERRUN) y pérdida de datos.

Bloqueo: Tus funciones uart_send y uart_receive son bloqueantes (while (!(USART2->ISR & (1 << X)));). Esto significa que la CPU se detiene y espera si la UART no está lista, lo cual es inaceptable en sistemas en tiempo real o con multitarea. (Tu main loop es mejor porque solo hace polling y no bloquea directamente, pero la función uart_receive sí lo haría si la usaras activamente en el main).
Interrupciones (Futura mejora, probablemente en 7_NVIC.md):
Pros:
Eficiencia de CPU: La CPU solo es interrumpida cuando hay un evento UART (un byte recibido, un byte enviado, etc.). Puede dedicarse a otras tareas mientras tanto.
Respuesta en tiempo real: Garantiza que los eventos importantes (como la recepción de un byte) sean manejados de manera oportuna y predecible, independientemente de la carga de otras tareas en el main loop.
No bloqueante: Permite la implementación de comunicación UART no bloqueante, donde las funciones de envío/recepción regresan inmediatamente, y los datos se manejan en segundo plano.
Adecuado para altas velocidades de datos: Es la forma preferida de manejar la comunicación serial a velocidades de datos moderadas a altas, utilizando buffers circulares en la ISR para almacenar/recuperar datos.
Contras:
Mayor complejidad de implementación: Requiere configurar la NVIC (Nested Vectored Interrupt Controller), escribir un manejador de interrupción (USART2_IRQHandler), y gestionar la concurrencia (posibles race conditions si se accede a datos compartidos entre el main y la ISR).
Overhead de contexto: Cada interrupción conlleva un pequeño tiempo de sobrecarga debido al guardado y restauración del contexto del procesador.
Problemas de reentrada y latencia de ISR: Las ISRs deben ser cortas y eficientes. Bloquear o realizar operaciones largas dentro de una ISR puede afectar negativamente el rendimiento de otras interrupciones y del sistema.*/