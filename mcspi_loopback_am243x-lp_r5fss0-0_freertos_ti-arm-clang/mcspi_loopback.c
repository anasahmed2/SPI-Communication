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
#include <stdint.h>

#define APP_SPI_FRAME_BITS      (8U)
#define APP_PACKET_SIZE_BYTES   (512U)

static uint8_t gRxBuf[APP_PACKET_SIZE_BYTES];
static uint8_t gTxBuf[APP_PACKET_SIZE_BYTES];

void *mcspi_loopback_main(void *args)
{
    int32_t status;
    MCSPI_Transaction transaction;

    Drivers_open();
    Board_driversOpen();

    DebugP_log("SPI SLAVE: Ready (echo mode)\r\n");

    while (1)
    {
        /* Prepare buffers for next transaction */
        memset(gRxBuf, 0, sizeof(gRxBuf));
        memset(gTxBuf, 0, sizeof(gTxBuf)); /* will be overwritten after receive */

        /* 1) Receive 512 bytes from master (while clocking out dummy bytes) */
        MCSPI_Transaction_init(&transaction);
        transaction.channel   = gConfigMcspi0ChCfg[0].chNum;  /* must match slave CS detect */
        transaction.dataSize  = APP_SPI_FRAME_BITS;
        transaction.count     = APP_PACKET_SIZE_BYTES;        /* frames [web:69] */
        transaction.txBuf     = gTxBuf;                       /* dummy during RX */
        transaction.rxBuf     = gRxBuf;
        transaction.csDisable = TRUE;
        transaction.timeout   = SystemP_WAIT_FOREVER;

        status = MCSPI_transfer(gMcspiHandle[CONFIG_MCSPI0], &transaction);

        if (!((status == SystemP_SUCCESS) &&
              (transaction.status == MCSPI_TRANSFER_COMPLETED)))
        {
            continue; /* re-arm and wait again */
        }

        /* 2) Echo back: load TX with what we received */
        memcpy(gTxBuf, gRxBuf, sizeof(gTxBuf));
        memset(gRxBuf, 0, sizeof(gRxBuf));

        /* 3) Send echo back to master (master must perform another 512-byte transfer to read it) */
        MCSPI_Transaction_init(&transaction);
        transaction.channel   = gConfigMcspi0ChCfg[0].chNum;
        transaction.dataSize  = APP_SPI_FRAME_BITS;
        transaction.count     = APP_PACKET_SIZE_BYTES;
        transaction.txBuf     = gTxBuf;       /* echo data */
        transaction.rxBuf     = gRxBuf;       /* ignored */
        transaction.csDisable = TRUE;
        transaction.timeout   = SystemP_WAIT_FOREVER;

        (void)MCSPI_transfer(gMcspiHandle[CONFIG_MCSPI0], &transaction);
        /* if this fails, loop will re-arm anyway */
    }
}
