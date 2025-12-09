#include "stm32f10x.h"   // 提供 uint8_t、uint16_t 等类型
#include "PIDView.h"
#include "OLED.h"
#include "PID.h"


// 参数显示界面初始化（显示标签）
void PIDView_Init(void)
{
    OLED_ShowString(0, 0,  "PID View", OLED_6X8);      // 文字初始化，显示“PID View”
    OLED_ShowString(0, 8,  "Kp:", OLED_6X8);            // Kp 标签
    OLED_ShowString(0, 16, "Ki:", OLED_6X8);            // Ki 标签
    OLED_ShowString(0, 24, "Kd:", OLED_6X8);            // Kd 标签
    OLED_ShowString(0, 32, "Ang:", OLED_6X8);           // Ang 标签
    OLED_ShowString(0, 40, "AvePWM:", OLED_6X8);        // AvePWM 标签
    OLED_ShowString(0, 48, "RunFlag:", OLED_6X8);       // RunFlag 标签
    OLED_Update();    // 更新显示
}

// 参数显示界面：显示当前 PID 参数值及其他信息
void PIDView_Display(PID_t *PID_Ptr,float angle_value,int16_t ave_pwm_value,uint8_t run_flag_value){
	// 显示当前的 PID 参数值（Kp, Ki, Kd）以及其他信息（Angle, AvePWM, RunFlag）
	OLED_Printf(0, 8, OLED_6X8, "Kp:%1.2f", PID_Ptr->Kp);  // Kp
	OLED_Printf(0, 16, OLED_6X8, "Ki:%1.2f", PID_Ptr->Ki);  // Ki
	OLED_Printf(0, 24, OLED_6X8, "Kd:%1.2f", PID_Ptr->Kd);  // Kd
	
	OLED_Printf(0, 32, OLED_6X8, "Ang:%5.2f", angle_value);       // Angle
	OLED_Printf(0, 40, OLED_6X8, "AvePWM:%4d", ave_pwm_value);     // AvePWM
	OLED_Printf(0, 48, OLED_6X8, "RunFlag:%d", run_flag_value);    // RunFlag
    
    OLED_Update();    // 更新显示
}
