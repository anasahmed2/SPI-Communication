/*
 * spi_slave_task.c (Full Duplex Fix)
 */
#include <string.h>
#include <kernel/dpl/DebugP.h>
#include <kernel/dpl/ClockP.h>
#include "FreeRTOS.h"
#include "task.h"
#include "ti_drivers_config.h"
#include "ti_drivers_open_close.h"
#include "ti_board_open_close.h"

#define APP_MCSPI_MSGSIZE   (512U)
#define NUM_PACKETS         (10U) /* 10 Packets */
#define MCSPI_CHANNEL_NUM   (0U) 

uint8_t gSlaveTxBuffer[APP_MCSPI_MSGSIZE] __attribute__((aligned(128)));
uint8_t gSlaveRxBuffer[APP_MCSPI_MSGSIZE] __attribute__((aligned(128)));

void spi_main_task(void *args)
{
    int32_t status;
    MCSPI_Transaction spiTransaction;
    uint32_t pkt;

    Drivers_open();
    Board_driversOpen();
    DebugP_log("[SLAVE] Ready (Full Duplex).\r\n");

    /* Initial Setup: Send 0xAA (Garbage) for first packet */
    memset(gSlaveTxBuffer, 0xAA, APP_MCSPI_MSGSIZE);

    for(pkt = 0; pkt < NUM_PACKETS; pkt++)
    {
        /* Clear RX to clean state */
        memset(gSlaveRxBuffer, 0xCC, APP_MCSPI_MSGSIZE);

        MCSPI_Transaction_init(&spiTransaction);
        spiTransaction.channel = MCSPI_CHANNEL_NUM;
        spiTransaction.count   = APP_MCSPI_MSGSIZE;
        spiTransaction.txBuf   = (void *)gSlaveTxBuffer; // Pre-loaded with prev packet data
        spiTransaction.rxBuf   = (void *)gSlaveRxBuffer; // Receiving new data
        spiTransaction.args    = NULL;

        /* Block until Master Transaction */
        status = MCSPI_transfer(gMcspiHandle[CONFIG_MCSPI0], &spiTransaction);
        
        if(status == SystemP_SUCCESS) {
             /* CRITICAL: Copy received data to TX buffer for NEXT transaction */
             memcpy(gSlaveTxBuffer, gSlaveRxBuffer, APP_MCSPI_MSGSIZE);
             DebugP_log("[SLAVE] Processed Pkt %d\r\n", pkt);
        } else {
             DebugP_log("[SLAVE] Error %d at pkt %d. Resync...\r\n", status, pkt);
             MCSPI_close(gMcspiHandle[CONFIG_MCSPI0]);
             MCSPI_open(CONFIG_MCSPI0, NULL);
             continue;
        }
    }

    DebugP_log("[SLAVE] Done.\r\n");
    Board_driversClose();
    Drivers_close();
    vTaskDelete(NULL);
}
