#include "Final_Functions.h"
#include "Keypad.h"

// Stepper motor initialization
void stepper_motor_init(void){
	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOBEN;		// enable port B clock
	Motor_Time_Init();
	// set pins 0-3 as output
	GPIOB->MODER |= GPIO_MODER_MODE11_0;
	GPIOB->MODER |= GPIO_MODER_MODE12_0;
	GPIOB->MODER |= GPIO_MODER_MODE13_0;
	GPIOB->MODER |= GPIO_MODER_MODE14_0;	
}

// 0=forward 1=backwards
void driveMotor(int direction){
	if (direction == 0){
		GPIOB->ODR |= (1U<<11);
		GPIOB->ODR &=~ (1U<<12);
		GPIOB->ODR &=~ (1U<<13);
		GPIOB->ODR &=~ (1U<<14);
for(int i=0; i<2000; i++);
		
		GPIOB->ODR &=~ (1U<<11);
		GPIOB->ODR |= (1U<<12);
		GPIOB->ODR &=~ (1U<<13);
		GPIOB->ODR &=~ (1U<<14);
for(int i=0; i<2000; i++);
		
		GPIOB->ODR &=~ (1U<<11);
		GPIOB->ODR &=~ (1U<<12);
		GPIOB->ODR |= (1U<<13);
		GPIOB->ODR &=~ (1U<<14);
for(int i=0; i<2000; i++);
		
		GPIOB->ODR &=~ (1U<<11);
		GPIOB->ODR &=~ (1U<<12);
		GPIOB->ODR &=~ (1U<<13);
		GPIOB->ODR |= (1U<<14);
for(int i=0; i<2000; i++);
	} else {
		GPIOB->ODR &=~ (1U<<11);
		GPIOB->ODR &=~ (1U<<12);
		GPIOB->ODR &=~ (1U<<13);
		GPIOB->ODR |= (1U<<14);
for(int i=0; i<2000; i++);
		
		GPIOB->ODR &=~ (1U<<11);
		GPIOB->ODR &=~ (1U<<12);
		GPIOB->ODR |= (1U<<13);
		GPIOB->ODR &=~ (1U<<14);
for(int i=0; i<2000; i++);
		
		GPIOB->ODR &=~ (1U<<11);
		GPIOB->ODR |= (1U<<12);
		GPIOB->ODR &=~ (1U<<13);
		GPIOB->ODR &=~ (1U<<14);
for(int i=0; i<2000; i++);
		
		GPIOB->ODR |= (1U<<11);
		GPIOB->ODR &=~ (1U<<12);
		GPIOB->ODR &=~ (1U<<13);
		GPIOB->ODR &=~ (1U<<14);
for(int i=0; i<2000; i++);
	}
}	

void water_level_init(void){
	RCC->AHB1ENR |= RCC_AHB1ENR_GPIODEN;		// enable port D clock
	GPIOD->MODER &=~ GPIO_MODER_MODE6;		// set PD6 to input
	//*********************** ADC Setup ********************************//
	// initialize clock for Port A and ADC
	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;	
	RCC->APB2ENR |= RCC_APB2ENR_ADC1EN;
	// intiialize Port A pin 1 as ADC pin
	GPIOA->MODER |= GPIO_MODER_MODE1_Msk;
	// CR1 register (default is 12 bit)
	// Set Sampling time to 84 (100) cycles in SMPR2 register (default is 000)
	ADC1->SMPR2 |= ADC_SMPR2_SMP0_2;
	// Set Continuious mode ADC (CR2 register)
	ADC1->CR2 |= ADC_CR2_CONT_Msk;
	// Specify channel number 1 of the 1st conversion
	ADC1->SQR1 &=~ ADC_SQR1_L;
	ADC1->SQR3 |= ADC_SQR3_SQ1_0;
	//Set end of conversion interrupt in CR1 register
	ADC1->CR1 |= ADC_CR1_EOCIE;

}

// if it returns 1, water is low
int read_water_level(void){
	int water_level;
	// Turn on ADC conversion 
	ADC1->CR2 |= ADC_CR2_ADON;
	// Start Conversion
	ADC1->CR2 |= ADC_CR2_SWSTART;
	while((ADC1->SR & ADC_SR_EOC) == 0);
	water_level = ADC1->DR;
	// Turn off ADC Conversion
	ADC1->CR2 &=~ ADC_CR2_ADON;
	if (water_level < 400){
		return 1;
	} else {
		return 0;
	}
}


void Motor_Time_Init(void){
	SysTick->CTRL = 0;	//disable SysTick
	SysTick->LOAD = 15999-1;		//setbuf reload register

	//set interrupt priority of SysTick to less priority
	NVIC_SetPriority (SysTick_IRQn, 6);
	SysTick->VAL = 0;	//reset the SysTick counter value

	//select processor clock: 1 = processor clock; 0 = external clock
	SysTick->CTRL |= SysTick_CTRL_CLKSOURCE_Msk;
	//enables SysTick interrupt, 1 = Enable, 0 = Disable
	SysTick->CTRL |= SysTick_CTRL_TICKINT_Msk;
	//enable SysTick
	SysTick->CTRL |= SysTick_CTRL_ENABLE_Msk;
}

//void systick_handler(void){	//systick interrupt service routine
//	if (delaytime > 0){
//		delaytime--;
//	}
//}

void myDelay(uint32_t TimeToDelay){
	// TimeToDelay specifies the delay time length
	timerVal = TimeToDelay;
	while(timerVal != 0);	//busy wait until 0
}

void water_valve_init() {
	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;            // enable port A clock
	GPIOA->MODER |= GPIO_MODER_MODE2_0;             // use pin A2
}

void water_control(int mode){
	if (mode == 0){
				GPIOA->ODR &=~ GPIO_ODR_OD2;
	} else {
				GPIOA->ODR |= GPIO_ODR_OD2;
	}
}
