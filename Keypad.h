#include "stm32f4xx.h"
#include "LCD.h"
#include <string.h>

void keypad_init(void);
void set_columns_output(void);
void set_columns_input(void);
void set_rows_output(void);
void set_rows_input(void);

char read_input(void);
char convert_input(int row, int col);
void idle_timer_init(uint32_t load_reg_value);
volatile static int timerVal;
volatile static char keypad[4][3] = {{'1', '2', '3'}, {'4', '5', '6'}, {'7', '8', '9'}, {'*', '0', '#'}};
volatile static char inputString[16] = "";


