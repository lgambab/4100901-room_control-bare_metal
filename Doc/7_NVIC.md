# 7. NVIC.md
> **Referencia:**
> Zhu, Yifen. *Embedded Systems with ARM Cortex-M Microcontrollers in Assembly Language and C*, 2nd Edition, 2022. ***Capítulo 11 recomendado para esta sección***

## 1. Objetivo

Este documento guía la configuración del NVIC (Nested Vectored Interrupt Controller) y EXTI para interrupciones en **C puro**, empleando `typedef struct` para mapear registros de periféricos, definiendo macros para acceso a hardware y organizando las funciones. Se enfocará en habilitar interrupciones para el botón (EXTI13) y UART (USART2).

---

## 2. Preparación del Proyecto

1. **Crear el archivo de cabecera**: en `Inc/`, crea un archivo llamado `nvic.h`.
2. **Crear el archivo de código**: en `Src/`, crea un archivo llamado `nvic.c`.
3. **Incluir en la compilación**: agrega `Src/nvic.c` en `cmake/vscode_generated.cmake` en la sección de fuentes.
4. **Agregar inicialización SYSCFG en RCC**: en `Src/rcc.c`, agrega la función `rcc_syscfg_clock_enable()` para habilitar el reloj de SYSCFG.
5. **Implementar el código**: usa el código de las secciones 3, 4 y 5 como referencia para implementar tu librería.

---

## 3. Código de referencia para implementar (`Inc/nvic.h`)

En el archivo `Inc/nvic.h` implementa el siguiente código:

```c
#ifndef NVIC_H
#define NVIC_H

#include <stdint.h>

// SYSCFG
typedef struct {
    volatile uint32_t MEMRMP;
    volatile uint32_t CFGR1;
    volatile uint32_t EXTICR[4];
    volatile uint32_t SCSR;
    volatile uint32_t CFGR2;
    volatile uint32_t SWPR;
    volatile uint32_t SKR;
} SYSCFG_TypeDef;

// EXTI
typedef struct {
    volatile uint32_t IMR1;
    volatile uint32_t EMR1;
    volatile uint32_t RTSR1;
    volatile uint32_t FTSR1;
    volatile uint32_t SWIER1;
    volatile uint32_t PR1;
    uint32_t RESERVED1[2];
    volatile uint32_t IMR2;
    volatile uint32_t EMR2;
    volatile uint32_t RTSR2;
    volatile uint32_t FTSR2;
    volatile uint32_t SWIER2;
    volatile uint32_t PR2;
} EXTI_TypeDef;

// NVIC
typedef struct {
    volatile uint32_t ISER[8U];         /*!< Offset: 0x000 (R/W)  Interrupt Set Enable Register */
    uint32_t RESERVED0[24U];
    volatile uint32_t ICER[8U];         /*!< Offset: 0x080 (R/W)  Interrupt Clear Enable Register */
    uint32_t RESERVED1[24U];
    volatile uint32_t ISPR[8U];         /*!< Offset: 0x100 (R/W)  Interrupt Set Pending Register */
    uint32_t RESERVED2[24U];
    volatile uint32_t ICPR[8U];         /*!< Offset: 0x180 (R/W)  Interrupt Clear Pending Register */
    uint32_t RESERVED3[24U];
    volatile uint32_t IABR[8U];         /*!< Offset: 0x200 (R/W)  Interrupt Active bit Register */
    uint32_t RESERVED4[56U];
    volatile uint8_t  IP[240U];         /*!< Offset: 0x300 (R/W)  Interrupt Priority Register (8Bit wide) */
    uint32_t RESERVED5[644U];
    volatile uint32_t STIR;             /*!< Offset: 0xE00 ( /W)  Software Trigger Interrupt Register */
} NVIC_Type;

#define SYSCFG_BASE         (0x40010000UL)
#define EXTI_BASE           (0x40010400UL)
#define NVIC_BASE           (0xE000E100UL)

#define SYSCFG              ((SYSCFG_TypeDef *) SYSCFG_BASE)
#define EXTI                ((EXTI_TypeDef *)   EXTI_BASE)
#define NVIC                ((NVIC_Type *)      NVIC_BASE)

// IRQn enumerations (extracto para STM32L476RG)
typedef enum {
    EXTI15_10_IRQn              = 40,     /*!< External Line[15:10] Interrupts                  */
    USART2_IRQn                 = 38,     /*!< USART2 global Interrupt                          */
} IRQn_Type;

// Constantes
#define RCC_APB2ENR_SYSCFGEN_Pos    (0U)
#define RCC_APB2ENR_SYSCFGEN_Msk    (0x1UL << RCC_APB2ENR_SYSCFGEN_Pos)
#define RCC_APB2ENR_SYSCFGEN        RCC_APB2ENR_SYSCFGEN_Msk

// Prototipos de funciones
void nvic_exti_pc13_button_enable(void); // Configura EXTI13 y habilita su IRQ en NVIC
void nvic_usart2_irq_enable(void);       // Habilita USART2 IRQ en NVIC

#endif // NVIC_H
```

