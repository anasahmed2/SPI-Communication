/*
 * main.c
 * Common entry point for AM243x FreeRTOS applications
 */
#include <stdlib.h>
#include <kernel/dpl/DebugP.h>
#include "ti_drivers_config.h"
#include "ti_board_config.h"
#include "FreeRTOS.h"
#include "task.h"

/* Stack settings */
#define MAIN_TASK_PRI  (configMAX_PRIORITIES-1)
#define MAIN_TASK_SIZE (16384U/sizeof(configSTACK_DEPTH_TYPE))

StackType_t gMainTaskStack[MAIN_TASK_SIZE] __attribute__((aligned(32)));
StaticTask_t gMainTaskObj;
TaskHandle_t gMainTask;

/* External function prototype - implemented in spi_task.c */
extern void spi_main_task(void *args);

void freertos_main(void *args)
{
    /* Hand off to the specific application task */
    spi_main_task(args);
    
    /* If the task returns, delete the FreeRTOS task */
    vTaskDelete(NULL);
}

int main(void)
{
    /* Init SOC specific modules */
    System_init();
    Board_init();

    /* Create the main task */
    gMainTask = xTaskCreateStatic( 
                    freertos_main, 
                    "freertos_main", 
                    MAIN_TASK_SIZE, 
                    NULL, 
                    MAIN_TASK_PRI, 
                    gMainTaskStack, 
                    &gMainTaskObj );

    configASSERT(gMainTask != NULL);

    /* Start the scheduler */
    vTaskStartScheduler();

    /* Should never reach here */
    DebugP_assertNoLog(0);
    return 0;
}
