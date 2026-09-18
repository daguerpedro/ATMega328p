# Unidade de Contagem
**Timer/Contador**: Registrador `TCNT0`
### Contador
Recebe dados da [[#Counter Control Logic (CCL)]]
Composição do **bloco contador**:  
- Flipflop de controle (`count`)
- Multiplexador (`direction`):
	- TCNT0 += 1
	- TCNT0 -= 1
- `Clear`
- `TCNT0` (Registrador)
### Counter Control Logic (CCL)
Recebe registradores de periférico:
- `TCCR0A`
- `TCCR0B`
Recebe referências:
- BOTTOM: fixo em `0x00`
- MAX: `0xFF`ou Registrador `OCR0A`
Recebe `ClkT0`
