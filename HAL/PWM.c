#include "stm32f1xx_hal.h"
#include "main.h"

extern TIM_HandleTypeDef htim1; // 声明在 tim.c 中定义的 TIM1 句柄

void PWM_Init(void)
{
	// 1. 启动 PWM 通道
	HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1);
	HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_4); 

	__HAL_TIM_MOE_ENABLE(&htim1);
}

void PWM_SetCompare4(uint16_t Compare)
{
	// 直接修改 TIM1 句柄中 CCR4 寄存器的值
	// HAL 库宏定义: __HAL_TIM_SET_COMPARE(htim_pointer, Channel, Compare_value)
	__HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_4, Compare);
}

void PWM_SetCompare1(uint16_t Compare)
{
	// 直接修改 TIM1 句柄中 CCR1 寄存器的值
	__HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, Compare);
}
