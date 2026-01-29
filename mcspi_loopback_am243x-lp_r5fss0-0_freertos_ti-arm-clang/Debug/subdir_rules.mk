################################################################################
# Automatically-generated file. Do not edit!
################################################################################

SHELL = cmd.exe

# Each subdirectory must supply rules for building sources it contributes
build-201896929: ../example.syscfg
	@echo 'Building file: "$<"'
	@echo 'Invoking: SysConfig'
	"C:/ti/ccs2030/ccs/utils/sysconfig_1.25.0/sysconfig_cli.bat" --script "C:/Users/AnasAhmed/SPI_SLAVE/mcspi_loopback_am243x-lp_r5fss0-0_freertos_ti-arm-clang/example.syscfg" -o "syscfg" -s "C:/ti/mcu_plus_sdk_am243x_11_00_00_15/.metadata/product.json" -p "ALX" -r "ALX" --context "r5fss0-0" --compiler ticlang
	@echo 'Finished building: "$<"'
	@echo ' '

syscfg/ti_dpl_config.c: build-201896929 ../example.syscfg
syscfg/ti_dpl_config.h: build-201896929
syscfg/ti_drivers_config.c: build-201896929
syscfg/ti_drivers_config.h: build-201896929
syscfg/ti_drivers_open_close.c: build-201896929
syscfg/ti_drivers_open_close.h: build-201896929
syscfg/ti_pinmux_config.c: build-201896929
syscfg/ti_power_clock_config.c: build-201896929
syscfg/ti_board_config.c: build-201896929
syscfg/ti_board_config.h: build-201896929
syscfg/ti_board_open_close.c: build-201896929
syscfg/ti_board_open_close.h: build-201896929
syscfg/ti_enet_config.c: build-201896929
syscfg/ti_enet_config.h: build-201896929
syscfg/ti_enet_open_close.c: build-201896929
syscfg/ti_enet_open_close.h: build-201896929
syscfg/ti_enet_soc.c: build-201896929
syscfg/ti_enet_lwipif.c: build-201896929
syscfg/ti_enet_lwipif.h: build-201896929
syscfg/linker.cmd: build-201896929
syscfg/linker_defines.h: build-201896929
syscfg: build-201896929

syscfg/%.o: ./syscfg/%.c $(GEN_OPTS) | $(GEN_FILES) $(GEN_MISC_FILES)
	@echo 'Building file: "$<"'
	@echo 'Invoking: Arm Compiler'
	"C:/ti/ti_cgt_arm_llvm_4.0.1.LTS/bin/tiarmclang.exe" -c -mcpu=cortex-r5 -mfloat-abi=hard -mfpu=vfpv3-d16 -mlittle-endian -mthumb -I"C:/ti/ti_cgt_arm_llvm_4.0.1.LTS/include/c" -I"C:/ti/mcu_plus_sdk_am243x_11_00_00_15/source" -I"C:/ti/mcu_plus_sdk_am243x_11_00_00_15/source/kernel/freertos/FreeRTOS-Kernel/include" -I"C:/ti/mcu_plus_sdk_am243x_11_00_00_15/source/kernel/freertos/portable/TI_ARM_CLANG/ARM_CR5F" -I"C:/ti/mcu_plus_sdk_am243x_11_00_00_15/source/kernel/freertos/config/am243x/r5f" -DSOC_AM243X -D_DEBUG_=1 -g -Wall -Wno-gnu-variable-sized-type-not-at-end -Wno-unused-function -MMD -MP -MF"syscfg/$(basename $(<F)).d_raw" -MT"$(@)" -I"C:/Users/AnasAhmed/SPI_SLAVE/mcspi_loopback_am243x-lp_r5fss0-0_freertos_ti-arm-clang/Debug/syscfg"  $(GEN_OPTS__FLAG) -o"$@" "$<"
	@echo 'Finished building: "$<"'
	@echo ' '

%.o: ../%.c $(GEN_OPTS) | $(GEN_FILES) $(GEN_MISC_FILES)
	@echo 'Building file: "$<"'
	@echo 'Invoking: Arm Compiler'
	"C:/ti/ti_cgt_arm_llvm_4.0.1.LTS/bin/tiarmclang.exe" -c -mcpu=cortex-r5 -mfloat-abi=hard -mfpu=vfpv3-d16 -mlittle-endian -mthumb -I"C:/ti/ti_cgt_arm_llvm_4.0.1.LTS/include/c" -I"C:/ti/mcu_plus_sdk_am243x_11_00_00_15/source" -I"C:/ti/mcu_plus_sdk_am243x_11_00_00_15/source/kernel/freertos/FreeRTOS-Kernel/include" -I"C:/ti/mcu_plus_sdk_am243x_11_00_00_15/source/kernel/freertos/portable/TI_ARM_CLANG/ARM_CR5F" -I"C:/ti/mcu_plus_sdk_am243x_11_00_00_15/source/kernel/freertos/config/am243x/r5f" -DSOC_AM243X -D_DEBUG_=1 -g -Wall -Wno-gnu-variable-sized-type-not-at-end -Wno-unused-function -MMD -MP -MF"$(basename $(<F)).d_raw" -MT"$(@)" -I"C:/Users/AnasAhmed/SPI_SLAVE/mcspi_loopback_am243x-lp_r5fss0-0_freertos_ti-arm-clang/Debug/syscfg"  $(GEN_OPTS__FLAG) -o"$@" "$<"
	@echo 'Finished building: "$<"'
	@echo ' '


