# Refactorización: Uso de Librerías para Periféricos

# Refactorización: Uso de Librerías para Periféricos

> **Referencia:**
> Zhu, Yifen. *Embedded Systems with ARM Cortex-M Microcontrollers in Assembly Language and C*, 2nd Edition, 2022. ***Capítulo 8 recomendado para esta sección***

En esta guía aprenderás a modularizar tu proyecto STM32 usando librerías para los periféricos principales.

Esto mejora la organización y reutilización del código, especialmente cuando el código se hace más grande.

## 1. Estructura Recomendada

```
Inc/
  gpio.h
  rcc.h
  systick.h

Src/
  main.c    ### programa principal
  gpio.c
  rcc.c
  systick.c
```

## 2. Ejemplo de Uso en `main.c`

```c
#include "gpio.h"
#include "systick.h"
#include "rcc.h"

static volatile uint32_t ms_counter = 17;

// --- Programa principal ------------------------------------------------------
int main(void)
{
    rcc_init();
    init_led();
    init_button();
    init_systick();

    while (1) {
        if (read_button() != 0) { // Botón presionado
            ms_counter = 0;   // reiniciar el contador de milisegundos
            set_led();        // Encender LED
        }
        
        if (ms_counter >= 3000) { // Si han pasado 3 segundos o más, apagar LED
            clear_led();             // Apagar LED
        }
    }
}

// --- Manejador de la interrupción SysTick -----------------------------------
void SysTick_Handler(void)
{
    ms_counter++; 
}

```

## 3. Implementación de Librerías

### gpio.c
- Inicializa y controla pines digitales segun las funciones declaradas en `gpio.h`.

```c
// gpio.h
#include <stdint.h>

void init_led(void);
void init_button(void);

void set_led(void);
void clear_led(void);
uint8_t read_button(void);

```

### systick.c
- Configura el temporizador SysTick para contador de milisegundos segun `systick.h`.
```c
// systick.h
#include <stdint.h>

void init_systick(void);

```

### rcc.c
- Configura los relojes del sistema segun `rcc.h`.
```c
// rcc.h
#include <stdint.h>

void rcc_init(void);

```

## 4. Ejercicio

Refactoriza el código del taller en lenguaje C para usar estas librerías.


---

> Esta estructura facilita la comprensión y el mantenimiento del código en proyectos embebidos.

**Siguiente guía:**
Comunicación UART: [USART.md](6_USART.md)
