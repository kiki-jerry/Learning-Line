#ifndef __PIDADJ_H
#define __PIDADJ_H

#include "stm32f10x.h"   // 这里提供 uint8_t 等类型定义
#include "PID.h" // 需要引用PID结构体定义

// 初始化 PID 调参模块 (初始化界面静态文字等)
void PIDAdj_Init(void);

// 调参主逻辑，请在主循环 while(1) 中调用
// 参数：传入需要调节的 PID 结构体指针
void PIDAdj_Run(PID_t *PID_Ptr, uint8_t key_number);

// 定时任务，用于控制光标闪烁
// 请在定时器中断中调用 (建议 10ms 或 20ms 调用一次)
void PIDAdj_TimerTask(void);

#endif
