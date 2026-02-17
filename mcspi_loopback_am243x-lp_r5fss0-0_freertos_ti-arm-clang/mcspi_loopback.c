/*
 * spi_slave_task.c (Raw Print)
 */
#include <string.h>
#include <kernel/dpl/DebugP.h>
#include "FreeRTOS.h"
#include "task.h"
#include "ti_drivers_config.h"
#include "ti_drivers_open_close.h"
#include "ti_board_open_close.h"

#define APP_MCSPI_MSGSIZE   (64U)
#define NUM_PACKETS         (10U)
#define MCSPI_CHANNEL_NUM   (0U) 

uint8_t gSlaveTxBuffer[APP_MCSPI_MSGSIZE] __attribute__((aligned(128)));
uint8_t gSlaveRxBuffer[APP_MCSPI_MSGSIZE] __attribute__((aligned(128)));

void spi_main_task(void *args)
{
    MCSPI_Transaction spiTransaction;
    uint32_t pkt;

    Drivers_open();
    Board_driversOpen();
    
    DebugP_log("[SLAVE] Ready (Printing Raw Data).\r\n");

    for(pkt = 0; pkt < NUM_PACKETS; pkt++)
    {
        /* Clear Buffer with 0xCC (Dummy) */
        memset(gSlaveRxBuffer, 0xCC, APP_MCSPI_MSGSIZE);

        MCSPI_Transaction_init(&spiTransaction);
        spiTransaction.channel = MCSPI_CHANNEL_NUM;
        spiTransaction.count   = APP_MCSPI_MSGSIZE;
        spiTransaction.txBuf   = (void *)gSlaveTxBuffer;
        spiTransaction.rxBuf   = (void *)gSlaveRxBuffer;
        
        if(MCSPI_transfer(gMcspiHandle[CONFIG_MCSPI0], &spiTransaction) == SystemP_SUCCESS) {
            /* Print first 4 bytes */
            DebugP_log("[SLAVE] Pkt %d: %02X %02X %02X %02X\r\n", 
                       pkt, gSlaveRxBuffer[0], gSlaveRxBuffer[1], gSlaveRxBuffer[2], gSlaveRxBuffer[3]);
        } else {
            DebugP_log("[SLAVE] Error.\r\n");
            MCSPI_close(gMcspiHandle[CONFIG_MCSPI0]);
            MCSPI_open(CONFIG_MCSPI0, NULL);
        }
    }
    
    DebugP_log("[SLAVE] Done.\r\n");
    Board_driversClose();
    Drivers_close();
    vTaskDelete(NULL);
}
