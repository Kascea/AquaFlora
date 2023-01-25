/* LCD.c
 * The goal of this lab is help you figure out
 * (on your own) how to interface to a peripheral
 * that you have never used before based on the
 * documentation you find (either datasheet or other
 * available sources/examples online).
 * ENJOY!!! (and I really mean it!)
 */

#include "stm32f4xx.h" 
#include <string.h>
#include "LCD.h"
#include <stdio.h>

/*******************************
 * START OF ACTUAL CODE
 *******************************
 */

/*******************************
 * main()
 * Inputs: NONE
 * Outputs: NONE
 * Main function. Goals:
 * Initialize LCD Port
 * Initialize LCD
 * Place Cursor on Line 1
 * Write character string to Line 1
 * Place Cursor on Line 2
 * Write character string to Line 2
 *******************************
 */

static uint8_t bf;

/*************************
 * FUNCTION DECLARATIONS:
 * LCD related functions
**************************/

/*******************************
 * LCD_port_init()
 * Inputs: NONE
 * Outputs: NONE
 * Port Initialization
 * Refer to the #define statements at top to
 * see what ports are used to connect
 * the STMicro Board with the HD44780 LCD driver
 * Set appropriate pins as digital input/outputs
 ******************
 * VERY IMPORTANT:  
 *****************
 * The Data7 (DB7) port (connected to GPIO PORT D) 
 * needs to be able to switch between
 * default mode OUTPUT (to send data) 
 * and INPUT (to check Busy Flag).
 *******************************
 */
void LCD_port_init(){
//STEP 1: Enable GPIOD in RCC AHB1ENR register
	RCC->AHB1ENR |= (RCC_AHB1ENR_GPIODEN); //Enable port d clock

//STEP 2: Set MODER of GPIOD Pins 7, 6, 5, 3, 2, 1 & 0 as outputs
	GPIOD->MODER &= ~(GPIO_MODER_MODER7 | GPIO_MODER_MODER6 | GPIO_MODER_MODER5 | GPIO_MODER_MODER3 | GPIO_MODER_MODER2 | GPIO_MODER_MODER1 | GPIO_MODER_MODER0); //Clears pins 
	GPIOD->MODER |= (GPIO_MODER_MODER7_0 | GPIO_MODER_MODER6_0 | GPIO_MODER_MODER5_0 | GPIO_MODER_MODER3_0 | GPIO_MODER_MODER2_0 | GPIO_MODER_MODER1_0 | GPIO_MODER_MODER0_0 ); //Set pins to output

//STEP 3: Set OTYPER of GPIOD Pins 7, 6, 5, 3, 2, 1 & 0 as push-pull
	GPIOD->OTYPER &= ~(GPIO_OTYPER_OT7 | GPIO_OTYPER_OT6 | GPIO_OTYPER_OT5 | GPIO_OTYPER_OT3 | GPIO_OTYPER_OT2 | GPIO_OTYPER_OT1 | GPIO_OTYPER_OT0);
 
//Done with LCD port Initialization
}

/*******************************
 * LCD_init()
 * Inputs: NONE
 * Outputs: NONE
 * LCD Initialization
 * Read the manual carefully
 * We are doing INITIALIZATION BY INSTRUCTION
 * Don't rush it.
 *******************************
 */

void LCD_init(){

// STEP 1: Wait for 100ms for power-on-reset to take effect

// OK - nothing needs to be done here. 
// Keep the board powered. By the time your code is downloaded
// to flash and you are ready to start execution using the 
// debugger - 100ms will have passed

// STEP 2: Set RS pin LOW to send instructions
	clear_PIN(RS);

// Send instructions using following format:
// Check BF; Set EN=HIGH; Send 4-bit instruction; Set EN=low;

// STEP 3a-3d: Set 4-bit mode (takes a total of 4 steps)
	LCD_sendInstr(0x3);
	LCD_sendInstr(0x3);
	LCD_sendInstr(0x3);
	LCD_sendInstr(0x2);

// STEP 4: Set 2 line display -- treats 16 char as 2 lines
//			001DL NF** (DL 0: 4bits; N= 1: 2 lines; F=0 : 5x8 display
	LCD_sendInstr(0x2);
	LCD_sendInstr(0x8);

// STEP 5: Set DISPLAY to OFF
	LCD_sendInstr(0x0);
	LCD_sendInstr(0x8);

// STEP 6: CLEAR DISPLAY
	LCD_sendInstr(0x0);
	LCD_sendInstr(0x1);

// STEP 7: SET ENTRY MODE - Auto increment; no scrolling
	LCD_sendInstr(0x0);
	LCD_sendInstr(0x6);

// STEP 8: Set Display to ON with Cursor and Blink.
	LCD_sendInstr(0x0);
	LCD_sendInstr(0xF);

}

/*******************************
 * place_lcd_cursor()
 * Inputs: unsigned character
 * Outputs: NONE
 * sets Cursor position to
 * Line 1, character 1 (hex address 0x80)
 * or Line 2, character 1 (hex addres 0xC0)
 *
 *******************************
 */

void place_lcd_cursor(uint8_t lineno){
	if(lineno == 1) {
		LCD_sendInstr(0x8);
		LCD_sendInstr(0x0);
	} else {
		LCD_sendInstr(0xC);
		LCD_sendInstr(0x0);
	}
}



