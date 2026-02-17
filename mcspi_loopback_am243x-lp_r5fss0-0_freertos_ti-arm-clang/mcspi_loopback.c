/*
 * spi_slave_task.c
 * - Reduced to 100 packets.
 * - Added continue on error instead of break.
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
#define NUM_PACKETS         (100U) /* REDUCED to 100 */
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

    DebugP_log("[SLAVE] Ready (100 Packets).\r\n");

    for(pkt = 0; pkt < NUM_PACKETS; pkt++)
    {
        /* STEP 1: Receive Data */
        memset(gSlaveTxBuffer, 0, APP_MCSPI_MSGSIZE);
        memset(gSlaveRxBuffer, 0xCC, APP_MCSPI_MSGSIZE);

        MCSPI_Transaction_init(&spiTransaction);
        spiTransaction.channel   = MCSPI_CHANNEL_NUM;
        spiTransaction.count     = APP_MCSPI_MSGSIZE;
        spiTransaction.txBuf     = (void *)gSlaveTxBuffer;
        spiTransaction.rxBuf     = (void *)gSlaveRxBuffer;
        spiTransaction.args      = NULL;

        status = MCSPI_transfer(gMcspiHandle[CONFIG_MCSPI0], &spiTransaction);
        if(status != SystemP_SUCCESS) {
            DebugP_log("[SLAVE] Rx Error %d at pkt %d. Resyncing...\r\n", status, pkt);
            /* RECOVERY: If we missed the Master, just loop back and wait for the NEXT one.
               The Master will likely error out on this pkt too, but we will catch the next one. */
            MCSPI_close(gMcspiHandle[CONFIG_MCSPI0]); // Optional: Reset driver
            MCSPI_open(CONFIG_MCSPI0, NULL);          // Optional: Re-open
            continue; 
        }

        /* STEP 2: Echo Data */
        memcpy(gSlaveTxBuffer, gSlaveRxBuffer, APP_MCSPI_MSGSIZE);

        MCSPI_Transaction_init(&spiTransaction);
        spiTransaction.channel   = MCSPI_CHANNEL_NUM;
        spiTransaction.count     = APP_MCSPI_MSGSIZE;
        spiTransaction.txBuf     = (void *)gSlaveTxBuffer; 
        spiTransaction.rxBuf     = (void *)gSlaveRxBuffer; 
        
        status = MCSPI_transfer(gMcspiHandle[CONFIG_MCSPI0], &spiTransaction);
        if(status != SystemP_SUCCESS) {
            DebugP_log("[SLAVE] Tx Error %d at pkt %d\r\n", status, pkt);
            continue;
        }
    }

    DebugP_log("[SLAVE] Finished.\r\n");

    Board_driversClose();
    Drivers_close();
    vTaskDelete(NULL);
}
