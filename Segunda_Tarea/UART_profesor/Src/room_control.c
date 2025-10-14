#include "room_control.h" // Incluimos nuestro propio header
#include "gpio.h"         // Para controlar LEDs
#include "systick.h"      // Para obtener ticks (ms_counter)
#include "uart.h"         // Para enviar mensajes
#include "tim.h"          // Para controlar el PWM
#include <string.h>       // Para usar strcmp si se implementan comandos UART complejos

// --- Variables Globales del Módulo Room Control ---
volatile RoomState_t current_state = ROOM_IDLE; // Variable de estado global (unificada)
volatile uint32_t last_button_press_time_ms = 0; // Último tiempo de pulsación para timeout (unificada)
volatile uint32_t last_button_event_time_ms = 0; // Para anti-rebote
volatile uint32_t last_heartbeat_toggle_ms = 0;  // Para el heartbeat

// Necesitamos acceso a ms_counter global, declarado en main.c
extern volatile uint32_t ms_counter;


void room_control_app_init(void)
{
    // Inicializar el periférico TIM3 para PWM
    tim3_ch1_pwm_init(1000); // 1 kHz PWM
    tim3_ch1_pwm_set_duty_cycle(PWM_INITIAL_DUTY); // Establecer el duty cycle inicial (0%)

    // Inicializar el LED PA5 (si se usa independientemente del PWM)
    clear_gpio(GPIOA, 5); // Asegurar que PA5 esté apagado inicialmente

    current_state = ROOM_IDLE; // Asegurar estado inicial
    uart_send_string("Room Control: Iniciado en estado IDLE.\r\n");
    // Inicializar todos los contadores al valor actual de ms_counter
    last_button_press_time_ms = ms_counter;
    last_button_event_time_ms = ms_counter;
    last_heartbeat_toggle_ms = ms_counter;
}

void room_control_on_button_press(void)
{
    // Lógica de anti-rebote
    if (ms_counter - last_button_event_time_ms < DEBOUNCE_TIME_MS) {
        return; // Evento ignorado por anti-rebote
    }
    last_button_event_time_ms = ms_counter; // Actualizar tiempo del último evento válido

    // Actualizar el tiempo de la última pulsación del botón para el timeout
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
            // Si ya está OCCUPIED y se pulsa el botón, lo apagamos y volvemos a IDLE
            current_state = ROOM_IDLE;
            clear_gpio(GPIOA, 5); // Apagar LED PA5
            tim3_ch1_pwm_set_duty_cycle(0);   // Apagar luces PWM
            uart_send_string("Room Control: Boton -> OCCUPIED a IDLE. Luces apagadas.\r\n");
            break;
        default:
            break; // No debería ocurrir
    }
}