## 5. Código de referencia para implementar (`Src/nvic.c`)

En el archivo `Src/nvic.c` implementa el siguiente código:

```c
#include "nvic.h"
#include "rcc.h"  // Para rcc_syscfg_clock_enable

static void nvic_enable_irq(uint32_t IRQn)
{
    NVIC->ISER[IRQn / 32U] |= (1UL << (IRQn % 32U));
}

void nvic_exti_pc13_button_enable(void) {
    // 1. Habilitar el reloj para SYSCFG
    rcc_syscfg_clock_enable();

    // 2. Configurar la línea EXTI13 (SYSCFG_EXTICR)
    SYSCFG->EXTICR[3] &= ~(0x000FU << 4);  // Limpiar campo EXTI13
    SYSCFG->EXTICR[3] |=  (0x0002U << 4);  // Conectar EXTI13 a PC13

    // 3. Configurar la línea EXTI13 para interrupción
    EXTI->IMR1 |= (1U << 13);

    // 4. Configurar el trigger de flanco de bajada
    EXTI->FTSR1 |= (1U << 13);
    EXTI->RTSR1 &= ~(1U << 13);

    // 5. Habilitar la interrupción EXTI15_10 en el NVIC
    nvic_enable_irq(EXTI15_10_IRQn);
}

void nvic_usart2_irq_enable(void) {
    // Habilitar interrupción de recepción en USART2
    USART2->CR1 |= (1U << 5);  // RXNEIE
    nvic_enable_irq(USART2_IRQn);
}
```

---

## 6. Explicación de funciones

| Función               | Descripción                                                                  |
| --------------------- | ---------------------------------------------------------------------------- |
| `nvic_enable_irq()`   | Función auxiliar para habilitar una IRQ específica en el NVIC.              |
| `nvic_exti_pc13_button_enable()` | Configura SYSCFG para mapear PC13 a EXTI13, habilita IMR y FTSR, y activa la IRQ en NVIC. |
| `nvic_usart2_irq_enable()` | Habilita RXNEIE en USART2 y la IRQ correspondiente en NVIC.                  |

---

## 7. Ejercicios Adicionales

Una vez que hayas implementado la funcionalidad básica NVIC/EXTI:

1. **Configurar prioridades**: Modifica la función para establecer prioridades de interrupción usando `NVIC->IP`.
2. **Demostrar nesting**: Configura la prioridad de la IRQ del botón para que bloquee la del UART, demostrando interrupciones anidadas.

---

## 8. Verificación de Funcionamiento

Para verificar que tu implementación funciona correctamente:

1. **Compilación**: El proyecto debe compilar sin errores.
2. **Integración**: Las funciones deben ser llamadas desde `main.c` y las ISRs deben responder correctamente.
3. **Depuración**: Usa un debugger para verificar que las interrupciones se activen en los eventos correctos.

**Siguiente guía:**
TIM: [TIM_PWM.md](8_TIM_PWM.md)
