; ------------------------------------------------------------------------------
; Project:  Exercicios
; File:     pingpong.asm
; Author:   Pedro Daguer
; Created:  2026-08-25
; Modified: 2026-08-25
; Version:  1.0
; Notes:    Template de Prof. Leandro Schwarz
; ------------------------------------------------------------------------------

; ------------------------------------------------------------------------------
; Include definition files
; ------------------------------------------------------------------------------
.include "include/m328Pdef.inc"

; ------------------------------------------------------------------------------
; Register definitions
; ------------------------------------------------------------------------------
.def    aux       = R16
.def    ani       = R17
; ------------------------------------------------------------------------------
; Interrupt vectors
; ------------------------------------------------------------------------------
.org 0x0000
    JMP resetHandler
.org INT0addr
    JMP int0Handler
.org INT1addr
    JMP int1Handler
.org PCI0addr
    JMP pcint0Handler
.org PCI1addr
    JMP pcint1Handler
.org PCI2addr
    JMP pcint2Handler
.org WDTaddr
    JMP wdtHandler
.org OC2Aaddr
    JMP timer2CompAHandler
.org OC2Baddr
    JMP timer2CompBHandler
.org OVF2addr
    JMP timer2OvfHandler
.org ICP1addr
    JMP timer1CaptureHandler
.org OC1Aaddr
    JMP timer1CompAHandler
.org OC1Baddr
    JMP timer1CompBHandler
.org OVF1addr
    JMP timer1OvfHandler
.org OC0Aaddr
    JMP timer0CompAHandler
.org OC0Baddr
    JMP timer0CompBHandler
.org OVF0addr
    JMP timer0OvfHandler
.org SPIaddr
    JMP spiHandler
.org URXCaddr
    JMP usartRxHandler
.org UDREaddr
    JMP usartDataEmptyHandler
.org UTXCaddr
    JMP usartTxHandler
.org ADCCaddr
    JMP adcHandler
.org ERDYaddr
    JMP eepromHandler
.org ACIaddr
    JMP analogCompHandler
.org TWIaddr
    JMP twiHandler
.org SPMRaddr
    JMP flashHandler
.org INT_VECTORS_SIZE


main:
    LDI aux, 0xFF   ; 0xFF    > 0b11111111
    OUT DDRD, aux   ; DDRD    > OUTPUT
    OUT PORTD, ani  ; ENVIA ANI P/ SAIDA

startAnim1:
    LDI ani, 0b00000001 ; PRIMEIRO LED
    RJMP loopAnim1

startAnim2:
    LDI ani, 0b10000000 ; ULTIMO LED
    RJMP loopAnim2

loopAnim1:
    OUT PORTD, ani ; ENVIA ANI P/ SAIDA
    RCALL delayMs250
    LSL ani        ; ANIMA P/ ESQUERDA
    BREQ startAnim2 ; CHEGOU NO FIM? PROX ANIM
    RJMP loopAnim1 ; Ñ CHEGOU? CONTINUA

loopAnim2:
    OUT PORTD, ani ; ENVIA ANI P/ SAIDA
    RCALL delayMs250
    LSR ani        ; ANIMA P/ DIREITA
    BRCS startAnim1 ; CHEGOU NO FIM? PROX ANIM
    RJMP loopAnim2 ; Ñ CHEGOU? CONTINUA

delayMs250: ; Called with RCALL (3 cycles)
 NOP ; 1 cycle
 LDI R18, 21 ; 1 cycle
 LDI R19, 75 ; 1 cycle
 LDI R20, 188 ; 1 cycle
 RJMP PC + 1 ; 2 cycles

delayMs250Loop:
 DEC R20 ; 3.999.987 cycles
 BRNE delayMs250Loop ; ...
 DEC R19 ; ...
 BRNE delayMs250Loop ; ...
 DEC R18 ; ...
 BRNE delayMs250Loop ; ...
 RET ; 4 cycles

int0Handler:
int1Handler:
pcint0Handler:
pcint1Handler:
pcint2Handler:
wdtHandler:
timer2CompAHandler:
timer2CompBHandler:
timer2OvfHandler:
timer1CaptureHandler:
timer1CompAHandler:
timer1CompBHandler:
timer1OvfHandler:
timer0CompAHandler:
timer0CompBHandler:
timer0OvfHandler:
spiHandler:
usartRxHandler:
usartDataEmptyHandler:
usartTxHandler:
adcHandler:
eepromHandler:
analogCompHandler:
twiHandler:
flashHandler:
    RETI
resetHandler:
    LDI     R16, 0
    STS     UCSR0B, R16
    LDI     R19, LOW(RAMEND)
    OUT     SPL, R19
    LDI     R19, HIGH(RAMEND)
    OUT     SPH,R19
    JMP     main
