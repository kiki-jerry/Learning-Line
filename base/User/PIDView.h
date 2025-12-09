#ifndef __PIDVIEW_H
#define __PIDVIEW_H

#include "stm32f10x.h"   // 提供 uint8_t、uint16_t 等类型
#include "PID.h"         // 需要引用 PID_t 结构体定义

// 初始化 PID 参数显示界面（初始化界面上的标签等）
void PIDView_Init(void);

// 显示当前的 PID 参数值（Kp, Ki, Kd）及其他需要的数据
void PIDView_Display(PID_t *PID_Ptr,float angle_value,int16_t ave_pwm_value,uint8_t run_flag_value);

#endif
