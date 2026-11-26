# ThermoLog

**Termo-higrômetro digital com registro de dados ambientais.**

Projeto final da disciplina de Microcontroladores I do Instituto Federal de Santa Catarina (IFSC) — Campus Florianópolis, desenvolvido durante o semestre 2026/2.

## Sobre o projeto

O ThermoLog é um projeto de sistema embarcado baseado no microcontrolador ATmega328P, destinado ao monitoramento da temperatura e da umidade relativa do ar em ambientes convencionais.

O dispositivo foi idealizado para operar de forma autônoma, sem conexão com a internet, oferecendo uma interface gráfica configurável e permitindo o registro histórico das medições para análise posterior das condições ambientais.

## Funcionalidades previstas

- Aquisição de temperatura e umidade relativa do ar a cada 2 segundos.
- Exibição simultânea das grandezas medidas em um display OLED.
- Seleção da unidade de temperatura entre Celsius (°C) e Fahrenheit (°F).
- Navegação e configuração do sistema por meio de um encoder rotativo com botão integrado.
- Configuração do relógio de tempo real (RTC) e associação de data e hora às amostras.
- Armazenamento das medições e configurações em cartão SD.
- Indicação de erros na aquisição de dados.

## Hardware

| Componente | Modelo / especificação | Função |
| --- | --- | --- |
| Microcontrolador | ATmega328P | Processamento e gerenciamento do sistema |
| Sensor | DHT11 / KY-015 | Medição de temperatura e umidade |
| Display | OLED SSD1306, 0,96″, 128 × 64 pixels | Interface gráfica |
| Encoder rotativo | EC11 com botão integrado | Navegação e configuração |
| RTC | DS1302 | Registro de data e hora |
| Módulo SD | SPI / FAT32 | Armazenamento de dados e configurações |

## Firmware

O firmware é desenvolvido em C++ para a arquitetura AVR de 8 bits, utilizando uma arquitetura orientada a eventos. 

### Bibliotecas e drivers

- [FUNSAPE++ AVR8](https://github.com/leandroschwarz/funsapeAvr8Lib_xxx/) (`development2`) — biblioteca utilitária para AVR-GCC, desenvolvida pelo Prof. Leandro Schwarz.
- [SSD1306](https://github.com/lexus2k/ssd1306) — biblioteca para controle do display OLED.
- [FatFs](https://elm-chan.org/fsw/ff/) — módulo de sistema de arquivos para armazenamento em cartão SD.
- **DHT11 e Encoder** — drivers de implementação própria.
- **DS1302** — driver previsto com base na adaptação do driver DS1307 da FUNSAPE++.

## Documentação

A documentação oficial do projeto está disponível no arquivo `projeto_termo_higrometro.pdf`, que apresenta os requisitos do sistema, a arquitetura de hardware e firmware, os diagramas e a estratégia de desenvolvimento.
