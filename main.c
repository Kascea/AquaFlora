#include "stm32f4xx.h"                  // Device header
#include "Final_Functions.h"
#include "LCD.h"
#include "Buttons.h"
#include "Keypad.h"
#include <stdio.h>
#include "Infared.h"
#include "SPI.h"


volatile signed char data_x[8]; ////code variables


//volatile signed char data_x[200]; ////testing variables
//volatile signed char data_y[200];
//volatile signed char data_z[200];

typedef enum {START, KEYPAD_ENTRY, IDLE, SCAN, WATERING, WATER_LOW, ROLLBACK} state;
volatile state current;

int main(void){
	int ctr = 0;
	while(1) {
		switch(current){
			case START:
				// do all initialization
				water_level_init();
				keypad_init();
				buttons_init();
				LCD_port_init();
				LCD_init();
				Infared_init();  //enables IRQ also - put before SPI
				water_valve_init();
				stepper_motor_init();
				SPI_init();
			
				current = KEYPAD_ENTRY;
				break;
			
			case KEYPAD_ENTRY:
				idle_timer_init(15999999); //1 sec timer
				while(current == KEYPAD_ENTRY);
				break;
			
			case IDLE:
				while(current == IDLE);
				break;
			
			case SCAN:
				while(current == SCAN) { //interrupt from infrared sensors truggers 
					driveMotor(0); //forward = 0, backward = 1
				}
				break;
			
			case WATERING:
				if(read_water_level()) {
					current = WATER_LOW;
					break;
				}
				place_lcd_cursor(2);
				LCD_print_string("WATERING");
				while(current == WATERING) {
					water_control(1); //open valve
					for(int i=0; i<15999999; i++); // 1 sec
					water_control(0);
					current = SCAN;
					for(int i=0; i<15999999; i++); // 1 sec //wait 5 secs for all water to drip out
				}
				break;
			
			case WATER_LOW:
				LCD_print_string("WATER LEVEL LOW");
				while(current == WATER_LOW) {
					if(!read_water_level()) { //stay in WATER_LOW until read_water_level returns a 0
						current =  WATERING;
					}
				}
				break;
			case ROLLBACK:
				ctr = 0;
				while(current == ROLLBACK && ctr < 1599999900) {
					driveMotor(1);
					ctr++;
				}
				current = START;
				break;
		}	
	}
}

void EXTI2_IRQHandler() { //Infrared interrupt for plant detection
	if(current != SCAN) {
		EXTI->PR |= EXTI_PR_PR2;
		NVIC_ClearPendingIRQ(EXTI2_IRQn);
		return;
	}
	for(int i=0; i<15999999; i++); // 1 sec;
	current = WATERING;
	
	EXTI->PR |= EXTI_PR_PR2;
	NVIC_ClearPendingIRQ(EXTI2_IRQn);
}


void EXTI4_IRQHandler() { //Infrared interrupt for collison detection
	if(current != SCAN) {
		EXTI->PR |= EXTI_PR_PR4;
		NVIC_ClearPendingIRQ(EXTI4_IRQn);
		return;
	}
	current = ROLLBACK;
	TIM3_init();
	
	EXTI->PR |= EXTI_PR_PR4;
	NVIC_ClearPendingIRQ(EXTI4_IRQn);
}

