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