# Microcontrolador ATMega328p
Repositório para disciplina de Microcontroladores I.
# Exercício Prático
### Contador com Display Multiplexado
1. Itens Abordados
   - Periféricos:
       - Ports de I/O.
       - Interrupções Externas.
       - Timer (Periférico interno).
   - Dispositivos:
     - Displays de Sete Segmentos Multiplexados.
2. Objetivo
   Desenvolver um contador crescente de quatro dígitos em um display de sete segmentos multiplexado, utilizando interrupções
externas.

3. Requisitos
   1. O código deve ser programado para o microcontrolador ATmega328P.
   2. O código deve ser programado em C/C++.
   3. O valor da contagem deve ser armazenado em uma variável uint16_t value e deve iniciar em 2408.
   4. O valor de value deve ser apresentado no display multiplexado.
   5. A multiplexação do display deve ser realizada através de um atraso por software (delay) curto o suficiente para garantir o efeito da
   persistência da visão; recomenda-se que cada display permaneça acesso por cerca de 1 ms.
   6. O sistema possui quatro botões de contato rápidos, denominados M, C, D e U.
      - A cada borda descendente do botão U, o valor de value é incrementado em uma unidade.
      - A cada borda descendente do botão D, o valor de value é incrementado em uma dezena.
      - A cada borda descendente do botão C, o valor de value é incrementado em uma centena.
      - A cada borda descendente do botão M, o valor de value é incrementado em um milhar.
   7. Os botões devem ser tratados via interrupção externa, de modo a permitir que o valor da contagem continue a ser apresentado
   corretamente no display, mesmo que os botões estejam pressionados.
   8. O usuário pode pressionar mais de um botão ao mesmo tempo, porém, a ação de cada botão deve ser executada apenas quando
   aquele botão é pressionado.
   9. O valor de value vai de 0000 a 9999, se o usuário mandar incrementar o valor além deste limite, o valor deve estourar ciclicamente,
   por exemplo, 9945 + 100 = 45.