#include "radio_gpio.h"
#include "main.h"

// --- CONTROLLO ACCENSIONE/SPEGNIMENTO RADIO ---
void RadioEnterShutdown(void) {
    HAL_GPIO_WritePin(SDN_GPIO_Port, SDN_Pin, GPIO_PIN_SET);
}

void RadioExitShutdown(void) {
    HAL_GPIO_WritePin(SDN_GPIO_Port, SDN_Pin, GPIO_PIN_RESET);
    HAL_Delay(5); // Aspettiamo che il chip si svegli
}

FlagStatus RadioCheckShutdown(void) {
    return (FlagStatus)HAL_GPIO_ReadPin(SDN_GPIO_Port, SDN_Pin);
}

// --- FUNZIONI VUOTE (STUB) CON I TIPI CORRETTI ---
void RadioGpioInit(RadioGpioPin xGpio, RadioGpioMode xGpioMode) {}

void RadioGpioInterruptCmd(RadioGpioPin xGpio, uint8_t nPreemption, uint8_t nSubpriority, FunctionalState xNewState) {}

FlagStatus RadioGpioGetLevel(RadioGpioPin xGpio) {
    return RESET;
}

void RadioGpioSetLevel(RadioGpioPin xGpio, GPIO_PinState xLevel) {}
