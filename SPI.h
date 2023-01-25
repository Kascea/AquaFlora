void SPI_init(void);
int SPI_Read(unsigned int address);
void SPI_Write(unsigned int address,unsigned int data);
unsigned char spi_send (unsigned char byte);
void TIM3_IRQHandler(void);
void shift(signed char Array[20]);
signed int max(signed char Array[20]);
signed int min(signed char Array[20]);
void TIM3_init(void);
double RMS_div8(signed char Array[8]);


		
