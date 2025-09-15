
    .section .text
    .syntax unified
    .thumb

    .global main
    .global init_led
    .global init_button
    .global delay_3s

// --- Direcciones de registros ------------------------------------------------
    .equ RCC_BASE,       0x40021000         @ Base de RCC
    .equ RCC_AHB2ENR,    RCC_BASE + 0x4C    @ Enable GPIOx clock (AHB2ENR)

    .equ GPIOA_BASE,     0x48000000
    .equ GPIOA_MODER,    GPIOA_BASE + 0x00
    .equ GPIOA_ODR,      GPIOA_BASE + 0x14

    .equ GPIOC_BASE,     0x48000800
    .equ GPIOC_MODER,    GPIOC_BASE + 0x00
    .equ GPIOC_IDR,      GPIOC_BASE + 0x10

    .equ LD2_PIN,        5                  @ PA5
    .equ B1_PIN,         13                 @ PC13

// --- SysTick -----------------------------------------------------------------
    .equ SYST_CSR,       0xE000E010
    .equ SYST_RVR,       0xE000E014
    .equ SYST_CVR,       0xE000E018
    .equ HSI_FREQ,       4000000            @ 4 MHz

// --- Programa principal ------------------------------------------------------
main:
    bl init_led
    bl init_button

loop:
    @ Leer botón PC13
    movw  r0, #:lower16:GPIOC_IDR
    movt  r0, #:upper16:GPIOC_IDR
    ldr   r1, [r0]
    tst   r1, #(1 << B1_PIN)        @ Verificar bit 13
    bne   loop                      @ Si está en alto -> no presionado

    @ Botón presionado -> encender LED
    movw  r0, #:lower16:GPIOA_ODR
    movt  r0, #:upper16:GPIOA_ODR
    ldr   r1, [r0]
    orr   r1, r1, #(1 << LD2_PIN)
    str   r1, [r0]

    @ Esperar 3 segundos
    bl delay_3s

    @ Apagar LED
    @por lo visto el delay genera que l direccion de ro cambie y por ende no funcione como se espera
    movw  r0, #:lower16:GPIOA_ODR
    movt  r0, #:upper16:GPIOA_ODR
    ldr   r1, [r0]
    bic   r1, r1, #(1 << LD2_PIN)
    str   r1, [r0]

    b loop

// --- Inicialización LED PA5 --------------------------------------------------
init_led:
    @ Habilitar reloj GPIOA
    movw  r0, #:lower16:RCC_AHB2ENR
    movt  r0, #:upper16:RCC_AHB2ENR
    ldr   r1, [r0]
    orr   r1, r1, #(1 << 0)          @ bit0 -> GPIOAEN
    str   r1, [r0]

    @ Configurar PA5 como salida
    movw  r0, #:lower16:GPIOA_MODER
    movt  r0, #:upper16:GPIOA_MODER
    ldr   r1, [r0]
    bic   r1, r1, #(0b11 << (LD2_PIN*2))
    orr   r1, r1, #(0b01 << (LD2_PIN*2))
    str   r1, [r0]
    bx    lr

// --- Inicialización botón PC13 -----------------------------------------------
init_button:
    @ Habilitar reloj GPIOC
    movw  r0, #:lower16:RCC_AHB2ENR
    movt  r0, #:upper16:RCC_AHB2ENR
    ldr   r1, [r0]
    orr   r1, r1, #(1 << 2)          @ bit2 -> GPIOCEN
    str   r1, [r0]

    @ Configurar PC13 como entrada (00)
    movw  r0, #:lower16:GPIOC_MODER
    movt  r0, #:upper16:GPIOC_MODER
    ldr   r1, [r0]
    bic   r1, r1, #(0b11 << (B1_PIN*2))
    str   r1, [r0]
    bx    lr

// --- Retardo de 3 segundos usando SysTick ------------------------------------
delay_3s:
    @ Configurar reload = 4 MHz - 1 (1 s)
    movw  r0, #:lower16:SYST_RVR
    movt  r0, #:upper16:SYST_RVR
    movw  r1, #(HSI_FREQ - 1) & 0xFFFF
    movt  r1, #((HSI_FREQ - 1) >> 16)
    str   r1, [r0]

    movw  r0, #:lower16:SYST_CSR
    movt  r0, #:upper16:SYST_CSR
    movs  r1, #(1 << 0)|(1 << 2)     @ ENABLE=1, CLKSOURCE=1 (sin interrupción)
    str   r1, [r0]

    movs  r2, #3                     @ contador de 3 segundos

wait_loop:
    @ Esperar que COUNTFLAG=1 (bit16 en CSR)
    movw  r0, #:lower16:SYST_CSR
    movt  r0, #:upper16:SYST_CSR

poll:
    ldr   r1, [r0]
    tst   r1, #(1 << 16)
    beq   poll                       @ esperar hasta que expire 1 s
    subs  r2, r2, #1
    bne   wait_loop

    bx    lr
