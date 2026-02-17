/*
 * spi_slave_task.c (Debug 0xAA)
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
#define NUM_PACKETS         (10U)
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
    DebugP_log("[SLAVE] Ready (Sending 0xAA).\r\n");

    /* Pre-fill TX buffer with 0xAA once */
    memset(gSlaveTxBuffer, 0xAA, APP_MCSPI_MSGSIZE);

    for(pkt = 0; pkt < NUM_PACKETS; pkt++)
    {
        /* Clear RX to verify we are receiving master data */
        memset(gSlaveRxBuffer, 0x00, APP_MCSPI_MSGSIZE);

        MCSPI_Transaction_init(&spiTransaction);
        spiTransaction.channel = MCSPI_CHANNEL_NUM;
        spiTransaction.count   = APP_MCSPI_MSGSIZE;
        spiTransaction.txBuf   = (void *)gSlaveTxBuffer; // Always 0xAA
        spiTransaction.rxBuf   = (void *)gSlaveRxBuffer;
        spiTransaction.args    = NULL;

        /* Wait for transfer */
        status = MCSPI_transfer(gMcspiHandle[CONFIG_MCSPI0], &spiTransaction);
        
        if(status == SystemP_SUCCESS) {
             /* Check first byte just for log */
             DebugP_log("[SLAVE] Rx Pkt %d. First Byte: %d\r\n", pkt, gSlaveRxBuffer[0]);
        } else {
             DebugP_log("[SLAVE] Error %d at pkt %d.\r\n", status, pkt);
             /* Resync attempt */
             MCSPI_close(gMcspiHandle[CONFIG_MCSPI0]);
             MCSPI_open(CONFIG_MCSPI0, NULL);
        }
    }

    DebugP_log("[SLAVE] Done.\r\n");
    Board_driversClose();
    Drivers_close();
    vTaskDelete(NULL);
}
