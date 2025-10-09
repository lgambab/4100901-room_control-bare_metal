# 8. TIM.md
> **Referencia:**
> Zhu, Yifen. *Embedded Systems with ARM Cortex-M Microcontrollers in Assembly Language and C*, 2nd Edition, 2022. ***Capítulo 15 recomendado para esta sección***

## 1. Objetivo

Este documento guía la configuración de **TIM3\_CH1** como salida PWM en **C puro**, empleando `typedef struct` para mapear registros de periféricos, definiendo macros para acceso a hardware y organizando las funciones.

---

## 2. Preparación del Proyecto

1. **Crear el archivo de código**: en `Src/`, crea un archivo llamado `tim.c`.
2. **Incluir en la compilación**: agrega `Src/tim.c` en `cmake/vscode_generated.cmake` en la sección de fuentes.
3. **Conectar LED**: conecta un LED con resistor (220Ω) entre PA6 y GND para simular una bombilla controlada por PWM.
4. **Agregar el código de ejemplo**: copia el bloque de la sección 3 y 4, y pégalo en `Src/tim.c`.

---

## 3. Código de ejemplo (`Inc/tim.h` y `Src/tim.c`)

En `Inc/tim.h`:
```c
#ifndef TIM_H
#define TIM_H

#include <stdint.h>
#include "gpio.h"  // Librería GPIO existente
#include "rcc.h"   // Librería RCC existente

// TIM3
typedef struct {
    volatile uint32_t CR1;
    volatile uint32_t CR2;
    volatile uint32_t SMCR;
    volatile uint32_t DIER;
    volatile uint32_t SR;
    volatile uint32_t EGR;
    volatile uint32_t CCMR1;
    volatile uint32_t CCMR2;
    volatile uint32_t CCER;
    volatile uint32_t CNT;
    volatile uint32_t PSC;
    volatile uint32_t ARR;
    volatile uint32_t RESERVED1;
    volatile uint32_t CCR1;
    volatile uint32_t CCR2;
    volatile uint32_t CCR3;
    volatile uint32_t CCR4;
    volatile uint32_t RESERVED2;
    volatile uint32_t DCR;
    volatile uint32_t DMAR;
} TIM_TypeDef;

#define TIM3_BASE           (0x40000400UL)
#define TIM3                ((TIM_TypeDef *) TIM3_BASE)

// Constantes
#define TIM_PCLK_FREQ_HZ    4000000U   // 4 MHz APB1 timer clock
#define PWM_FREQUENCY       1000U      // 1 kHz PWM
#define PWM_PERIOD          (TIM_PCLK_FREQ_HZ / PWM_FREQUENCY)
#define PWM_DUTY_CYCLE      50         // 50% duty cycle
#define PWM_PIN             6U         // PA6 = TIM3_CH1

#define PWM_DC_TO_CCR(DC) ((PWM_PERIOD * (DC)) / 100) // Macro para calcular CCR

// Prototipos de funciones
void tim3_ch1_pwm_init(uint32_t pwm_freq_hz);
void tim3_ch1_pwm_set_duty_cycle(uint8_t duty_cycle_percent); // duty_cycle en % (0-100)

#endif // TIM_H
```

En `Src/tim.c`:
```c
#include "tim.h"
#include "rcc.h"  // Para rcc_tim3_clock_enable
#include "gpio.h" // Para gpio_setup_pin

void tim3_ch1_pwm_init(uint32_t pwm_freq_hz)
{
    // 1. Configurar PA6 como Alternate Function (AF2) para TIM3_CH1
    gpio_setup_pin(GPIOA, 6, GPIO_MODE_AF, 2);

    // 2. Habilitar el reloj para TIM3
    rcc_tim3_clock_enable();

    // 3. Configurar TIM3
    TIM3->PSC = 100 - 1; // (4MHz / 100 = 40kHz)
    TIM3->ARR = (TIM_PCLK_FREQ_HZ / 100 / pwm_freq_hz) - 1; // 40kHz / pwm_freq_hz

    // Configurar el Canal 1 (CH1) en modo PWM 1
    TIM3->CCMR1 = (6U << 4);                    // PWM mode 1 on CH1
    TIM3->CCER  |= (1 << 0);                    // Enable CH1 output

    // Finalmente, habilitar el contador del timer
    TIM3->CR1 |= 0x01 << 0;
}

void tim3_ch1_pwm_set_duty_cycle(uint8_t duty_cycle_percent)
{
    if (duty_cycle_percent > 100) {
        duty_cycle_percent = 100;
    }

    // Calcular el valor de CCR1 basado en el porcentaje y el valor de ARR
    uint16_t tim3_ch1_arr_value = TIM3->ARR;
    uint32_t ccr_value = (((uint32_t)tim3_ch1_arr_value + 1U) * duty_cycle_percent) / 100U;

    TIM3->CCR1 = ccr_value;
}
```

---

## 5. Explicación de funciones

| Función           | Descripción                                                          |
| ----------------- | -------------------------------------------------------------------- |
| `tim3_ch1_pwm_init()` | Configura PA6 como AF2, habilita TIM3 clock, programa PSC/ARR, configura PWM1 en CH1, habilita salida y contador. |
| `tim3_ch1_pwm_set_duty_cycle()` | Calcula y establece CCR1 basado en el porcentaje de duty cycle. |

---

## 6. Ejercicio

1. Ajusta **duty cycle** cambiando `tim3_ch1_pwm_set_duty_cycle` al 10%, 25%, y 75%.
2. Crea bucle en `main()` que varíe el duty ciclo del 0% al 100% de forma suave.
3. Integra en `room_control.c` para controlar el brillo del LED PWM vía UART.

---

**Siguiente guía:**
ROOM_CONTROL: [ROOM_CONTROL.md](9_ROOM_CONTROL.md)
