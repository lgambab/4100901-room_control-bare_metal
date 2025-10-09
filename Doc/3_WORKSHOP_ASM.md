# WORKSHOP_ASM.md
> **Referencia:**
> Zhu, Yifen. *Embedded Systems with ARM Cortex-M Microcontrollers in Assembly Language and C*, 2nd Edition, 2022. ***Capítulos 7 y 14 recomendados para esta sección***

## Taller: Systick y Parpadeo de LED como Funciones

**Objetivo:**  
Configurar el **Systick** con el reloj por defecto (4 MHz) para generar una interrupción cada **1 s**, Usar un LED como indicador del sistema y un botón para controlar el LED; implementar las secciones lógicas como funciones ASM sencillas y usar **macros** para todas las direcciones de registro.

### 1. Descripción del programa existente

0. Los macros definen las direcciones de memoria a las que se va a leer y escribir [[Ver RM0351, sección 2.2.2 y tabla 1](https://www.st.com/resource/en/reference_manual/rm0351-stm32l47xxx-stm32l48xxx-stm32l49xxx-and-stm32l4axxx-advanced-armbased-32bit-mcus-stmicroelectronics.pdf)].
1. **init_gpio** configura **PA5** como salida push-pull para controlar el LED LD2 [[Ver RM0351, secciones 6.4.17 y 8.5.1](https://www.st.com/resource/en/reference_manual/rm0351-stm32l47xxx-stm32l48xxx-stm32l49xxx-and-stm32l4axxx-advanced-armbased-32bit-mcus-stmicroelectronics.pdf)].
2. **init_systick** configura para que el Systick interrumpa cada **1 s** (AHB a 4 MHz) [[Ver PM0214, Sección 4.5](https://www.st.com/resource/en/programming_manual/pm0214-stm32-cortexm4-mcus-and-mpus-programming-manual-stmicroelectronics.pdf)].
3. En el **SysTick_Handler**, alternar el LED: **toggle_pa5** [[Ver RM0351, sección 8.5.6](https://www.st.com/resource/en/reference_manual/rm0351-stm32l47xxx-stm32l48xxx-stm32l49xxx-and-stm32l4axxx-advanced-armbased-32bit-mcus-stmicroelectronics.pdf)].  

---


---

### 2. Creación y configuración del archivo Assembly

1. **Crear el archivo de código**: en la carpeta `Src/`, crea un archivo llamado `workshop.s`.
2. **Incluir en la compilación**: abre `cmake/vscode_generated.cmake` y agrega `Src/workshop.s` a la lista de fuentes del ejecutable.
3. **Agregar el código de ejemplo**: copia el bloque de la sección siguiente y pégalo en `Src/workshop.s`.

---

### 3. Código de ejemplo (`Src/workshop.s`)

Compila y depura el siguiente código en tu NUCLEO_L476RG y analiza la funcionalidad actual.
```assembly
// --- Ejemplo de parpadeo de LED LD2 en STM32F476RGTx -------------------------
    .section .text
    .syntax unified
    .thumb

    .global main
    .global init_led
    .global init_systick
    .global SysTick_Handler

// --- Definiciones de registros para LD2 (Ver RM0351) -------------------------
    .equ RCC_BASE,       0x40021000         @ Base de RCC
    .equ RCC_AHB2ENR,    RCC_BASE + 0x4C    @ Enable GPIOA clock (AHB2ENR)
    .equ GPIOA_BASE,     0x48000000         @ Base de GPIOA
    .equ GPIOA_MODER,    GPIOA_BASE + 0x00  @ Mode register
    .equ GPIOA_ODR,      GPIOA_BASE + 0x14  @ Output data register
    .equ LD2_PIN,        5                  @ Pin del LED LD2

// --- Definiciones de registros para SysTick (Ver PM0214) ---------------------
    .equ SYST_CSR,       0xE000E010         @ Control and status
    .equ SYST_RVR,       0xE000E014         @ Reload value register
    .equ SYST_CVR,       0xE000E018         @ Current value register
    .equ HSI_FREQ,       4000000            @ Reloj interno por defecto (4 MHz)

// --- Programa principal ------------------------------------------------------
main:
    bl init_led
    bl init_systick
loop:
    wfi
    b loop

// --- Inicialización de GPIOA PA5 para el LED LD2 -----------------------------
init_led:
    movw  r0, #:lower16:RCC_AHB2ENR
    movt  r0, #:upper16:RCC_AHB2ENR
    ldr   r1, [r0]
    orr   r1, r1, #(1 << 0)                @ Habilita reloj GPIOA
    str   r1, [r0]

    movw  r0, #:lower16:GPIOA_MODER
    movt  r0, #:upper16:GPIOA_MODER
    ldr   r1, [r0]
    bic   r1, r1, #(0b11 << (LD2_PIN * 2)) @ Limpia bits MODER5
    orr   r1, r1, #(0b01 << (LD2_PIN * 2)) @ PA5 como salida
    str   r1, [r0]
    bx    lr

// --- Inicialización de Systick para 1 s --------------------------------------
init_systick:
    movw  r0, #:lower16:SYST_RVR
    movt  r0, #:upper16:SYST_RVR
    movw  r1, #:lower16:HSI_FREQ
    movt  r1, #:upper16:HSI_FREQ
    subs  r1, r1, #1                       @ reload = 4 000 000 - 1
    str   r1, [r0]

    movw  r0, #:lower16:SYST_CSR
    movt  r0, #:upper16:SYST_CSR
    movs  r1, #(1 << 0)|(1 << 1)|(1 << 2)  @ ENABLE=1, TICKINT=1, CLKSOURCE=1
    str   r1, [r0]
    bx    lr

// --- Manejador de la interrupción SysTick ------------------------------------
    .thumb_func                            @ Ensure Thumb bit
SysTick_Handler:
    movw  r0, #:lower16:GPIOA_ODR
    movt  r0, #:upper16:GPIOA_ODR
    ldr   r1, [r0]
    eor   r1, r1, #(1 << LD2_PIN)          @ toggle PA5
    str   r1, [r0]
    bx    lr

```

> **Nota sobre el retorno de la ISR:**  
> - En un manejador de excepción (como SysTick), el registro **LR** contiene un valor especial **EXC_RETURN** que indica al procesador cómo restaurar el contexto.  
> - La instrucción **`bx lr`** es la forma correcta de salir de la ISR y regresar a la ejecución previa.  
> - No uses `b` ni `bl` para este propósito, ya que no restaurarían correctamente el estado de la pila y registros del procesador.

---

### 3. Ejercicio 

#### Parte 1: Botón y LED

En esta primera parte, extiende el código base para:

1. Configurar **PC13** (botón B1 en Nucleo‑L476RG) como entrada.
2. Cuando se detecte una pulsación, **encender el LED LD2** en **PA5**.
3. Mantener el LED encendido durante **3 segundos** utilizando **SysTick** o un contador basado en `SYST_CVR`.
4. Apagar el LED después de que pasen los **3 segundos**.

> **Pistas:**
> - Implemente una función que inicialice el boton de forma similar a la del LED LD2.
> - Implemente un función que lea el estado del boton (usando el registro IDR del puerto de botón) y encienda el LED (debe correr en el loop infinito).
> - Puedes reutilizar la configuración de SysTick para generar ticks de millisegundos o segundos para contar el tiempo usando una variable en memoria.

#### Parte 2: Documentación

Después de implementar la lógica, prepara una **presentación** de tu trabajo donde muestres:

- Descripción del flujo del programa usando diagramas de flujo (se recomienda en [mermaid](https://mermaid.live/)).
- Reflexión sobre posibles mejoras o dificultades encontradas.

---

### 4. Entrega

- Sube el enlace de Git con los cambios, o el archivo assembly modificado (`Src/main.s`), la documentación, y el enlace del video en Youtube a la tarea del classroom.

¡Éxito con el taller!

---

**Siguiente guía:**
Continúa con la guía de C: [WORKSHOP_C.md](4_WORKSHOP_C.md)
