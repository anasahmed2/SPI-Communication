/*
 * spi_slave_task.c (Sync + Half Duplex)
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
    int synced = 0;

    Drivers_open();
    Board_driversOpen();
    DebugP_log("[SLAVE] Waiting for Sync...\r\n");

    /* SYNC PHASE */
    while(!synced) {
        memset(gSlaveTxBuffer, 0xAA, APP_MCSPI_MSGSIZE); // Tell Master we are here
        memset(gSlaveRxBuffer, 0x00, APP_MCSPI_MSGSIZE);

        MCSPI_Transaction_init(&spiTransaction);
        spiTransaction.channel = MCSPI_CHANNEL_NUM;
        spiTransaction.count   = APP_MCSPI_MSGSIZE;
        spiTransaction.txBuf   = (void *)gSlaveTxBuffer;
        spiTransaction.rxBuf   = (void *)gSlaveRxBuffer;
        
        status = MCSPI_transfer(gMcspiHandle[CONFIG_MCSPI0], &spiTransaction);
        
        /* Check if Master sent 0xFF */
        if(status == SystemP_SUCCESS && gSlaveRxBuffer[0] == 0xFF) {
            synced = 1;
            /* DO NOT LOG HERE - it takes too long. Just switch to test mode. */
        }
    }

    /* TEST PHASE */
    for(pkt = 0; pkt < NUM_PACKETS; pkt++)
    {
        /* STEP 1: RECEIVE DATA */
        memset(gSlaveTxBuffer, 0x00, APP_MCSPI_MSGSIZE); // Dummy
        memset(gSlaveRxBuffer, 0xCC, APP_MCSPI_MSGSIZE);
        
        MCSPI_Transaction_init(&spiTransaction);
        spiTransaction.channel = MCSPI_CHANNEL_NUM;
        spiTransaction.count   = APP_MCSPI_MSGSIZE;
        spiTransaction.txBuf   = (void *)gSlaveTxBuffer;
        spiTransaction.rxBuf   = (void *)gSlaveRxBuffer; 
        
        status = MCSPI_transfer(gMcspiHandle[CONFIG_MCSPI0], &spiTransaction);
        if(status != SystemP_SUCCESS) {
             MCSPI_close(gMcspiHandle[CONFIG_MCSPI0]);
             MCSPI_open(CONFIG_MCSPI0, NULL);
             continue;
        }

        /* STEP 2: SEND ECHO */
        memcpy(gSlaveTxBuffer, gSlaveRxBuffer, APP_MCSPI_MSGSIZE);
        
        MCSPI_Transaction_init(&spiTransaction);
        spiTransaction.channel = MCSPI_CHANNEL_NUM;
        spiTransaction.count   = APP_MCSPI_MSGSIZE;
        spiTransaction.txBuf   = (void *)gSlaveTxBuffer; 
        spiTransaction.rxBuf   = (void *)gSlaveRxBuffer; 
        
        status = MCSPI_transfer(gMcspiHandle[CONFIG_MCSPI0], &spiTransaction);
    }

    DebugP_log("[SLAVE] Done.\r\n");
    Board_driversClose();
    Drivers_close();
    vTaskDelete(NULL);
}
