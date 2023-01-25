#include "stm32f4xx.h"                  // Device header

void water_level_init(void);
int read_water_level(void);
void stepper_motor_init(void);
void driveMotor(int direction);
void Motor_Time_Init(void);
void SysTick_Handler(void);
void myDelay(uint32_t TimeToDelay);
void water_valve_init(void);
void water_control(int mode);