/*******************************
 * LCD_sendData()
 * Inputs: unsigned character data (8-bit)
 * Outputs: NONE
 * writes the character to LCD.
 * Since we are using 4-bit mode
 * this function will take the character (8-bit)
 * transmit upper 4 bits and then lower 4 bits.
 * make sure the RS, RW and EN signals are set to correct value
 * for each 4-bit. 
 * also make sure to check the BF
 *******************************
 */

void LCD_sendData(uint8_t data)
{
	check_BF();
	
	clear_PIN(RW); //Set to 0 for write
	set_PIN(RS); //Set to 1 for Data
	
	set_PIN(EN);
	
	//Send data of 4 bits
	GPIOD->ODR &= ~(0xFu);
	GPIOD->ODR |= (data & 0xF);
	
	clear_PIN(EN);
}

/*******************************
 * LCD_sendInstr()
 * Inputs: unsigned character INSTRUCTION (8-bit)
 * Outputs: NONE
 * Sends commands to LCD
 * We are using 4-bit mode but 
 * this function accepts (8-bit) character
 * as input. You can make the call on how to handle that.
 * make sure the RS, RW and EN signals are set to correct value
 * for each 4-bit part of instructions. 
 * also make sure to check the BF
 *******************************
 */

void LCD_sendInstr(uint8_t Instruction)
{
	check_BF();
	
	clear_PIN(RW); //Set to 0 for write
	clear_PIN(RS); //Set to 0 for Insructions
	
	set_PIN(EN);
	
	//Send Instruction of 4 bits
	GPIOD->ODR &= ~(0xFu);
	GPIOD->ODR |= (Instruction & 0xF);
	
	clear_PIN(EN);
}

/*******************************
 * clear_PIN()
 * Inputs: an integer PIN NUMBER (e.g. RW, EN)
 * Outputs: NONE
 * CLEARS PIN in GPIOD to 0
 * Read the Reference manual carefully
 * you can use the BSRR register without masks
 * OR you can use the ODR register WITH &~ (AND-NOT) mask 
 * to clear ONE specified pin.
 *******************************
 */
void clear_PIN(int PINNO){
	GPIOD->ODR &= ~(1u<<PINNO);
}

/*******************************
 * set_PIN()
 * Inputs: an integer PIN NUMBER (e.g. RW, EN)
 * Outputs: NONE
 * SETS PIN in GPIOD to 1
 * Read the Reference manual carefully
 * you can use the BSRR register without masks
 * OR you can use the ODR register WITH | (OR) mask 
 * to SET ONE specified pin.
 *******************************
 */
void set_PIN(int PINNO){
	GPIOD->ODR |= (1u<<PINNO);
}

/*******************************
 * check_BF()
 * Inputs: NONE
 * Outputs: NONE
 * Checks BF flag on DB7 pin of LCD
 * and prevents code from moving ahead
 * if the BF flag is 1 (indicating LCD busy)
 *******************************
 */

void check_BF(){
	// STEP 1: Clear RS (set RS=0) as reading flag is an instruction
	clear_PIN(RS);
		
	// STEP 2: set Data Pin 7 connected to GPIOD Pin 3 as input 
	// 		   (no pull-up or pull down setup needed here)
	GPIOD->MODER &= ~(3u<<(DB7*2)); //Need to Clear both bits
	
	// STEP 3: Set RW = 1 to read the BF flag.
	set_PIN(RW);

	// STEP 4: Set EN = 1
	set_PIN(EN);

	// STEP 5: Read the BUSY FLAG on Pin 3 of GPIOD.
	//		   Wait here if BUSY and keep reading pin  
	//         until BF becomes 0 indicating NOT BUSY.
	bf = GPIOD->IDR & (1u<<DB7);

	while(bf) {
		bf = GPIOD->IDR & (1u<<DB7);
	}
	
	// STEP 6: CLEAR EN =0
	clear_PIN(EN);
	
	//STEP 7: CLEAR RW =0 
	clear_PIN(RW);

	//STEP 8: Set Data Pin 7 connected to GPIOD Pin 3 as output 
	//Set 0 bit in Pin 3 MODER
	GPIOD->MODER |=(1u<<(DB7*2)); //Need to set first bit of 2

}

void LCD_print_float(float num) {
	char str[16];
	sprintf(str, "%6.3f", num);
	LCD_print_string(str);
}

void LCD_print_int(int num) {
	char str[16];
	sprintf(str, "%d", num);
	LCD_print_string(str);
}

void reverse_array(char arr[]) {
	char temp;
	unsigned int j = strlen(arr)-1;
	for(unsigned int i=0; i< j ; i++) {
		temp = arr[i];
		arr[i] = arr[j];
		arr[j] = temp;
		j--;
	}
}

void LCD_print_character(char c) {
	uint8_t upper = 0xF & (c>>4);
	uint8_t lower = 0xF & c;
	LCD_sendData(upper); 
	LCD_sendData(lower); 
}

void LCD_print_string(char str[]) {
	unsigned int j = strlen(str);
	for(unsigned int i=0; i < j; i++) {
		LCD_print_character(str[i]);
	}
}

void LCD_clear() {
	LCD_sendInstr(0x0);
	LCD_sendInstr(0x1);
}

void print_seconds(int secs) {
	LCD_clear();
	LCD_print_int(secs);
	LCD_print_string(" secs");
}


