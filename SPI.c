#include "SPI.h"      
#include "stm32f4xx.h"
#include "math.h" 

//volatile signed char data_x[20]; //code variables
//volatile signed char data_y[20];

//volatile signed char data_x[150]; testing variables
//volatile signed char data_y[150];
//volatile signed char data_z[150];

void SPI_init()
{
	
	int p;
	int whoami;
	int ENABLE;
	
//iniailize clocks for SPI and GPIO A, GPIOE
	RCC -> AHB1ENR |= RCC_AHB1ENR_GPIOAEN;
	RCC -> APB2ENR |= RCC_APB2ENR_SPI1EN;
	RCC -> AHB1ENR |= RCC_AHB1ENR_GPIOEEN;

//set GPIO A pins 5-7 to AF, E3 to ODR
	GPIOA -> MODER |= GPIO_MODER_MODE5_1;
	GPIOA -> MODER |= GPIO_MODER_MODE6_1;
	GPIOA -> MODER |= GPIO_MODER_MODE7_1;
	GPIOE -> MODER |= GPIO_MODER_MODE3_0;
	
//set GPIOA AFL to AF 5
	GPIOA -> AFR[0] |= GPIO_AFRL_AFRL5_2;
	GPIOA -> AFR[0] |= GPIO_AFRL_AFRL5_0;
	GPIOA -> AFR[0] |= GPIO_AFRL_AFRL6_2;
	GPIOA -> AFR[0] |= GPIO_AFRL_AFRL6_0;
	GPIOA -> AFR[0] |= GPIO_AFRL_AFRL7_2;
	GPIOA -> AFR[0] |= GPIO_AFRL_AFRL7_0;	
	
//set push pull on E3
	GPIOE -> OTYPER &= ~GPIO_OTYPER_OT3;
	
	//set CS pin high 
	GPIOE -> ODR |= GPIO_ODR_OD3;
	
	//1. Select the BR[2:0] bits to define the serial clock baud rate (see SPI_CR1 register).
	
	SPI1 -> CR1 |= SPI_CR1_BR_0; //writes a 001 that sets spi clk to fpclk/16 (1Mhz)
	SPI1 -> CR1 |= SPI_CR1_BR_1;
	
//2. Select the CPOL and CPHA bits to define one of the four relationships between the 
//data transfer and the serial clock (see Figure 248). This step is not required when the 
//TI mode is selected.
	
	SPI1 -> CR1 |= SPI_CR1_CPOL;
	SPI1 -> CR1 |= SPI_CR1_CPHA;
	
//3. Set the DFF bit to define 8- or 16-bit data frame format (clears for 8bit)
	
	SPI1 -> CR1 &= ~SPI_CR1_DFF;
	
//4. Configure the LSBFIRST bit in the SPI_CR1 register to define the frame format. This 
//step is not required when the TI mode is selected.
	
 SPI1 -> CR1 &= ~SPI_CR1_LSBFIRST;
	
//5. If the NSS pin is required in input mode, in hardware mode, connect the NSS pin to a 
//high-level signal during the complete byte transmit sequence. In NSS software mode, 
//set the SSM and SSI bits in the SPI_CR1 register. If the NSS pin is required in output 
//mode, the SSOE bit only should be set. This step is not required when the TI mode is 
//selected.
	
	SPI1 -> CR1 |= 3<<8;
	
//6. Set the FRF bit in SPI_CR2 to select the motorolla protocol for serial communications.

	SPI1 -> CR2 &= ~SPI_CR2_FRF;

//7. The MSTR and SPE bits must be set (they remain set only if the NSS pin is connected 
//to a high-level signal).


	SPI1 -> CR1 |= SPI_CR1_MSTR; //this bit goes away after the next line and I think this is a reason it doesn't work
	SPI1 -> CR1 |= SPI_CR1_SPE;
	
		GPIOE-> ODR |= (1<<3);
	GPIOE-> ODR &= ~GPIO_ODR_OD3;
	p = spi_send(0x8F);
	whoami = spi_send(0x0); // read from WHOAMI
	GPIOE-> ODR|= (1<<3);// GPIO_ODR_OD3;
	
	GPIOE-> ODR &= ~GPIO_ODR_OD3;
	p = spi_send(0x20);
	p = spi_send(0x57); //enables X, Y, and Z (50 Hz)
	GPIOE-> ODR|= (1<<3);// GPIO_ODR_OD3;


	GPIOE-> ODR &= ~GPIO_ODR_OD3;
	p = spi_send(0xA0);
	ENABLE = spi_send(0x0); //reads CR1 (should be a F27, it reads a 7, which is default, so the write doesn't work)
	GPIOE-> ODR|= (1<<3);// GPIO_ODR_OD3;

	NVIC_ClearPendingIRQ(TIM3_IRQn);
	NVIC_SetPriority(TIM3_IRQn,0);
	NVIC_EnableIRQ(TIM3_IRQn);

}

void TIM3_init(){
	RCC -> APB1ENR |= RCC_APB1ENR_TIM3EN;
	TIM3 -> PSC = 15999;
	TIM3 -> ARR = 49; 
	TIM3 -> CNT = 0;
	TIM3 -> SR &= ~TIM_SR_UIF;
	TIM3 -> DIER |= TIM_DIER_UIE;
	TIM3  -> CR1 |= TIM_CR1_CEN;
	
}

