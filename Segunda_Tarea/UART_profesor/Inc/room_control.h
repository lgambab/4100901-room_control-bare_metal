#ifndef ROOM_CONTROL_H
#define ROOM_CONTROL_H

#include <stdint.h>
typedef enum {
    ROOM_IDLE,
    ROOM_OCCUPIED
} RoomState_t; // Usamos RoomState_t para consistencia con el main.c anterior

// Constantes
#define LED_TIMEOUT_MS          3000 // Timeout para apagar LED en estado OCCUPIED
#define DEBOUNCE_TIME_MS        100  // Tiempo de anti-rebote para el botón
#define PWM_INITIAL_DUTY        0    // Duty cycle inicial del PWM (0% apagado)
#define HEARTBEAT_INTERVAL_MS   500  // Intervalo del heartbeat

/**
 * @brief Función a ser llamada por EXTI15_10_IRQHandler cuando se detecta
 *        la pulsación del botón B1.
 */
void room_control_on_button_press(void);

/**
 * @brief Función a ser llamada por USART2_IRQHandler cuando se recibe un carácter.
 * @param received_char El carácter recibido por UART.
 */
void room_control_on_uart_receive(char received_char);

/**
 * @brief (Opcional) Función para realizar inicializaciones específicas de la lógica
 *        de room_control, si las hubiera.
 */
void room_control_app_init(void);

/**
 * @brief Función para actualizar la lógica de estados periódicamente (llamar en el bucle principal).
 *        Maneja timeouts, transiciones automáticas, etc.
 */
void room_control_update(void);
// Declaraciones de variables globales (para que main.c pueda acceder a ellas)
extern volatile RoomState_t current_state;
extern volatile uint32_t last_button_press_time_ms; // Unificada
extern volatile uint32_t last_button_event_time_ms; // Para anti-rebote
extern volatile uint32_t last_heartbeat_toggle_ms;  // Para el heartbeat
void room_control_heartbeat_update(void);
#endif // ROOM_CONTROL_H