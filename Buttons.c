#include "stm32f4xx.h" 
#include <stdio.h>
#include "Buttons.h"
#include <string.h>
#include "Keypad.h"

void buttons_init() {
	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOBEN;
	
	GPIOB->MODER &= ~(GPIO_MODER_MODER8 | GPIO_MODER_MODER7);
	
	GPIOB->PUPDR &= ~(GPIO_PUPDR_PUPD8 | GPIO_PUPDR_PUPD7); //Clears pins
	GPIOB->PUPDR |= (GPIO_PUPDR_PUPD8_0 | GPIO_PUPDR_PUPD7_0); //Pull up
	
	//Set up interrupts
	SYSCFG->EXTICR[2] |= (SYSCFG_EXTICR3_EXTI8_PB);
	SYSCFG->EXTICR[1] |= (SYSCFG_EXTICR2_EXTI7_PB);
	EXTI->IMR |= (EXTI_IMR_IM8 | EXTI_IMR_IM7); // Interrupt Mask
	EXTI->RTSR |= (EXTI_RTSR_TR8 | EXTI_RTSR_TR7); //Rising trigger selection
	NVIC_SetPriority(EXTI9_5_IRQn,0);
	NVIC_ClearPendingIRQ(EXTI9_5_IRQn);
	NVIC_EnableIRQ(EXTI9_5_IRQn);
}