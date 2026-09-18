A ABI (Application Binary Interface) para o compilador AVR-GCC define as regras de como as funções utilizam os 32 registradores de uso geral (R0 a R31) do microcontrolador AVR.  

| Registrador  | Nome/Função          | Observação                      |
| ------------ | -------------------- | ------------------------------- |
| R0           | Temporário           | ---                             |
| **R1**       | Zero Reg             | **Sempre deve ser 0**           |
| R18 a R27    | Variáveis/Temporário | ---                             |
| R30, R31     | Z Pointer            | ---                             |
| **R2 a R17** | Variáveis locais     | **Deve ser salvo e restaurado** |
| **R28, R29** | Y Pointer            | **Deve ser salvo e restaurado** |
