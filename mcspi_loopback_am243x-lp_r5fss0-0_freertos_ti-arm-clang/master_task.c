/*
 * spi_master_task.c (Polling for Ready)
 */
#include <string.h>
#include <kernel/dpl/DebugP.h>
#include <kernel/dpl/ClockP.h>
#include "FreeRTOS.h"
#include "task.h"
#include "ti_drivers_config.h"
#include "ti_drivers_open_close.h"
#include "ti_board_open_close.h"

#define APP_MCSPI_MSGSIZE   (32U)
#define NUM_PACKETS         (1000U) 
#define MCSPI_CHANNEL_NUM   (1U) 

uint8_t gTxBuffer[APP_MCSPI_MSGSIZE] __attribute__((aligned(128)));
uint8_t gRxBuffer[APP_MCSPI_MSGSIZE] __attribute__((aligned(128)));

void spi_main_task(void *args)
{
    MCSPI_Transaction spiTransaction;
    uint32_t i, pkt;
    uint32_t okCount = 0;
    uint32_t errorCount = 0;
    uint64_t startTime, endTime, totalTime;

    Drivers_open();
    Board_driversOpen();

    startTime = ClockP_getTimeUsec();

    /* --- PHASE 1: SEND DATA --- */
    DebugP_log("[MASTER] Sending %d Packets...\r\n", NUM_PACKETS);
    ClockP_sleep(5);

    for(pkt = 0; pkt < NUM_PACKETS; pkt++)
    {
        for(i=0; i<APP_MCSPI_MSGSIZE; i++) gTxBuffer[i] = (uint8_t)(pkt + i);

        MCSPI_Transaction_init(&spiTransaction);
        spiTransaction.channel = MCSPI_CHANNEL_NUM;
        spiTransaction.count   = APP_MCSPI_MSGSIZE;
        spiTransaction.txBuf   = (void *)gTxBuffer;
        spiTransaction.rxBuf   = (void *)gRxBuffer; 
        
        MCSPI_transfer(gMcspiHandle[CONFIG_MCSPI0], &spiTransaction);
        ClockP_usleep(50000); 
    }
    DebugP_log("[MASTER] Send Complete. Waiting for Slave Ready...\r\n");

    /* --- PHASE 2: POLL FOR READY (0xAA) --- */
    int slave_ready = 0;
    while(!slave_ready) {
        memset(gTxBuffer, 0, APP_MCSPI_MSGSIZE); // Dummy
        memset(gRxBuffer, 0, APP_MCSPI_MSGSIZE);

        MCSPI_Transaction_init(&spiTransaction);
        spiTransaction.channel = MCSPI_CHANNEL_NUM;
        spiTransaction.count   = APP_MCSPI_MSGSIZE; // Poll 1 packet
        spiTransaction.txBuf   = (void *)gTxBuffer;
        spiTransaction.rxBuf   = (void *)gRxBuffer;
        
        MCSPI_transfer(gMcspiHandle[CONFIG_MCSPI0], &spiTransaction);

            /* ... inside polling loop ... */
        DebugP_log("[MASTER] Polling... Rx[0]=0x%02X\r\n", gRxBuffer[0]);

        /* Accept 0xAA (Perfect) or 0xD5 (Phase Shifted) */
        if(gRxBuffer[0] == 0xAA) { 
            slave_ready = 1;
            DebugP_log("[MASTER] Slave is READY (Got Signal)!\r\n");
        }
    }

    /* --- PHASE 3: RECEIVE DATA --- */
    DebugP_log("[MASTER] Reading Data Back...\r\n");
    for(pkt = 0; pkt < NUM_PACKETS; pkt++)
    {
        memset(gTxBuffer, 0, APP_MCSPI_MSGSIZE);
        memset(gRxBuffer, 0, APP_MCSPI_MSGSIZE);

        MCSPI_Transaction_init(&spiTransaction);
        spiTransaction.channel = MCSPI_CHANNEL_NUM;
        spiTransaction.count   = APP_MCSPI_MSGSIZE;
        spiTransaction.txBuf   = (void *)gTxBuffer;
        spiTransaction.rxBuf   = (void *)gRxBuffer;
        
        MCSPI_transfer(gMcspiHandle[CONFIG_MCSPI0], &spiTransaction);

        int match = 1;
        
        for (int i = 0; i < APP_MCSPI_MSGSIZE; i++) {
            uint8_t expected = (uint8_t) (pkt + i);

            if (gRxBuffer[i] != expected) {
                match = 0;
                break;
            }
        }

        if (match) {
            okCount++;
            //DebugP_log("[MASTER] Pkt %d: OK\r\n", pkt);
        } else {
            errorCount++;
            DebugP_log("[MASTER] Pkt %d: FAIL (Rx[0]=0x%02X Exp=0x%02X)\r\n", 
                       pkt, gRxBuffer[0], (uint8_t)pkt);
        }

        // DebugP_log("[MASTER] Rx Pkt %d: %02X %02X %02X %02X\r\n", 
        //            pkt, gRxBuffer[0], gRxBuffer[1], gRxBuffer[2], gRxBuffer[3]);
        
        ClockP_usleep(20000); 
    }

    endTime = ClockP_getTimeUsec();
    totalTime = endTime - startTime;

    DebugP_log("------------------------------------------------\r\n");
    DebugP_log("Test Completed.\r\n");
    DebugP_log("Total Packets: %d\r\n", NUM_PACKETS);
    DebugP_log("OK Packets:    %d\r\n", okCount);
    DebugP_log("Error Packets: %d\r\n", errorCount);
    DebugP_log("Total Time:    %lld usec\r\n", totalTime);
    DebugP_log("------------------------------------------------\r\n");

    Board_driversClose();
    Drivers_close();
    vTaskDelete(NULL);
}
