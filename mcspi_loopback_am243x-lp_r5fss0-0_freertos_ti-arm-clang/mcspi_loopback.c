/*
 * spi_slave_task.c (Dumb Echo)
 */
#include <string.h>
#include <kernel/dpl/DebugP.h>
#include "FreeRTOS.h"
#include "task.h"
#include "ti_drivers_config.h"
#include "ti_drivers_open_close.h"
#include "ti_board_open_close.h"

#define APP_MCSPI_MSGSIZE   (512U)
#define MCSPI_CHANNEL_NUM   (0U) 

uint8_t gSlaveTxBuffer[APP_MCSPI_MSGSIZE] __attribute__((aligned(128)));
uint8_t gSlaveRxBuffer[APP_MCSPI_MSGSIZE] __attribute__((aligned(128)));

void spi_main_task(void *args)
{
    MCSPI_Transaction spiTransaction;
    Drivers_open();
    Board_driversOpen();
    
    DebugP_log("[SLAVE] Ready. Sending 0x55 forever.\r\n");

    /* Fill TX with 0x55 */
    memset(gSlaveTxBuffer, 0x55, APP_MCSPI_MSGSIZE);

    while(1) {
        MCSPI_Transaction_init(&spiTransaction);
        spiTransaction.channel = MCSPI_CHANNEL_NUM;
        spiTransaction.count   = APP_MCSPI_MSGSIZE;
        spiTransaction.txBuf   = (void *)gSlaveTxBuffer;
        spiTransaction.rxBuf   = (void *)gSlaveRxBuffer;
        
        /* Block until Master clocks */
        MCSPI_transfer(gMcspiHandle[CONFIG_MCSPI0], &spiTransaction);
        
        /* If we get here, a transfer happened. Just reload and wait again. */
        /* Optional: Toggle an LED here if you have one to prove life */
    }
}
