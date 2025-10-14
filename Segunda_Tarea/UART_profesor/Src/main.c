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
    init_gpio(GPIOA, 5, 0x01, 0x00, 0x01, 0x00, 0x00); // LED PA5 (Output)
    init_gpio(GPIOC, 13, 0x00, 0x00, 0x01, 0x01, 0x00); // Botón PC13 (Input)

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