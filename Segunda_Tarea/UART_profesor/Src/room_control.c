#include "room_control.h"

#include "gpio.h"    // Para controlar LEDs
#include "systick.h" // Para obtener ticks y manejar tiempos
#include "uart.h"    // Para enviar mensajes
#include "tim.h"     // Para controlar el PWM

// Estados de la sala
typedef enum {
    ROOM_IDLE,
    ROOM_OCCUPIED
} room_state_t;

// Variable de estado global
room_state_t current_state = ROOM_IDLE;

static volatile uint32_t last_button_press_ms = 0; // Último tiempo de pulsación del botón

void room_control_app_init(void)
{
    // TODO: Implementar inicializaciones específicas de la aplicación
    // Inicializar el periférico TIM3 para PWM
    tim3_ch1_pwm_init(1000); // 1 kHz PWM
    tim3_ch1_pwm_set_duty_cycle(PWM_INITIAL_DUTY); // Establecer el duty cycle inicial

    // Inicializar el LED PA5 (si se usa independientemente del PWM)
    clear_gpio(GPIOA, 5); // Asegurar que PA5 esté apagado inicialmente

    current_state = ROOM_IDLE; // Asegurar estado inicial
    uart_send_string("Room Control: Iniciado en estado IDLE.\r\n");
}

void room_control_on_button_press(void)
{
    // TODO: Implementar la lógica para manejar la pulsación del botón usando estados
    // Ejemplo: Si idle, cambiar a occupied; si occupied, cambiar a idle
    // Actualizar el tiempo de la última pulsación del botón
    extern volatile uint32_t ms_counter; // Necesitamos acceso al ms_counter global
    last_button_press_time_ms = ms_counter;

    // Lógica para manejar la pulsación del botón usando estados
    switch (current_state) {
        case ROOM_IDLE:
            current_state = ROOM_OCCUPIED;
            set_gpio(GPIOA, 5); // Encender LED PA5 como indicador de ocupado
            tim3_ch1_pwm_set_duty_cycle(100); // Encender luces PWM al máximo
            uart_send_string("Room Control: Boton -> IDLE a OCCUPIED. Luces al 100%.\r\n");
            break;
        case ROOM_OCCUPIED:
            current_state = ROOM_IDLE;
            clear_gpio(GPIOA, 5); // Apagar LED PA5
            tim3_ch1_pwm_set_duty_cycle(0);   // Apagar luces PWM
            uart_send_string("Room Control: Boton -> OCCUPIED a IDLE. Luces apagadas.\r\n");
            break;
        default:
            break;
    }
}

void room_control_on_uart_receive(char received_char)
{
    switch (received_char) {
        case 'h':
        case 'H':
            // TODO: Set PWM to 100%
            tim3_ch1_pwm_set_duty_cycle(100);
            uart_send_string("Room Control: PWM a 100%.\r\n");
            // Si estaba IDLE y se enciende, puede ser que la sala se ocupe
            if (current_state == ROOM_IDLE) {
                 current_state = ROOM_OCCUPIED;
                 set_gpio(GPIOA, 5);
            }
            break;
        case 'l':
        case 'L':
            // TODO: Set PWM to 0%
            tim3_ch1_pwm_set_duty_cycle(0);
            uart_send_string("Room Control: PWM a 0%.\r\n");
            // Si se apagan las luces, la sala pasa a IDLE
            if (current_state == ROOM_OCCUPIED) {
                current_state = ROOM_IDLE;
                clear_gpio(GPIOA, 5);
            }
            break;
        case 'O':
        case 'o':
            // TODO: Cambiar estado a occupied
            if (current_state == ROOM_IDLE) {
                current_state = ROOM_OCCUPIED;
                set_gpio(GPIOA, 5); // Encender LED PA5
                tim3_ch1_pwm_set_duty_cycle(100); // Luces al máximo
                uart_send_string("Room Control: UART -> Cambiado a OCCUPIED. Luces al 100%.\r\n");
            } else {
                uart_send_string("Room Control: Ya en estado OCCUPIED.\r\n");
            }
            break;
        case 'I':
        case 'i':
            // TODO: Cambiar estado a idle
            if (current_state == ROOM_OCCUPIED) {
                current_state = ROOM_IDLE;
                clear_gpio(GPIOA, 5); // Apagar LED PA5
                tim3_ch1_pwm_set_duty_cycle(0);   // Apagar luces
                uart_send_string("Room Control: UART -> Cambiado a IDLE. Luces apagadas.\r\n");
            } else {
                uart_send_string("Room Control: Ya en estado IDLE.\r\n");
            }
            break;
        default:
            // TODO: Echo the character
            uart_send_string("Room Control: Comando no reconocido.\r\n");
            break;
    }
}

void room_control_update(void)
{
    // TODO: Implementar lógica periódica, como timeouts para apagar LED en estado occupied
    // Ejemplo: Si estado occupied y han pasado 3s desde button press, cambiar a idle y apagar LEDextern volatile uint32_t ms_counter; // Necesitamos acceso al ms_counter global

    // Lógica periódica, como timeouts para apagar LED en estado occupied
    if (current_state == ROOM_OCCUPIED) {
        // Si han pasado LED_TIMEOUT_MS desde la última pulsación de botón
        if (ms_counter - last_button_press_time_ms >= LED_TIMEOUT_MS) {
            current_state = ROOM_IDLE; // Cambiar a estado IDLE
            clear_gpio(GPIOA, 5);       // Apagar LED PA5
            tim3_ch1_pwm_set_duty_cycle(0); // Apagar luces PWM
            uart_send_string("Room Control: Timeout! Cambiado a IDLE. Luces apagadas.\r\n");
        }
    }
}