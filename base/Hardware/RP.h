#ifndef __RP_H
#define __RP_H

#include "stm32f10x.h"  // STM32 标准外设库头文件 stm32f10x_header

// 旋转编码器初始化函数（使用 PA6 = A 相，PA7 = B 相）
// rp_init_function
void RP_Init(void);

// 清零旋转编码器计数（原始脉冲和换算后的值都会归零）
// rp_reset_function
void RP_Reset(void);

// 获取旋转编码器原始脉冲计数值（每次状态跳变记一次，正负表示方向）
// encoder_index 编码器索引，目前只有1路，写1即可 encoder_index
// rp_get_pulse_count_function
int16_t RP_GetPulseCount(uint8_t encoder_index);

// 获取旋转编码器换算后的数值（每拧一格 = 0.1，顺时针为正，逆时针为负）
// encoder_index 编码器索引，目前只有1路，写1即可 encoder_index
// rp_get_value_function
float RP_GetValue(uint8_t encoder_index);

#endif
