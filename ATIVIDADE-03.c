#include <stdio.h>
#include "pico/stdlib.h"
#include "FreeRTOS.h"
#include "FreeRTOSConfig.h"
#include "task.h"
#include "hardware/gpio.h"
#include "hardware/i2c.h"
#include "pico/bootrom.h"
#include "hardware/pwm.h"

// Bibliotecas locais
#include "libs/ssd1306.h"
#include "libs/definicoes.h"
#include "libs/pio_config.h"

// Estados do semáforo
typedef enum {
    ESTADO_VERDE,
    ESTADO_AMARELO,
    ESTADO_VERMELHO,
    ESTADO_NOTURNO
} EstadoSemaforo;

volatile bool noturno = false;
volatile EstadoSemaforo estado_atual = ESTADO_VERDE;
ssd1306_t ssd;


#define BOTAO_B 6

// Função essencial para troca de modo imediata
void delay_interrompivel(int total_ms) {
    const int passo = 50;
    int elapsed = 0;
    while (elapsed < total_ms) {
        if (noturno) break;
        vTaskDelay(pdMS_TO_TICKS(passo));
        elapsed += passo;
    }
}

void pwm_init_buzzer(uint pin) {
    gpio_set_function(pin, GPIO_FUNC_PWM);
    uint slice_num = pwm_gpio_to_slice_num(pin);
    pwm_config config = pwm_get_default_config();
    pwm_config_set_clkdiv(&config, clock_get_hz(clk_sys) / (5000 * 4096));  // Frequência do PWM
    pwm_init(slice_num, &config, true);
    pwm_set_gpio_level(pin, 0);  // Inicializa com o PWM desligado
}


// Task que define o estado do semáforo com base no modo
/*
 A função delay interrompivel foi criada para permitir que o semáforo mude de estado imediatamente quando o botão A é pressionado, mesmo durante os delays de cada estado. O if (noturno) verifica se o modo noturno está ativo. Se estiver, a execução volta ao início do loop.
*/
void vControleTask() {
    while (true) {
        if (!noturno) {
            estado_atual = ESTADO_VERDE;
            delay_interrompivel(TIME_GREEN);
            if (noturno) continue;

            estado_atual = ESTADO_AMARELO;
            delay_interrompivel(TIME_YELLOW);
            if (noturno) continue;

            estado_atual = ESTADO_VERMELHO;
            delay_interrompivel(TIME_RED);
            if (noturno) continue;
        } else {
            estado_atual = ESTADO_NOTURNO;
            vTaskDelay(pdMS_TO_TICKS(100));
        }
    }
}

// Tarefa do LED RGB
void vLedRGBTask() {
    gpio_init(LED_GREEN);
    gpio_set_dir(LED_GREEN, GPIO_OUT);
    gpio_init(LED_RED);
    gpio_set_dir(LED_RED, GPIO_OUT);

    while (true) {
        switch (estado_atual) {
            case ESTADO_VERDE:
                gpio_put(LED_RED, 0);
                gpio_put(LED_GREEN, 1);
                break;

            case ESTADO_AMARELO:
                gpio_put(LED_GREEN, 1);
                gpio_put(LED_RED, 1); // Simula amarelo
                break;

            case ESTADO_VERMELHO:
                gpio_put(LED_RED, 1);
                gpio_put(LED_GREEN, 0);
                break;

            case ESTADO_NOTURNO:
                gpio_put(LED_GREEN, 1);
                gpio_put(LED_RED, 1);

                for (int i = 0; i < 60; i++) { // 3 segundos em passos de 50ms
                    if (estado_atual != ESTADO_NOTURNO) break;
                    vTaskDelay(pdMS_TO_TICKS(50));
                }

                gpio_put(LED_GREEN, 0);
                gpio_put(LED_RED, 0);

                for (int i = 0; i < 60; i++) {
                    if (estado_atual != ESTADO_NOTURNO) break;
                    vTaskDelay(pdMS_TO_TICKS(50));
                }

                /* Ambas instruções for proporcionam que a matriz de LEDs seja atualizada para o modo normal, assim que o estado for alterado. A cada 50ms é verificado se o estado atual é diferente do estado noturno durante 60 vezes (3 segundos) */

                continue;
        }

        vTaskDelay(pdMS_TO_TICKS(50));
    }
}


