#include "stm32f4xx.h" 
/*******************************
 * LCD pins connections to PORTD
 *******************************
 */
#define RS 7
#define RW 6 
#define EN 5

#define DB7 3
#define DB6 2
#define DB5 1
#define DB4 0  

/*******************************
 * FUNCTION PROTOTYPES
 *******************************
 */
// LCD related functions
void LCD_port_init(void);
void LCD_init(void);
void LCD_sendData(unsigned char data);
void LCD_sendInstr(unsigned char Instruction);
void place_lcd_cursor(unsigned char lineno);

// PIN set and clear functions
void clear_PIN(int PINNO);
void set_PIN(int PINNO);
void check_BF(void);

//Print functions
void LCD_print_int(int num);
void LCD_print_float(float num);
void LCD_print_character(char c);
void LCD_print_string(char str[16]);

//Helper functions
void reverse_array(char* arr);
void LCD_clear(void);
void print_seconds(int seconds);

// END Functions
