#include <stdio.h>
#include "pico/stdlib.h"
#include "FreeRTOS.h"
#include "FreeRTOSConfig.h"
#include "task.h"
#include "libs/ssd1306.h"
#include "hardware/gpio.h"
#include "hardware/i2c.h"
#include "libs/definicoes.h"
#include "libs/pio_config.h"

bool noturne_mode = false; 
ssd1306_t ssd;

void vLedRGBTask()
{
    gpio_init(LED_GREEN);
    gpio_set_dir(LED_GREEN, GPIO_OUT);
    gpio_init(LED_RED);
    gpio_set_dir(LED_RED, GPIO_OUT);
    gpio_init(LED_BLUE);
    gpio_set_dir(LED_BLUE, GPIO_OUT);
    
    while(true)
    {
        // Se o modo noturno estivar desativado, o semáforo vai operar normalmente
        if(!noturne_mode){
            gpio_put(LED_RED, 0); // Apaga o LED vermelho
            gpio_put(LED_GREEN, 1); // Acendo o LED verde por 3 segundos
            vTaskDelay(pdMS_TO_TICKS(TIME_GREEN));
            
            // Cor amarelo
            gpio_put(LED_RED, 1); // 
            vTaskDelay(pdMS_TO_TICKS(TIME_YELLOW)); // Acende o amarelo por 2 segundos
            gpio_put(LED_GREEN, 0); // Apaga o LED verde
            gpio_put(LED_RED, 0); // Apaga o LED vermelho

            // Cor vermelho
            gpio_put(LED_RED, 1); // Acende o LED vermelho por 3 segundos
            vTaskDelay(pdMS_TO_TICKS(TIME_RED)); // Acende o vermelho por 3 segundos
            gpio_put(LED_RED, 0); // Apaga o LED vermelho
        }
        else{

            gpio_put(LED_RED, 1); 
            gpio_put(LED_GREEN, 1);
            vTaskDelay(pdMS_TO_TICKS(3000)); // Acende o vermelho e verde por 3 segundos
            gpio_put(LED_RED, 0); // Apaga o LED vermelho
            gpio_put(LED_GREEN, 0); // Apaga o LED 
            vTaskDelay(pdMS_TO_TICKS(3000));
        }
    }
}

void vDisplayTask()
{
    i2c_init(I2C_PORT, 400 * 1000);

    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA);
    gpio_pull_up(I2C_SCL);

    ssd1306_init(&ssd, WIDTH, HEIGHT, false, ADRESS, I2C_PORT);
    ssd1306_config(&ssd);
    ssd1306_send_data(&ssd);

    ssd1306_fill(&ssd, false); // Limpa o display
    ssd1306_send_data(&ssd);
    bool cor = true;
    while (true)
    {
        
        if(!noturne_mode){
            ssd1306_fill(&ssd, !cor);                          // Limpa o display
            ssd1306_rect(&ssd, 3, 3, 122, 60, cor, !cor);      // Desenha um retângulo
            ssd1306_draw_string(&ssd,"EM FRENTE",8,8);
            ssd1306_send_data(&ssd); // Envia os dados para o 
            
            vTaskDelay(pdMS_TO_TICKS(TIME_GREEN));
            ssd1306_fill(&ssd, !cor);                          // Limpa o display
            ssd1306_draw_string(&ssd,"ATENCAO",8,8);
            ssd1306_send_data(&ssd); // Envia os dados para o display

            vTaskDelay(pdMS_TO_TICKS(TIME_YELLOW));
            ssd1306_fill(&ssd, !cor);                          // Limpa o display
            ssd1306_draw_string(&ssd, "PARE", 8,8);
            ssd1306_send_data(&ssd); // Envia os dados para o display
            vTaskDelay(pdMS_TO_TICKS(TIME_RED));
        } else{
            ssd1306_draw_string(&ssd, "NOTURNO", 8, 8);
        }
            
    }
    
}

void vMatrizTask()
{
    np_init(MATRIZ_LEDS);
    np_clear();

    bool matriz[25] = {    
    0, 0, 0, 0, 0, 
    0, 1, 1, 1, 0, 
    0, 1, 1, 1, 0, 
    0, 1, 1, 1, 0, 
    0, 0, 0, 0, 0
};

    while (true)
    {
        if(!noturne_mode){
            np_set_leds(matriz, 0, 50, 0); // Verde
            vTaskDelay(pdMS_TO_TICKS(TIME_GREEN));

            np_set_leds(matriz, 50, 50, 0); // Amarelo
            vTaskDelay(pdMS_TO_TICKS(TIME_YELLOW));

            np_set_leds(matriz, 50, 0, 0); // Vermelho
            vTaskDelay(pdMS_TO_TICKS(TIME_RED));
        } else {
            np_set_leds(matriz, 50, 50, 0); // Amarelo
            vTaskDelay(pdMS_TO_TICKS(3000)); // Acende o amarelo por 3 segundos
            np_set_leds(matriz, 0, 0, 0); // Apaga os LEDs
        }
    }
}


// Trecho para modo BOOTSEL com botão B
#include "pico/bootrom.h"
#define botaoB 6
void gpio_irq_handler(uint gpio, uint32_t events)
{
    if(gpio == 5){
        noturne_mode = !noturne_mode;
    }
    else if(gpio == botaoB){
        reset_usb_boot(0, 0);
    }
    
}


int main()
{
    stdio_init_all();

    // Para ser utilizado o modo BOOTSEL com botão B
    gpio_init(botaoB);
    gpio_set_dir(botaoB, GPIO_IN);
    gpio_pull_up(botaoB);

    gpio_init(5);
    gpio_set_dir(5, GPIO_IN);
    gpio_pull_up(5);

    gpio_set_irq_enabled_with_callback(5, GPIO_IRQ_EDGE_FALL, true, &gpio_irq_handler);
    gpio_set_irq_enabled_with_callback(botaoB, GPIO_IRQ_EDGE_FALL, true, &gpio_irq_handler);
    // Fim do trecho para modo BOOTSEL com botão B

    xTaskCreate(vMatrizTask, "Matriz Task", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY, NULL);
    xTaskCreate(vLedRGBTask, "LED RGB Task", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY, NULL);
    xTaskCreate(vDisplayTask, "Display Task", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY, NULL);
    vTaskStartScheduler();
    panic_unsupported();
}