// Tarefa do Display OLED
void vDisplayTask() {
    i2c_init(I2C_PORT, 400 * 1000);
    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA);
    gpio_pull_up(I2C_SCL);

    ssd1306_init(&ssd, WIDTH, HEIGHT, false, ADRESS, I2C_PORT);
    ssd1306_config(&ssd);
    ssd1306_fill(&ssd, false);
   
    ssd1306_send_data(&ssd);

    while (true) {
        ssd1306_fill(&ssd, false);
        ssd1306_rect(&ssd, 3, 3, 122, 60, true, false);      // Desenha um retângulo
        switch (estado_atual) {
            case ESTADO_VERDE:
                ssd1306_draw_string(&ssd, "ATRAVESSE!", 8, 8);
                break;
            case ESTADO_AMARELO:
                ssd1306_draw_string(&ssd, "ATENCAO!", 8, 8);
                break;
            case ESTADO_VERMELHO:
                ssd1306_draw_string(&ssd, "PARE!", 8, 8);
                break;
            case ESTADO_NOTURNO:
                ssd1306_draw_string(&ssd, "CUIDADO!", 8, 8);
                break;
        }
        ssd1306_send_data(&ssd);
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

// Tarefa da Matriz de LEDs 5x5
void vMatrizTask() {
    np_init(MATRIZ_LEDS);
    np_clear();

    bool matriz[25] = {
        0, 0, 0, 0, 0,
        0, 1, 1, 1, 0,
        0, 1, 1, 1, 0,
        0, 1, 1, 1, 0,
        0, 0, 0, 0, 0
    };

    while (true) {
        switch (estado_atual) {
            case ESTADO_VERDE:
                np_set_leds(matriz, 0, 50, 0); // Verde
                break;
            case ESTADO_AMARELO:
                np_set_leds(matriz, 50, 50, 0); // Amarelo
                break;
            case ESTADO_VERMELHO:
                np_set_leds(matriz, 50, 0, 0); // Vermelho
                break;
            case ESTADO_NOTURNO:
                np_set_leds(matriz, 50, 50, 0); // Amarelo

                for (int i = 0; i < 60; i++) { // Total de 3 segundos dividido em 50ms
                    if (estado_atual != ESTADO_NOTURNO) break;
                    vTaskDelay(pdMS_TO_TICKS(50));
                }

                np_set_leds(matriz, 0, 0, 0);

                for (int i = 0; i < 60; i++) {
                    if (estado_atual != ESTADO_NOTURNO) break;
                    vTaskDelay(pdMS_TO_TICKS(50));
                }
                /* Ambas instruções for proporcionam que a matriz de LEDs seja atualizada para o modo normal, assim que o estado for alterado. A cada 50ms é verificado se o estado atual é diferente do estado noturno durante 60 vezes (3 segundos) */

                continue;
        }

        vTaskDelay(pdMS_TO_TICKS(50));
    }
}


// Tarefa do botão A (modo noturno) e botão B (BOOTSEL)
void vBotaoTask() {
    gpio_init(BUTTON_A);
    gpio_set_dir(BUTTON_A, GPIO_IN);
    gpio_pull_up(BUTTON_A);

    gpio_init(BOTAO_B);
    gpio_set_dir(BOTAO_B, GPIO_IN);
    gpio_pull_up(BOTAO_B);

    bool estado_anterior_a = true;

    while (true) {
        bool estado_atual_a = gpio_get(BUTTON_A);
        if (!estado_atual_a && estado_anterior_a) {
            noturno = !noturno;
        }
        estado_anterior_a = estado_atual_a;

        if (!gpio_get(BOTAO_B)) {
            reset_usb_boot(0, 0);
        }

        vTaskDelay(pdMS_TO_TICKS(50));
    }
}

// Tarefa do buzzer 
void vBuzzerTask(){
    pwm_init_buzzer(BUZZER_A);

    while(true)
    {
        switch(estado_atual)
        {
            case ESTADO_VERDE:
            pwm_set_gpio_level(BUZZER_A, 2048);
            vTaskDelay(pdMS_TO_TICKS(1000));
            pwm_set_gpio_level(BUZZER_A, 0);
            vTaskDelay(pdMS_TO_TICKS(1000));
            break;

            case ESTADO_AMARELO:
            pwm_set_gpio_level(BUZZER_A, 2048);
            vTaskDelay(pdMS_TO_TICKS(200));
            pwm_set_gpio_level(BUZZER_A, 0);
            vTaskDelay(pdMS_TO_TICKS(200));
            break;

            case ESTADO_VERMELHO:
            pwm_set_gpio_level(BUZZER_A, 2048);
            vTaskDelay(pdMS_TO_TICKS(500));
            pwm_set_gpio_level(BUZZER_A, 0);
            vTaskDelay(pdMS_TO_TICKS(1500));
            break;

            case ESTADO_NOTURNO:
            pwm_set_gpio_level(BUZZER_A, 2048);
            vTaskDelay(pdMS_TO_TICKS(2000));
            pwm_set_gpio_level(BUZZER_A, 0);
            vTaskDelay(pdMS_TO_TICKS(2000));
            break;

        }
    }
}

int main() {
    stdio_init_all();

    xTaskCreate(vControleTask, "Controle Task", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY, NULL);
    xTaskCreate(vLedRGBTask, "LED RGB Task", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY, NULL);
    xTaskCreate(vDisplayTask, "Display Task", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY, NULL);
    xTaskCreate(vMatrizTask, "Matriz Task", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY, NULL);
    xTaskCreate(vBotaoTask, "Botao Task", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY, NULL);
    xTaskCreate(vBuzzerTask, "Buzzer Task", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY, NULL);
    vTaskStartScheduler();
    panic_unsupported();
}