void room_control_on_uart_receive(char received_char)
{
    // Al recibir un comando UART que enciende las luces o entra en OCCUPIED,
    // debemos resetear el temporizador de timeout
    uint8_t reset_timeout = 0;

    switch (received_char) {
        case 'h': // High brightness
        case 'H':
            tim3_ch1_pwm_set_duty_cycle(100);
            uart_send_string("Room Control: PWM a 100%.\r\n");
            reset_timeout = 1;
            break;
        case 'l': // Low brightness (effectively off)
        case 'L':
            tim3_ch1_pwm_set_duty_cycle(0);
            uart_send_string("Room Control: PWM a 0%.\r\n");
            break;
        case 'O': // Force OCCUPIED state
        case 'o':
            if (current_state == ROOM_IDLE) {
                current_state = ROOM_OCCUPIED;
                set_gpio(GPIOA, 5); // Encender LED PA5
                tim3_ch1_pwm_set_duty_cycle(100); // Luces al máximo
                uart_send_string("Room Control: UART -> Cambiado a OCCUPIED. Luces al 100%.\r\n");
                reset_timeout = 1;
            } else {
                uart_send_string("Room Control: Ya en estado OCCUPIED.\r\n");
            }
            break;
        case 'I': // Force IDLE state
        case 'i':
            if (current_state == ROOM_OCCUPIED) {
                current_state = ROOM_IDLE;
                clear_gpio(GPIOA, 5); // Apagar LED PA5
                tim3_ch1_pwm_set_duty_cycle(0);   // Apagar luces
                uart_send_string("Room Control: UART -> Cambiado a IDLE. Luces apagadas.\r\n");
            } else {
                uart_send_string("Room Control: Ya en estado IDLE.\r\n");
            }
            break;
        // Comandos de brillo por números
        case '0': tim3_ch1_pwm_set_duty_cycle(0);   reset_timeout = 0; uart_send_string("Room Control: PWM a 0%.\r\n"); break;
        case '1': tim3_ch1_pwm_set_duty_cycle(10);  reset_timeout = 1; uart_send_string("Room Control: PWM a 10%.\r\n"); break;
        case '2': tim3_ch1_pwm_set_duty_cycle(20);  reset_timeout = 1; uart_send_string("Room Control: PWM a 20%.\r\n"); break;
        case '3': tim3_ch1_pwm_set_duty_cycle(30);  reset_timeout = 1; uart_send_string("Room Control: PWM a 30%.\r\n"); break;
        case '4': tim3_ch1_pwm_set_duty_cycle(40);  reset_timeout = 1; uart_send_string("Room Control: PWM a 40%.\r\n"); break;
        case '5': tim3_ch1_pwm_set_duty_cycle(50);  reset_timeout = 1; uart_send_string("Room Control: PWM a 50%.\r\n"); break;
        case '6': tim3_ch1_pwm_set_duty_cycle(60);  reset_timeout = 1; uart_send_string("Room Control: PWM a 60%.\r\n"); break;
        case '7': tim3_ch1_pwm_set_duty_cycle(70);  reset_timeout = 1; uart_send_string("Room Control: PWM a 70%.\r\n"); break;
        case '8': tim3_ch1_pwm_set_duty_cycle(80);  reset_timeout = 1; uart_send_string("Room Control: PWM a 80%.\r\n"); break;
        case '9': tim3_ch1_pwm_set_duty_cycle(90);  reset_timeout = 1; uart_send_string("Room Control: PWM a 90%.\r\n"); break;
        case 'f': // Comando para full brillo
        case 'F': tim3_ch1_pwm_set_duty_cycle(100); reset_timeout = 1; uart_send_string("Room Control: PWM a 100%.\r\n"); break;
        default:
            uart_send_string("Room Control: Comando UART desconocido.\r\n");
            break;
    }

    // Si algún comando de UART puso las luces o el estado en OCCUPIED,
    // actualizamos el estado y el tiempo de la última actividad.
    if (reset_timeout) {
        current_state = ROOM_OCCUPIED; // Asegurar estado OCCUPIED
        set_gpio(GPIOA, 5);           // Asegurar LED PA5 encendido
        last_button_press_time_ms = ms_counter; // Resetear timeout
    } else if (received_char == '0' || received_char == 'l' || received_char == 'L') {
        // Si se apagaron las luces con '0' o 'l'/'L', podemos cambiar a IDLE.
        current_state = ROOM_IDLE;
        clear_gpio(GPIOA, 5);
    }
}


void room_control_update(void)
{
    // Lógica periódica: Timeout para apagar LED en estado occupied
    if (current_state == ROOM_OCCUPIED) {
        if (ms_counter - last_button_press_time_ms >= LED_TIMEOUT_MS) {
            current_state = ROOM_IDLE; // Cambiar a estado IDLE
            clear_gpio(GPIOA, 5);       // Apagar LED PA5
            tim3_ch1_pwm_set_duty_cycle(0); // Apagar luces PWM
            uart_send_string("Room Control: Timeout! Cambiado a IDLE. Luces apagadas.\r\n");
        }
    }
}

// Nueva función para el heartbeat, llamada desde SysTick_Handler
void room_control_heartbeat_update(void) {
    if (ms_counter - last_heartbeat_toggle_ms >= HEARTBEAT_INTERVAL_MS) {
        last_heartbeat_toggle_ms = ms_counter;

        // Solo alternamos el LED PA5 si el sistema está en estado IDLE.
        if (current_state == ROOM_IDLE) {
            if (read_gpio(GPIOA, 5)) {
                clear_gpio(GPIOA, 5);
            } else {
                set_gpio(GPIOA, 5);
            }
        }
    }
}