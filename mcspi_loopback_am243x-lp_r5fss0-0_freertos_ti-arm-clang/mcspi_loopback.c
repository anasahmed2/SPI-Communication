/*
 * spi_task.c - SLAVE Version
 * Receives data from Master
 */
#include <string.h>
#include <kernel/dpl/DebugP.h>
#include "ti_drivers_config.h"
#include "ti_drivers_open_close.h"
#include "ti_board_open_close.h"

#define APP_MCSPI_MSGSIZE   (100U)
#define MCSPI_CHANNEL_NUM   (0U)

/* Buffers */
uint8_t gSlaveTxBuffer[APP_MCSPI_MSGSIZE] __attribute__((aligned(128)));
uint8_t gSlaveRxBuffer[APP_MCSPI_MSGSIZE] __attribute__((aligned(128)));

void spi_main_task(void *args)
{
    int32_t status;
    MCSPI_Transaction spiTransaction;
    uint32_t i;
    uint32_t errorCount = 0;

    /* Open Drivers */
    Drivers_open();
    Board_driversOpen();

    DebugP_log("[SLAVE] SPI Task Started.\r\n");

    /* 1. Prepare Buffers */
    memset(gSlaveRxBuffer, 0, APP_MCSPI_MSGSIZE);
    memset(gSlaveTxBuffer, 0xAA, APP_MCSPI_MSGSIZE); // Send dummy 0xAA back

    /* 2. Configure Transaction */
    MCSPI_Transaction_init(&spiTransaction);
    spiTransaction.channel   = MCSPI_CHANNEL_NUM;
    spiTransaction.count     = APP_MCSPI_MSGSIZE;
    spiTransaction.txBuf     = (void *)gSlaveTxBuffer;
    spiTransaction.rxBuf     = (void *)gSlaveRxBuffer;
    spiTransaction.args      = NULL;

    DebugP_log("[SLAVE] Ready and Waiting for Master...\r\n");

    /* 3. Wait for Transfer */
    status = MCSPI_transfer(gMcspiHandle[CONFIG_MCSPI0], &spiTransaction);

    if(status == SystemP_SUCCESS) {
        DebugP_log("[SLAVE] Data Received. Verifying...\r\n");

        /* 4. Verify Data */
        for(i = 0; i < APP_MCSPI_MSGSIZE; i++) {
            if(gSlaveRxBuffer[i] != (uint8_t)i) {
                errorCount++;
                if(errorCount < 5) {
                    DebugP_log("  Mismatch at %d: Exp %d, Got %d\r\n", i, i, gSlaveRxBuffer[i]);
                }
            }
        }

        if(errorCount == 0) {
            DebugP_log("[SLAVE] SUCCESS: All bytes match.\r\n");
        } else {
            DebugP_log("[SLAVE] FAILURE: %d errors found.\r\n", errorCount);
        }

    } else {
        DebugP_log("[SLAVE] Transfer Error: %d\r\n", status);
    }

    /* Close Drivers */
    Board_driversClose();
    Drivers_close();
    
    DebugP_log("[SLAVE] Task Exit.\r\n");
}
