/*
 * spi_slave_task.c
 *
 * Role: Slave
 * Behavior:
 * 1. Waits for Master to send data.
 * 2. Copies received data to Transmit buffer.
 * 3. Sends data back (Echo) on the next transaction.
 */

#include <string.h>
#include <kernel/dpl/DebugP.h>
#include "FreeRTOS.h"
#include "task.h"
#include "ti_drivers_config.h"
#include "ti_drivers_open_close.h"
#include "ti_board_open_close.h"

/* --- Configuration --- */
#define APP_MCSPI_MSGSIZE   (512U)
#define NUM_PACKETS         (1000U)
#define MCSPI_CHANNEL_NUM   (0U) /* Usually 0U for single-channel config */

/* Buffers - aligned for cache safety */
uint8_t gSlaveTxBuffer[APP_MCSPI_MSGSIZE] __attribute__((aligned(128)));
uint8_t gSlaveRxBuffer[APP_MCSPI_MSGSIZE] __attribute__((aligned(128)));

void spi_main_task(void *args)
{
    int32_t status;
    MCSPI_Transaction spiTransaction;
    uint32_t pkt;

    /* Open Drivers */
    Drivers_open();
    Board_driversOpen();

    DebugP_log("[SLAVE] SPI Slave Started.\r\n");
    DebugP_log("[SLAVE] Ready. Waiting for %d packets...\r\n", NUM_PACKETS);

    for(pkt = 0; pkt < NUM_PACKETS; pkt++)
    {
        /* ---------------------------------------------------------
         * STEP 1: Receive Data from Master
         * --------------------------------------------------------- */
        
        /* Fill TX with dummy (0) or error pattern (0xCC) during receive */
        memset(gSlaveTxBuffer, 0, APP_MCSPI_MSGSIZE);
        /* Clear RX to ensure we don't read stale data */
        memset(gSlaveRxBuffer, 0xCC, APP_MCSPI_MSGSIZE);

        MCSPI_Transaction_init(&spiTransaction);
        spiTransaction.channel   = MCSPI_CHANNEL_NUM;
        spiTransaction.count     = APP_MCSPI_MSGSIZE;
        spiTransaction.txBuf     = (void *)gSlaveTxBuffer;
        spiTransaction.rxBuf     = (void *)gSlaveRxBuffer;
        spiTransaction.args      = NULL;

        /* Block here until Master sends data */
        status = MCSPI_transfer(gMcspiHandle[CONFIG_MCSPI0], &spiTransaction);
        
        if(status != SystemP_SUCCESS) {
            DebugP_log("[SLAVE] Rx Error %d at packet %d\r\n", status, pkt);
            break; /* Exit loop on error */
        }

        /* ---------------------------------------------------------
         * STEP 2: Echo Data Back
         * --------------------------------------------------------- */
        
        /* Copy the data we just received into the TX buffer */
        memcpy(gSlaveTxBuffer, gSlaveRxBuffer, APP_MCSPI_MSGSIZE);

        /* Configure transaction to send the Echo */
        MCSPI_Transaction_init(&spiTransaction);
        spiTransaction.channel   = MCSPI_CHANNEL_NUM;
        spiTransaction.count     = APP_MCSPI_MSGSIZE;
        spiTransaction.txBuf     = (void *)gSlaveTxBuffer; // Sending Echo
        spiTransaction.rxBuf     = (void *)gSlaveRxBuffer; // Receive Dummy
        
        /* Block here until Master clocks out the data */
        status = MCSPI_transfer(gMcspiHandle[CONFIG_MCSPI0], &spiTransaction);
        
        if(status != SystemP_SUCCESS) {
            DebugP_log("[SLAVE] Tx Error %d at packet %d\r\n", status, pkt);
            break;
        }
    }

    DebugP_log("[SLAVE] Test Finished.\r\n");

    Board_driversClose();
    Drivers_close();
    
    /* Delete Task */
    vTaskDelete(NULL);
}