void EXTI15_10_IRQHandler() { //keypad interrupt
	if(current != KEYPAD_ENTRY) {
		//clear bits
		EXTI->PR |= EXTI_PR_PR12;
		EXTI->PR |= EXTI_PR_PR13;	
		EXTI->PR |= EXTI_PR_PR14; 	
		EXTI->PR |= EXTI_PR_PR15;
		NVIC_ClearPendingIRQ(EXTI15_10_IRQn);
		return;
	}
	char tmpstr[2];
	unsigned int strLength;
	if ((EXTI->PR & (EXTI_PR_PR12 | EXTI_PR_PR13 | EXTI_PR_PR14 | EXTI_PR_PR15)) ==0) { //Not from input
		return;
	} 
	tmpstr[0] = read_input();
	strLength = strlen(inputString);
	tmpstr[1] = '\0';
	if(tmpstr[0] == '#') { //Enter value
		timerVal = atoi(inputString);
		print_seconds(timerVal);
		current = IDLE;
		for(int i=0; i <500; i++);
    SysTick->CTRL |= SysTick_CTRL_ENABLE_Msk; //enable SysTick
	}
	else if(tmpstr[0] == '*') { //Clear string
		memset(inputString, 0, sizeof inputString);
		LCD_clear();
	}
	else if(tmpstr[0] != '!' && strLength < 15) {
		strcat(inputString, tmpstr);
		LCD_clear();
		LCD_print_string(inputString);
	}
	for(int i=0; i<20000; i++); //debounce	
		
	//clear bits
	EXTI->PR |= EXTI_PR_PR12;
	EXTI->PR |= EXTI_PR_PR13;	
	EXTI->PR |= EXTI_PR_PR14; 	
	EXTI->PR |= EXTI_PR_PR15;
	NVIC_ClearPendingIRQ(EXTI15_10_IRQn);
}

void EXTI9_5_IRQHandler() { //Button interrupt
	NVIC_ClearPendingIRQ(EXTI9_5_IRQn); //Clear interrupt
	EXTI->PR |= EXTI_PR_PR8; 	
	EXTI->PR |= EXTI_PR_PR7;
	
	int isOverride = GPIOB->IDR & (1u<<7); //Mask out two input bits
	if((isOverride>>7) == 1) { //Override
		timerVal = 0;
		LCD_clear();
		LCD_print_string("OVERRIDE");
		current = SCAN;
	} else { //Reset and calibrate
		timerVal = 0;
		memset(inputString, 0, sizeof inputString);
		LCD_clear();
		LCD_print_string("RESET");
		current = START;
	}
	
	for(int i=0; i<20000; i++); //debounce	
}

void SysTick_Handler(void){   //Triggers every 1 second
	if(current == IDLE) {
		if(timerVal > 0) {
			timerVal--;
			print_seconds(timerVal);
		} else {
			SysTick->CTRL &= ~SysTick_CTRL_ENABLE_Msk; //Disable SysTick timer
			current = SCAN;
		}
	} else {
		if (timerVal > 0){
			timerVal--;
		}
	}
}


void TIM3_IRQHandler(){
	int m;
	static double output;

	shift(data_x);
	GPIOE-> ODR &= ~GPIO_ODR_OD3;
	m = spi_send(0xA9); // read from X_outH (0)
	data_x[0] = spi_send(0x0);
	GPIOE-> ODR |= (1<<3);// GPIO_ODR_OD3;
	output = RMS_div8(data_x);
	
	int ctr = 0;
	if(ctr % 20 == 0) {
		LCD_print_float(output);
	}

	if (output > 20) //some number from testing)
	{
		TIM3-> CR1 &= ~TIM_CR1_CEN;
		TIM3-> DIER &= ~TIM_DIER_UIE;
		NVIC_DisableIRQ(TIM3_IRQn);
		RCC -> APB1ENR &= ~RCC_APB1ENR_TIM3EN;
	}

//	int m; //testing code
//	static int a = 0;
//	TIM3 -> SR &= ~TIM_SR_UIF; //make sure interrupt flag is off	
//	NVIC_ClearPendingIRQ(TIM3_IRQn);
//	if (a<= 200){
//	GPIOE-> ODR &= ~GPIO_ODR_OD3;
//	m = spi_send(0xA9); // read from X_outH (0)
//	data_x[a] = spi_send(0x0);
//	GPIOE-> ODR |= (1<<3);// GPIO_ODR_OD3;
//	
//	GPIOE-> ODR &= ~GPIO_ODR_OD3;
//	m = spi_send(0xAB); // read from y_outH
//	data_y[a] = spi_send(0x0);
//	GPIOE-> ODR |= (1<<3);// GPIO_ODR_OD3;
//	
//	GPIOE-> ODR &= ~GPIO_ODR_OD3;
//	m = spi_send(0xAD); // read from z_outH (1)
//	data_z[a] = spi_send(0x0) - 0x45;
//	GPIOE-> ODR |= (1<<3);// GPIO_ODR_OD3;
//	a++;
//	}
	}