int SPI_Read(unsigned int address) //sends an address byte and returns the data register
{
//	address = 0x0F;
	//drive the CS low
	GPIOE -> ODR &= ~GPIO_ODR_OD3;
	//send a read bit (0) and address to read from
	SPI1 -> DR = address;
	
	while (!(SPI1 -> SR & SPI_SR_RXNE)); //waits for recieve to finish, gets stuck here
	GPIOE -> ODR |= GPIO_ODR_OD3; //sets CS bit to end transmission

	return(SPI1 -> DR);

//GPIOE->ODR &= ~(1<<5);	//Reset bit 5
//SPI1->DR = address;
//while (~(SPI1->SR & SPI_SR_RXNE));	/* Wait for send to finish */
//return (SPI1->DR);
//GPIOE->ODR |= 1<<5;	//set bit 5 high
}


void SPI_Write(unsigned int address,unsigned int data) //sends and address byte and a data byte
{
	int mi = 0;
	address = 0x4BU;
	data = 0x8AU;
	//drive the CS low
	GPIOE -> ODR &= ~GPIO_ODR_OD3;
	
	//send address
	SPI1 -> DR |= (address | 1U<<7);

	while (!(SPI1 -> SR & SPI_SR_TXE)); //waits for the buffer to be empty before sending data (gets stuck here)
	mi = SPI1 -> DR;

	SPI1 -> SR &= ~SPI_SR_TXE;
	
	//send data
	SPI1 -> DR &= 0;
	SPI1 -> DR |= data;
	while (!(SPI1 -> SR & SPI_SR_TXE)); //waits for the buffer to be empty before sending data (gets stuck here)
	mi = SPI1 -> DR;


	//wait again
//	while ((SPI1 -> SR & SPI_SR_RXNE));
	
	GPIOE -> ODR |= GPIO_ODR_OD3;//sets CS bit to end transmission

}
unsigned char spi_send (unsigned char byte) //sends a byte
{
SPI1->DR = byte;
while (!(SPI1->SR & SPI_SR_RXNE));	/* Wait for send to finish */
return (SPI1->DR);
}

	void shift(signed char Array[8])
	{
	int temp = 0;
	for(int i = 7; i >= 0; i--)
		Array[i] = Array[i-1];
	}
	
	
	signed int max(signed char Array[20])
	{
		int MAX = Array[0];
	for(int i = 1; i<=19; i++)
		{
		if(Array[i] > MAX)
			MAX = Array[i];
		}
		return MAX;
	}
	
	signed int min(signed char Array[20])
		{
		int MIN = Array[0];
	for(int i = 1; i<=19; i++)
		{
		if(Array[i] < MIN)
			MIN = Array[i];
		}
		return MIN;
	}

		double RMS_div8(signed char Array[8])
		{
		double ADD;
		double root_mean_square;
	for(int i = 0; i<=7; i++)
		{
			ADD += Array[i]*Array[i];
		}
		root_mean_square = sqrt(ADD/5);
		return root_mean_square/8;
	}
//	void TIM3_IRQHandler(){
//	int m;
//	int MINX;
//	int MAXX;
//	int MINY;
//	int MAXY;
//	shift(data_x);
//	shift(data_y);
//	GPIOE-> ODR &= ~GPIO_ODR_OD3;
//	m = spi_send(0xA9); // read from X_outH (0)
//	data_x[0] = spi_send(0x0);
//	GPIOE-> ODR |= (1<<3);// GPIO_ODR_OD3;
//	
//	GPIOE-> ODR &= ~GPIO_ODR_OD3;
//	m = spi_send(0xAB); // read from y_outH
//	data_y[0] = spi_send(0x0);
//	GPIOE-> ODR |= (1<<3);// GPIO_ODR_OD3;
//	
//	MINX = min(data_x);
//	MINY = min(data_y);
//	MAXX = max(data_x);
//	MAXY = max(data_y);
//	
//	if ((MAXX - MINX) > 30) //some number from testing)
//	{
//	//stop motor, change state
//	}
//	if ((MAXY - MINY) > 30) //some number from testing)
//	{
//	//stop motor, change state
//	}
////	int m;
////	static int a = 0;
////	TIM3 -> SR &= ~TIM_SR_UIF; //make sure interrupt flag is off	
////	NVIC_ClearPendingIRQ(TIM3_IRQn);
////	if (a<= 150){
////	GPIOE-> ODR &= ~GPIO_ODR_OD3;
////	m = spi_send(0xA9); // read from X_outH (0)
////	data_x[a] = spi_send(0x0);
////	GPIOE-> ODR |= (1<<3);// GPIO_ODR_OD3;
////	
////	GPIOE-> ODR &= ~GPIO_ODR_OD3;
////	m = spi_send(0xAB); // read from y_outH
////	data_y[a] = spi_send(0x0);
////	GPIOE-> ODR |= (1<<3);// GPIO_ODR_OD3;
////	
////	GPIOE-> ODR &= ~GPIO_ODR_OD3;
////	m = spi_send(0xAD); // read from z_outH (1)
////	data_z[a] = spi_send(0x0) - 0x45;
////	GPIOE-> ODR |= (1<<3);// GPIO_ODR_OD3;
////	a++;
////	}
//	}




	