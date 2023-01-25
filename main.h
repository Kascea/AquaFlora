#include "stm32f4xx.h"                  // Device header
#include "Final_Functions.c"
#include "LCD.h"
#include "Buttons.h"
#include "Keypad.h"
#include <stdio.h>

typedef enum {START, KEYPAD_ENTRY, IDLE, SCAN, WATERING, WATER_LOW} state;
volatile state current;


