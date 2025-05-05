# 🚦 Semáforo acessível

Projeto desenvolvido para fins de aprendizado em Sistemas Embarcados, utilizando a placa de desenvolvimento BitDogLab. O sistema simula o funcionamento de um semáforo real, alternando entre os modos normal e noturno e utiliza diversos periféricos para indicar o estado atual de forma visual e sonora.

## 🎯 Objetivo

Construir um semáforo inteligente com várias tarefas via FreeRTOS, simulando os estados de trânsito (verde, amarelo, vermelho) no modo normal e um pisca-alerta noturno. O projeto visa exercitar o uso de multitarefa com RTOS.

## ⚙️ Como Funciona

- O sistema inicia no modo **normal**, alternando entre verde (8s), amarelo (3s) e vermelho (8s).
- O botão A alterna o sistema entre **modo normal** e **modo noturno**.
- No **modo noturno**, os LEDs piscam em amarelo, com sinalização visual e sonora contínua, simulando alerta noturno.
- O botão B reinicia o dispositivo, entrando em modo de gravação USB.
- O display OLED informa mensagens de aviso.
- A matriz WS2812 exibe padrões ou efeitos de luz diferentes conforme o estado.
- O buzzer emite sinais sonoros específicos para cada estado, incluindo toques intermitentes no modo noturno.

O sistema é totalmente multitarefa, com cada periférico controlado por uma tarefa independente utilizando FreeRTOS.

## 🧩 Periféricos Utilizados

- **Display OLED** – Exibe mensagens indicativas do estado do semáforo.
- **LED RGB** – Representa as cores do semáforo.
- **Matriz de LEDs WS2812 (5x5)** – Utilizada para efeitos de luz diferenciados conforme o estado.
- **Buzzer** – Emite sons para indicar troca de estado ou alerta noturno.
- **Botões (A e B)** – Alternam entre modos e reiniciam o sistema.

## 🛠️ Tecnologias e Ferramentas

- **Linguagem:** C
- **RTOS:** FreeRTOS
- **Placa:** BitDogLab (RP2040)
- **IDE Recomendada:** VSCode com extensão do Raspberry Pi Pico

## 🧠 Aprendizados

Este projeto reforçou conhecimentos em:

- Estruturação de tarefas em FreeRTOS
- Gerenciamento de tempo e delays não bloqueantes
- Comunicação I2C com periféricos (display OLED)
- Controle de LEDs WS2812 via PIO
- Geração de som com PWM

## Como executar o projeto 🛠️

1. **Configuração do Ambiente:**

   - Certifique-se de que o SDK do Raspberry Pi Pico esteja instalado.
   - Tenha o CMake e o toolchain ARM configurados no sistema.

2. **Clone o repositório:**

   ```bash
   git clone https://github.com/jacks0nsilva/ATIVIDADE-03.git
   ```

3. **Abra o projeto no VSCode.**

4. **Altere o caminho do Kernel do FreeRTOS:**

   - No arquivo `CMakeLists.txt`, localize a linha que define o caminho do FreeRTOS e ajuste conforme necessário. Por exemplo:

     ```cmake
     set(FREERTOS_KERNEL_PATH "/caminho/para/o/seu/FreeRTOS")
     ```

5. **Instale a extensão do Raspberry Pi Pico no VSCode, se ainda não tiver.**

6. **Siga os passos da extensão:**

   - **Clean CMake:** Limpa arquivos antigos e evita conflito de build.
   - **Compile Project:** Compila o firmware com todos os binários.
   - **Run Project [USB]:** Envia o binário automaticamente para a placa.
