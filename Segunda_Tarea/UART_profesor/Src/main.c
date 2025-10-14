#include "gpio.h"
#include "systick.h"
#include "rcc.h"
#include "uart.h"
#include "nvic.h"
#include "tim.h"
#include "room_control.h" // Incluimos el header unificado
#include <string.h>       // Para usar strcmp

// --- Variables Globales de main.c ---
volatile uint32_t ms_counter = 0; // Contador para el SysTick (NO static para que room_control pueda acceder)
static char rx_buffer[256];             // Buffer para caracteres UART recibidos por ISR
static uint8_t rx_index = 0;             // Índice del buffer de recepción
static volatile uint8_t uart_new_line_received = 0; // Bandera para nueva línea UART

// --- Programa Principal ------------------------------------------------------
int main(void)
{
    // 1. Inicialización de periféricos
    rcc_init();
    init_gpio(GPIOA, 5, 0x01, 0x00, 0x01, 0x00, 0x00); // LED PA5 (Output)
    init_gpio(GPIOC, 13, 0x00, 0x00, 0x01, 0x01, 0x00); // Botón PC13 (Input)
    init_systick();
    init_gpio_uart();
    init_uart();

    // Ahora room_control_app_init() se encarga del PWM y del estado inicial
    room_control_app_init();

    // 2. Configuración de Interrupciones (NVIC y EXTI)
    nvic_exti_pc13_button_enable();
    nvic_usart2_irq_enable();

    // 3. Configuración de Prioridades de Interrupción
    NVIC->IP[EXTI15_10_IRQn] = 0x00; // Prioridad más alta para el botón
    NVIC->IP[USART2_IRQn]    = 0x10; // Prioridad más baja para UART

    // 4. Mensaje de inicio (ya lo hace room_control_app_init)
    // uart_send_string("Sistema de Control de Sala Iniciado!\r\n");

    // 5. Bucle Principal (Superloop)
    while (1) {
        // Llama a la función de control de la sala en cada iteración
        room_control_update();

        // Procesar comandos UART si se recibe una línea completa
        if (uart_new_line_received) {
            uart_new_line_received = 0; // Limpiar la bandera
            // Procesar cada caracter de la línea recibida
            for (int i = 0; i < rx_index; i++) {
                // Solo procesamos caracteres "válidos", ignoramos '\r' y '\n' aquí
                if (rx_buffer[i] != '\r' && rx_buffer[i] != '\n' && rx_buffer[i] != '\0') {
                    room_control_on_uart_receive(rx_buffer[i]);
                }
            }
            rx_index = 0; // Resetear el índice del buffer para la próxima recepción
        }
    }
}

// --- Manejadores de Interrupción ---------------------------------------------

// ISR de SysTick (cada ms)
void SysTick_Handler(void)
{
    ms_counter++; // Incrementa el contador global
    room_control_heartbeat_update(); // Llama a la nueva función de heartbeat
}

// ISR para EXTI13 (Botón PC13)
void EXTI15_10_IRQHandler(void) {
    if ((EXTI->PR1 & (1U << 13)) != 0) {
        // Llama a la función de control de la sala para manejar la pulsación
        room_control_on_button_press();

        // El retraso artificial aquí no debería estar en un ISR real.
        // uart_send_string("Button EXTI IRQ START!\r\n"); // Ya lo maneja room_control
        // for(volatile int i=0; i<200000; i++);
        // uart_send_string("Button EXTI IRQ END!\r\n");   // Ya lo maneja room_control

        EXTI->PR1 |= (1U << 13);
    }
}

// ISR para USART2 (Recepción RXNE)
void USART2_IRQHandler(void) {
    if ((USART2->ISR & (1U << 5)) != 0) {
        char received_data = (char)(USART2->RDR & 0xFF);

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