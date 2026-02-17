/*
 * spi_task.c - SLAVE Version
 * 1. Receives 512 bytes from Master.
 * 2. Prepares that same data for the next transfer.
 * 3. Sends it back (Echo) when Master clocks again.
 */
#include <string.h>
#include <kernel/dpl/DebugP.h>
#include "ti_drivers_config.h"
#include "ti_drivers_open_close.h"
#include "ti_board_open_close.h"

#define APP_MCSPI_MSGSIZE   (512U)
#define NUM_PACKETS         (1000U)
#define MCSPI_CHANNEL_NUM   (0U) /* Ensure this matches your setup */

uint8_t gSlaveTxBuffer[APP_MCSPI_MSGSIZE] __attribute__((aligned(128)));
uint8_t gSlaveRxBuffer[APP_MCSPI_MSGSIZE] __attribute__((aligned(128)));

void spi_main_task(void *args)
{
    int32_t status;
    MCSPI_Transaction spiTransaction;
    uint32_t pkt;

    Drivers_open();
    Board_driversOpen();

    DebugP_log("[SLAVE] Ready. Waiting for %d packets...\r\n", NUM_PACKETS);

    for(pkt = 0; pkt < NUM_PACKETS; pkt++)
    {
        /* --- STEP 1: Receive Data from Master --- */
        /* Send Dummy (0x00) while receiving, or send previous packet */
        memset(gSlaveTxBuffer, 0, APP_MCSPI_MSGSIZE); 

        MCSPI_Transaction_init(&spiTransaction);
        spiTransaction.channel   = MCSPI_CHANNEL_NUM;
        spiTransaction.count     = APP_MCSPI_MSGSIZE;
        spiTransaction.txBuf     = (void *)gSlaveTxBuffer;
        spiTransaction.rxBuf     = (void *)gSlaveRxBuffer;
        spiTransaction.args      = NULL;

        /* Block until Master sends data */
        status = MCSPI_transfer(gMcspiHandle[CONFIG_MCSPI0], &spiTransaction);
        if(status != SystemP_SUCCESS) {
            DebugP_log("[SLAVE] Error receiving packet %d\r\n", pkt);
            break; 
        }

        /* --- STEP 2: Echo Data Back --- */
        /* Copy received data to TX buffer for the NEXT transaction */
        memcpy(gSlaveTxBuffer, gSlaveRxBuffer, APP_MCSPI_MSGSIZE);

        /* Configure transaction to send the Echo */
        /* We can pass NULL to rxBuf if we don't care what Master sends during echo */
        MCSPI_Transaction_init(&spiTransaction);
        spiTransaction.channel   = MCSPI_CHANNEL_NUM;
        spiTransaction.count     = APP_MCSPI_MSGSIZE;
        spiTransaction.txBuf     = (void *)gSlaveTxBuffer; // Contains data from Step 1
        spiTransaction.rxBuf     = (void *)gSlaveRxBuffer; // Receive Master's dummy data
        
        /* Block until Master clocks out the data */
        status = MCSPI_transfer(gMcspiHandle[CONFIG_MCSPI0], &spiTransaction);
        if(status != SystemP_SUCCESS) {
            DebugP_log("[SLAVE] Error sending echo %d\r\n", pkt);
            break;
        }
    }

    DebugP_log("[SLAVE] Test Finished.\r\n");

    Board_driversClose();
    Drivers_close();
    vTaskDelete(NULL);
}
