/*
 * spi_slave_task.c (Handshake)
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

/* Storage for 10 Packets */
uint8_t gStorage[NUM_PACKETS][APP_MCSPI_MSGSIZE]; 

void spi_main_task(void *args)
{
    MCSPI_Transaction spiTransaction;
    uint32_t pkt = 0;
    int phase1_done = 0;

    Drivers_open();
    Board_driversOpen();
    
    /* --- PHASE 1: RECEIVE UNTIL SIGNAL --- */
    DebugP_log("[SLAVE] Ready to Receive.\r\n");

    while(!phase1_done)
    {
        MCSPI_Transaction_init(&spiTransaction);
        spiTransaction.channel = MCSPI_CHANNEL_NUM;
        spiTransaction.count   = APP_MCSPI_MSGSIZE;
        spiTransaction.txBuf   = (void *)gSlaveTxBuffer; 
        spiTransaction.rxBuf   = (void *)gSlaveRxBuffer;
        
        if(MCSPI_transfer(gMcspiHandle[CONFIG_MCSPI0], &spiTransaction) == SystemP_SUCCESS) 
        {
            /* CHECK SIGNAL: First byte 0xFF? */
            if(gSlaveRxBuffer[0] == 0xFF) {
                DebugP_log("[SLAVE] Received Signal (0xFF). Switching Mode.\r\n");
                phase1_done = 1;
                break;
            }

            /* Store Data */
            if(pkt < NUM_PACKETS) {
                memcpy(gStorage[pkt], gSlaveRxBuffer, APP_MCSPI_MSGSIZE);
                
                /* Print received data */
                DebugP_log("[SLAVE] Rx Pkt %d: %02X %02X %02X %02X\r\n", 
                           pkt, gSlaveRxBuffer[0], gSlaveRxBuffer[1], gSlaveRxBuffer[2], gSlaveRxBuffer[3]);
                pkt++;
            }
        }
    }

    /* --- PHASE 2: SEND ALL BACK --- */
    DebugP_log("[SLAVE] PHASE 2: Sending Echo...\r\n");

    for(int i = 0; i < pkt; i++) // Send back however many we received
    {
        /* Retrieve Data */
        memcpy(gSlaveTxBuffer, gStorage[i], APP_MCSPI_MSGSIZE);

        MCSPI_Transaction_init(&spiTransaction);
        spiTransaction.channel = MCSPI_CHANNEL_NUM;
        spiTransaction.count   = APP_MCSPI_MSGSIZE;
        spiTransaction.txBuf   = (void *)gSlaveTxBuffer;
        spiTransaction.rxBuf   = (void *)gSlaveRxBuffer; 
        
        MCSPI_transfer(gMcspiHandle[CONFIG_MCSPI0], &spiTransaction);
        DebugP_log("[SLAVE] Sent Echo Pkt %d\r\n", i);
    }
    
    DebugP_log("[SLAVE] Done.\r\n");
    Board_driversClose();
    Drivers_close();
    vTaskDelete(NULL);
}
