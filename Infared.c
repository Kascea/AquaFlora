#include "Infared.h"                 
#include "stm32f4xx.h"

void Infared_init() {
	//set up two EXTI on the infared sensors (PB 3, 4, 5)
	RCC->AHB1ENR|=RCC_AHB1ENR_GPIOBEN;
	RCC->APB2ENR |= RCC_APB2ENR_SYSCFGEN; //turn on clocks for syscfg and GPIOB
		
	GPIOB -> MODER &= ~GPIO_MODER_MODE2; 
	GPIOB -> MODER &= ~GPIO_MODER_MODE4;

	GPIOB -> PUPDR |= GPIO_PUPDR_PUPD2_1;
	GPIOB -> PUPDR |= GPIO_PUPDR_PUPD4_1; //set as pull down
		
	SYSCFG -> EXTICR[0] |= SYSCFG_EXTICR1_EXTI2_PB;
	SYSCFG -> EXTICR[1] |= SYSCFG_EXTICR2_EXTI4_PB;
	
	EXTI->IMR |= EXTI_IMR_MR2;
	EXTI->IMR |= EXTI_IMR_MR4; //Interrupt Mask
		
	EXTI->FTSR|= EXTI_FTSR_TR2; 
	EXTI->FTSR|= EXTI_FTSR_TR4;
	
	__enable_irq();
	NVIC_SetPriority(EXTI2_IRQn,0);
	NVIC_ClearPendingIRQ(EXTI2_IRQn);
	NVIC_EnableIRQ(EXTI2_IRQn);
	NVIC_SetPriority(EXTI4_IRQn,0);
	NVIC_ClearPendingIRQ(EXTI4_IRQn);
	NVIC_EnableIRQ(EXTI4_IRQn);
}



