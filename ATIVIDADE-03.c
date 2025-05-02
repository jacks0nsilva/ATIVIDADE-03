#include <stdio.h>
#include "pico/stdlib.h"
#include "FreeRTOS.h"
#include "FreeRTOSConfig.h"
#include "task.h"
#include "hardware/gpio.h"

void vBlinkLedTask()
{
    while(true)
    {
        gpio_put(11, true);
        vTaskDelay(pdMS_TO_TICKS(500)); // Delay for 500 ms
        gpio_put(11, false);
        vTaskDelay(pdMS_TO_TICKS(500)); // Delay for 500 ms
    }
}

void vBlinkLedTask2()
{

    while(true)
    {
        gpio_put(12, true);
        vTaskDelay(pdMS_TO_TICKS(1000)); // Delay for 1000 ms
        gpio_put(12, false);
        vTaskDelay(pdMS_TO_TICKS(750)); // Delay for 750 ms
    }
}


// Trecho para modo BOOTSEL com botão B
#include "pico/bootrom.h"
#define botaoB 6
void gpio_irq_handler(uint gpio, uint32_t events)
{
    reset_usb_boot(0, 0);
}


int main()
{
    stdio_init_all();

    // Para ser utilizado o modo BOOTSEL com botão B
    gpio_init(botaoB);
    gpio_set_dir(botaoB, GPIO_IN);
    gpio_pull_up(botaoB);
    gpio_set_irq_enabled_with_callback(botaoB, GPIO_IRQ_EDGE_FALL, true, &gpio_irq_handler);
    // Fim do trecho para modo BOOTSEL com botão B

    gpio_init(11);
    gpio_set_dir(11, GPIO_OUT);

    gpio_init(12);
    gpio_set_dir(12, GPIO_OUT);

    xTaskCreate(vBlinkLedTask, "Blink Led Task", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY, NULL);
    
    xTaskCreate(vBlinkLedTask2, "Blink Led Task 2", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY, NULL);

    vTaskStartScheduler();
    panic_unsupported();
}
