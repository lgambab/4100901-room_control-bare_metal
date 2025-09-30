#include "systick.h"

#define SYST_BASE   0xE000E010U                                 // SysTick base
#define SYST_CSR    (*(volatile uint32_t *)(SYST_BASE + 0x00U)) // Control y estado
#define SYST_RVR    (*(volatile uint32_t *)(SYST_BASE + 0x04U)) // Valor de recarga
#define HSI_FREQ    4000000U                                   // Reloj interno 4 MHz

void init_systick(void);
{
    SYST_RVR = HSI_FREQ - 1;                      // Recarga = 4000000 - 1
    SYST_CSR = (1 << 0) | (1 << 1) | (1 << 2);    // ENABLE|TICKINT|CLKSOURCE
}