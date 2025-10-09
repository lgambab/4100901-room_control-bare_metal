# Taller Práctico: Sistema de Control Básico en C Puro (STM32L476RG)

**Universidad Nacional de Colombia - Sede Manizales**
**Curso:** Estructuras Computacionales (4100901)

## Introducción

Este taller tiene como objetivo principal consolidar los conocimientos adquiridos en los capítulos previos sobre la arquitectura del STM32L476RG y el manejo de sus periféricos mediante programación en C puro, accediendo directamente a los registros de hardware.

Replicaremos la funcionalidad del "Sistema de Control Básico" implementado en la práctica introductoria (que utilizaba HAL y STM32CubeMX), pero esta vez, construiremos cada módulo desde cero, basándonos en las implementaciones realizadas en los talleres de SysTick, GPIO, EXTI, TIM/PWM y UART.

**Objetivo General:**
Implementar un sistema embebido que gestione LEDs, lea un botón, controle un LED con PWM y se comunique vía UART, utilizando únicamente acceso directo a registros en C puro.

**Funcionalidades a Implementar:**
1.  **Heartbeat LED:** Parpadeo del LED integrado (LD2) como señal de actividad del sistema.
2.  **Control de LED Externo por Botón:**
    *   Detectar la pulsación del botón de usuario (B1) mediante interrupción externa (EXTI).
    *   Encender un LED externo durante 3 segundos tras la pulsación.
3.  **Comunicación UART:**
    *   Enviar mensajes al PC para indicar eventos (pulsación de botón, timeout del LED).
    *   Implementar el procesamiento de los caracteres recibidos desde el PC como comandos.
4.  **Control PWM de LED Externo:**
    *   Generar una señal PWM utilizando TIM3_CH1 para controlar la intensidad de un segundo LED externo.

## Estructura de la Guía

Esta guía está dividida en varias secciones para facilitar el proceso de desarrollo:

1.  **[SETUP.md](Doc/1_SETUP.md):** Preparación del hardware y el entorno de desarrollo.
2.  **[ASM_CONFIG.md](Doc/2_ASM_CONFIG.md):** Guía de introducción al set de instrucciones de un procesador (ISA) y al lenguaje ensamblador.
3.  **[WORKSHOP_ASM.md](Doc/3_WORKSHOP_ASM.md):** Taller práctico de ensamblador e instrucciones básicas del ISA de ARM.
4.  **[WORKSHOP_C.md](Doc/4_WORKSHOP_C.md):** Taller práctico traduciendo las instrucciones básicas del ISA de ARM a lenguaje C.
5.  **[REFACTOR_TO_LIB.md](Doc/5_REFACTOR_TO_LIB.md):** Refactorización del código a librerías modulares.
6.  **[USART.md](Doc/6_USART.md):** Implementación de comunicación serial.
7.  **[NVIC.md](Doc/7_NVIC.md):** Configuración de interrupciones.
8.  **[TIM_PWM.md](Doc/8_TIM_PWM.md):** Generación de señales PWM.
9.  **[ROOM_CONTROL.md](Doc/9_ROOM_CONTROL.md):** Integración de la lógica de la aplicación.
10. **[MAIN.md](Doc/10_MAIN.md):** Estructura del archivo `main.c`.
11. **[USER_MANUAL.md](Doc/11_USER_MANUAL.md):** Funciones y arquitectura del sistema.

> **Nota:** Cada documento incluye enlaces relativos para facilitar la navegación y referencias cruzadas.

Dirígete primero a [SETUP.md](Doc/1_SETUP.md) para configurar tu entorno de desarrollo.

