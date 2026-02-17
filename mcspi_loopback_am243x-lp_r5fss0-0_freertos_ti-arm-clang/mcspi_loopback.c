/*
 * spi_slave_task.c (Half Duplex)
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
    DebugP_log("[SLAVE] Ready (Half Duplex).\r\n");

    for(pkt = 0; pkt < NUM_PACKETS; pkt++)
    {
        /* --- STEP 1: RECEIVE DATA --- */
        memset(gSlaveRxBuffer, 0xCC, APP_MCSPI_MSGSIZE);
        
        MCSPI_Transaction_init(&spiTransaction);
        spiTransaction.channel = MCSPI_CHANNEL_NUM;
        spiTransaction.count   = APP_MCSPI_MSGSIZE;
        spiTransaction.txBuf   = (void *)gSlaveTxBuffer; // Don't care
        spiTransaction.rxBuf   = (void *)gSlaveRxBuffer; 
        spiTransaction.args    = NULL;

        status = MCSPI_transfer(gMcspiHandle[CONFIG_MCSPI0], &spiTransaction);
        if(status != SystemP_SUCCESS) {
             DebugP_log("Rx Error %d\r\n", status);
             MCSPI_close(gMcspiHandle[CONFIG_MCSPI0]);
             MCSPI_open(CONFIG_MCSPI0, NULL);
             continue;
        }

        /* --- STEP 2: SEND ECHO --- */
        /* Copy data to TX buffer */
        memcpy(gSlaveTxBuffer, gSlaveRxBuffer, APP_MCSPI_MSGSIZE);
        
        MCSPI_Transaction_init(&spiTransaction);
        spiTransaction.channel = MCSPI_CHANNEL_NUM;
        spiTransaction.count   = APP_MCSPI_MSGSIZE;
        spiTransaction.txBuf   = (void *)gSlaveTxBuffer; // Send Echo
        spiTransaction.rxBuf   = (void *)gSlaveRxBuffer; // Don't care
        
        status = MCSPI_transfer(gMcspiHandle[CONFIG_MCSPI0], &spiTransaction);
        if(status != SystemP_SUCCESS) {
             DebugP_log("Tx Error %d\r\n", status);
             continue;
        }
    }

    DebugP_log("[SLAVE] Done.\r\n");
    Board_driversClose();
    Drivers_close();
    vTaskDelete(NULL);
}
