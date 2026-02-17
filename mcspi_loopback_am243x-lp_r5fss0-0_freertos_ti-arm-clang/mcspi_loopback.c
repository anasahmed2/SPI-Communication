/*
 * spi_slave_task.c (Simple Echo Loop)
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
    
    DebugP_log("[SLAVE] Ready. Echo Loop Running.\r\n");

    /* Initial TX: 0xAA */
    memset(gSlaveTxBuffer, 0xAA, APP_MCSPI_MSGSIZE);

    while(1) {
        MCSPI_Transaction_init(&spiTransaction);
        spiTransaction.channel = MCSPI_CHANNEL_NUM;
        spiTransaction.count   = APP_MCSPI_MSGSIZE;
        spiTransaction.txBuf   = (void *)gSlaveTxBuffer;
        spiTransaction.rxBuf   = (void *)gSlaveRxBuffer;
        
        /* Block until transfer complete */
        if(MCSPI_transfer(gMcspiHandle[CONFIG_MCSPI0], &spiTransaction) == SystemP_SUCCESS) {
            /* Copy RX to TX for NEXT transfer */
            memcpy(gSlaveTxBuffer, gSlaveRxBuffer, APP_MCSPI_MSGSIZE);
        } else {
            /* Reset on error */
            MCSPI_close(gMcspiHandle[CONFIG_MCSPI0]);
            MCSPI_open(CONFIG_MCSPI0, NULL);
        }
    }
}
