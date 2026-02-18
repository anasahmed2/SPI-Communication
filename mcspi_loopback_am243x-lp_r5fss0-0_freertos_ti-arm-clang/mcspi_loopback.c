/*
 * spi_slave_task.c (Receive All, Send All)
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

/* Store received data to send back later */
uint8_t gStorage[NUM_PACKETS][APP_MCSPI_MSGSIZE]; 

void spi_main_task(void *args)
{
    MCSPI_Transaction spiTransaction;
    uint32_t pkt;

    Drivers_open();
    Board_driversOpen();
    
    /* --- PHASE 1: RECEIVE ALL --- */
    DebugP_log("[SLAVE] Ready to Receive.\r\n");

    for(pkt = 0; pkt < NUM_PACKETS; pkt++)
    {
        MCSPI_Transaction_init(&spiTransaction);
        spiTransaction.channel = MCSPI_CHANNEL_NUM;
        spiTransaction.count   = APP_MCSPI_MSGSIZE;
        spiTransaction.txBuf   = (void *)gSlaveTxBuffer; // Don't care
        spiTransaction.rxBuf   = (void *)gSlaveRxBuffer;
        
        if(MCSPI_transfer(gMcspiHandle[CONFIG_MCSPI0], &spiTransaction) == SystemP_SUCCESS) {
            /* Store Data */
            memcpy(gStorage[pkt], gSlaveRxBuffer, APP_MCSPI_MSGSIZE);
            DebugP_log("[SLAVE] Stored Pkt %d\r\n", pkt);
        }
    }

    DebugP_log("[SLAVE] Receive Complete. Switching to Send...\r\n");

    /* --- PHASE 2: SEND ALL --- */
    for(pkt = 0; pkt < NUM_PACKETS; pkt++)
    {
        /* Retrieve Data */
        memcpy(gSlaveTxBuffer, gStorage[pkt], APP_MCSPI_MSGSIZE);

        MCSPI_Transaction_init(&spiTransaction);
        spiTransaction.channel = MCSPI_CHANNEL_NUM;
        spiTransaction.count   = APP_MCSPI_MSGSIZE;
        spiTransaction.txBuf   = (void *)gSlaveTxBuffer;
        spiTransaction.rxBuf   = (void *)gSlaveRxBuffer; // Don't care
        
        if(MCSPI_transfer(gMcspiHandle[CONFIG_MCSPI0], &spiTransaction) == SystemP_SUCCESS) {
            DebugP_log("[SLAVE] Sent Pkt %d\r\n", pkt);
        }
    }
    
    DebugP_log("[SLAVE] Done.\r\n");
    Board_driversClose();
    Drivers_close();
    vTaskDelete(NULL);
}
