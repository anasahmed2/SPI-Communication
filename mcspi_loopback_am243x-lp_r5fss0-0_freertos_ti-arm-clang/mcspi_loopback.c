/*
 *  Copyright (C) 2021-24 Texas Instruments Incorporated
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions
 *  are met:
 *
 *    Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 *    Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the
 *    distribution.
 *
 *    Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 *  A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 *  OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 *  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 *  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 *  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 *  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 *  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 *  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/* This example demonstrates the McSPI RX and TX operation configured
 * in blocking, interrupt mode of operation.
 *
 * This example sends a known data in the TX mode of length APP_MCSPI_MSGSIZE
 * and then receives the same in RX mode. Internal pad level loopback mode
 * is enabled to receive data.
 * To enable internal pad level loopback mode, D0 pin is configured to both
 * TX Enable as well as RX input pin in the SYSCFG.
 *
 * When transfer is completed, TX and RX buffer data are compared.
 * If data is matched, test result is passed otherwise failed.
 */

#include <kernel/dpl/DebugP.h>
#include <kernel/dpl/ClockP.h>
#include "ti_drivers_config.h"
#include "ti_drivers_open_close.h"
#include "ti_board_open_close.h"
#include <string.h>

#define APP_SPI_TRANSFER_SIZE   (16)

uint8_t gMcspiTxBuffer[APP_SPI_TRANSFER_SIZE];
uint8_t gMcspiRxBuffer[APP_SPI_TRANSFER_SIZE];

void *mcspi_loopback_main(void *args)
{
    int32_t status;
    MCSPI_Transaction transaction;

    Drivers_open();
    Board_driversOpen();

    DebugP_log("SPI SLAVE: Ready and waiting...\r\n");

    /* Slave TX buffer = dummy data (master must clock to receive) */
    for (uint32_t i = 0; i < APP_SPI_TRANSFER_SIZE; i++)
    {
        gMcspiTxBuffer[i] = 0x55;   // Dummy reply bytes
        gMcspiRxBuffer[i] = 0;
    }

    MCSPI_Transaction_init(&transaction);
    transaction.channel   = gConfigMcspi0ChCfg[0].chNum;
    transaction.dataSize  = 8;
    transaction.count     = APP_SPI_TRANSFER_SIZE;
    transaction.txBuf     = gMcspiTxBuffer;   // required even if unused
    transaction.rxBuf     = gMcspiRxBuffer;
    transaction.csDisable = FALSE;

    DebugP_log("SPI SLAVE: Waiting for master...\r\n");

    /* This blocks until master drives clock + CS */
    status = MCSPI_transfer(gMcspiHandle[CONFIG_MCSPI0], &transaction);

    if ((status == SystemP_SUCCESS) &&
        (transaction.status == MCSPI_TRANSFER_COMPLETED))
    {
        DebugP_log("SPI SLAVE: Data received!\r\n");

        for (uint32_t i = 0; i < APP_SPI_TRANSFER_SIZE; i++)
        {
            DebugP_log("Byte %d = 0x%02X\r\n", i, gMcspiRxBuffer[i]);
        }
    }
    else
    {
        DebugP_log("SPI SLAVE: Transfer FAILED\r\n");
    }

    while (1)
    {
        ClockP_sleep(1);
    }

    Board_driversClose();
    Drivers_close();
    return NULL;
}
