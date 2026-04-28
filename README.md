# mcspi_loopback_am243x-lp_r5fss0-0_freertos_ti-arm-clang

This repository contains a McSPI (Multi-Channel SPI) loopback example for the TI AM243x platform. The example runs on an R5F core under FreeRTOS and uses TI drivers (ti-drivers) and Board support packages. It demonstrates an SPI Slave that receives multiple packets from a Master, signals readiness by returning 0xAA, then sends the stored packets back.

Key points
- Target: AM243x (example config: `targetConfigs/AM2434_ALX.ccxml`).
- OS: FreeRTOS.
- Toolchain / IDE: TI Code Composer Studio (ti-arm-clang). Project files for CCS are included.
- Example behavior: the slave receives `NUM_PACKETS` of `APP_MCSPI_MSGSIZE` bytes, writes 0xAA to the TX buffer to signal readiness, then transmits the received data back to the master.

Where to look
- Application entry: `main.c` — creates the FreeRTOS main task and calls `spi_main_task`.
- SPI logic: `mcspi_loopback.c` — implements `spi_main_task`, the receive/signal/send phases and debug prints.
- System/board files: `example.syscfg` and the `syscfg/` folder (auto-generated support code).

Build
1. Recommended: import the project into Code Composer Studio (File → Import → CCS Projects) and build using the ti-arm-clang toolchain.
2. Command-line (if environment is configured for TI tools):

```bash
cd Debug
make
```

Flash / Run
- Use the CCS target configuration `targetConfigs/AM2434_ALX.ccxml` to connect to your target and start a debug session.
- Open a serial console to the target's UART (typical settings: 115200 8N1) to view the DebugP_log output.

Behavior / Expected Output
- Console logs show progress, e.g.:
  - "[SLAVE] Ready to Receive." (receiving phase)
  - "[SLAVE] Data Received. Signaling Ready (0xAA)..." (ready signal)
  - "[SLAVE] Master Acknowledged Ready." (master polled)
  - "[SLAVE] Done." (all packets sent back)

Notes
- This example assumes a physical or virtual SPI master connected to the slave channel defined in `mcspi_loopback.c` (channel 0 by default).
- If you need help building or adapting this example to another board or master device, tell me which toolchain or connection method you prefer and I can add tailored instructions.

Authors / License
- Example project provided by TI (as included in the upstream example). Keep original license notices in the auto-generated files.

Technologies & Dependencies
- Languages: C for firmware and platform code.
- RTOS: FreeRTOS (task creation and scheduler used in `main.c`).
- TI Software Packages: TI Drivers (`ti-drivers`), Board Support Package (BSP / `ti_board_*`), DebugP (`kernel/dpl/DebugP`).
- SPI: MCSPI driver (MCSPI API used in `mcspi_loopback.c`).
- Toolchain / Build: ti-arm-clang (used by Code Composer Studio); project includes a `Makefile` in `Debug/` for command-line builds.
- IDE / Debug: Code Composer Studio (CCS) with target config `targetConfigs/AM2434_ALX.ccxml`.
- Hardware: Texas Instruments AM243x family (example configured for AM2434).
