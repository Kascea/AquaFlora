#include "Keypad.h"

////R1 = PD12, R2 = PD13, R3 = PD14, R4 = PD15
////C1 = PD8, C2 = PD9, C3 = PD10

void keypad_init() {
	RCC->AHB1ENR |= RCC_AHB1ENR_GPIODEN;
	RCC->APB2ENR |= RCC_APB2ENR_SYSCFGEN;
	set_rows_input();
	set_columns_output();
	
	//Set up interrupts
	SYSCFG->EXTICR[3] |= (SYSCFG_EXTICR4_EXTI15_PD | SYSCFG_EXTICR4_EXTI14_PD | SYSCFG_EXTICR4_EXTI13_PD | SYSCFG_EXTICR4_EXTI12_PD);
	EXTI->IMR |= (EXTI_IMR_IM15 | EXTI_IMR_IM14 | EXTI_IMR_IM13 | EXTI_IMR_IM12); // Interrupt Mask
	EXTI->FTSR |= (EXTI_FTSR_TR15 | EXTI_FTSR_TR14 | EXTI_FTSR_TR13 | EXTI_FTSR_TR12); //Falling trigger selection
	NVIC_SetPriority(EXTI15_10_IRQn,1);
	NVIC_ClearPendingIRQ(EXTI15_10_IRQn);
	NVIC_EnableIRQ(EXTI15_10_IRQn);
}

char read_input() {
	//Read rows
	uint32_t rowData = ~GPIOD->IDR & (GPIO_IDR_ID15_Msk | GPIO_IDR_ID14_Msk | GPIO_IDR_ID13_Msk | GPIO_IDR_ID12_Msk);
	
	//Read Cols
	set_rows_output();
	set_columns_input();
	for(int i=0; i < 1000; i++);
	uint32_t colData = ~GPIOD->IDR & (GPIO_IDR_ID10_Msk | GPIO_IDR_ID9_Msk | GPIO_IDR_ID8_Msk);
	
	rowData = rowData>>12u;
	int row;
		switch(rowData) {
		case 8:
			row = 4;
			break;
		case 4:
			row = 3;
			break;
		case 2:
			row = 2;
			break;
		case 1:
			row = 1;
			break;
		default:
			row = 0;
			break;
	}
		
	colData = colData>>8u;
	int col;
		switch(colData) {
		case 4:
			col = 3;
			break;
		case 2:
			col = 2;
			break;
		case 1:
			col = 1;
			break;
		default:
			col = 0;
			break;
	}
	//Return to original state
	set_rows_input();
	set_columns_output();
	return convert_input(row, col);
}

char convert_input(int row, int col) {
	if(row != 0 && col!= 0) {
		return keypad[row-1][col-1];
	}
	return '!';
}

void set_columns_output() {
	GPIOD->MODER &= ~(GPIO_MODER_MODER10 | GPIO_MODER_MODER9 | GPIO_MODER_MODER8); //Clears MODER
	GPIOD->MODER |= (GPIO_MODER_MODER10_0 | GPIO_MODER_MODER9_0 | GPIO_MODER_MODER8_0); 
	
	GPIOD->ODR = 0; //Writes 0s to pins
}

void set_columns_input() {
	GPIOD->MODER &= ~(GPIO_MODER_MODER10 | GPIO_MODER_MODER9 | GPIO_MODER_MODER8); //Clears MODER
	
	GPIOD->PUPDR &= ~(GPIO_PUPDR_PUPD10 | GPIO_PUPDR_PUPD9 | GPIO_PUPDR_PUPD8); //Clears pins
	GPIOD->PUPDR |= (GPIO_PUPDR_PUPD10_0 | GPIO_PUPDR_PUPD9_0 | GPIO_PUPDR_PUPD8_0);
}

void set_rows_output(void) {
	GPIOD->MODER &= ~(GPIO_MODER_MODER15 | GPIO_MODER_MODER14 | GPIO_MODER_MODER13 | GPIO_MODER_MODER12); //Clears MODER
	GPIOD->MODER |= (GPIO_MODER_MODER15_0 | GPIO_MODER_MODER14_0 | GPIO_MODER_MODER13_0 | GPIO_MODER_MODER12_0);
	
	GPIOD->ODR = 0; //Writes 0s to pins
}

void set_rows_input(void) {
	GPIOD->MODER &= ~(GPIO_MODER_MODER15 | GPIO_MODER_MODER14 | GPIO_MODER_MODER13 | GPIO_MODER_MODER12); //Clears MODER
	
	GPIOD->PUPDR &= ~(GPIO_PUPDR_PUPD15 | GPIO_PUPDR_PUPD14 | GPIO_PUPDR_PUPD13 | GPIO_PUPDR_PUPD12); //Clears pins
	GPIOD->PUPDR |= (GPIO_PUPDR_PUPD15_0 | GPIO_PUPDR_PUPD14_0 | GPIO_PUPDR_PUPD13_0 | GPIO_PUPDR_PUPD12_0);
}

void idle_timer_init(uint32_t load_reg_value){
	SysTick->CTRL = 0;      //disable SysTick
	SysTick->LOAD = load_reg_value-1;         //setbuf reload register
	
	//set interrupt priority of SysTick to less priority
	NVIC_SetPriority (SysTick_IRQn, 6);
	SysTick->VAL = 0; //reset the SysTick counter value
	
	//select processor clock: 1 = processor clock; 0 = external clock
	SysTick->CTRL |= SysTick_CTRL_CLKSOURCE_Msk;
	//enables SysTick interrupt, 1 = Enable, 0 = Disable
	SysTick->CTRL |= SysTick_CTRL_TICKINT_Msk;
	
	//disable SysTick
	SysTick->CTRL &= ~SysTick_CTRL_ENABLE_Msk;
	timerVal = 0;
}


