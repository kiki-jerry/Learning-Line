#include "stm32f10x.h"                  // Device header
#include "Delay.h"
#include "OLED.h"
#include "LED.h"
#include "Timer.h"
#include "Key.h"
#include "MPU6050.h"
#include "Motor.h"
#include "Encoder.h"
#include "Serial.h"
#include "BlueSerial.h"
#include "PID.h"
#include "RP.h"
#include "PIDAdj.h"
#include "PIDView.h"
#include <math.h>
#include <string.h> 
#include <stdlib.h>


/*Test PID*/
int16_t AX, AY, AZ, GX, GY, GZ;
int16_t LeftPWM, RightPWM, AvePWM, DifPWM;

uint16_t TimerCount;
uint8_t TimerErrorFlag;
uint8_t KeyNum, RunFlag;

float AngleAcc;
float AngleGyro;
float Angle;

PID_t AnglePID = {
	.Kp = 0.1,
	.Ki = 0.1,
	.Kd = 0.1,
	.OutMax = 100,
	.OutMin = -100,
};

// 界面模式变量 ui_mode：0 = 参数显示界面  1 = PID 编辑界面
static uint8_t ui_mode = 0;   // 界面模式变量 ui_mode
// 参数显示界面：显示当前 PID 参数和部分状态

int main(void){
	OLED_Init();
	LED_Init();
	Key_Init();
	Motor_Init();
	Encoder_Init();
	MPU6050_Init();
	Serial_Init();
	BlueSerial_Init();
	PIDAdj_Init(); 
	PIDView_Init();
	RP_Init();
	OLED_Clear();
	Timer_Init();
	
	BlueSerial_SendString("hello");
	BlueSerial_Printf("hello");
	
	ui_mode = 0;        // 启动时先进入“参数显示界面”
	
	while(1){
		KeyNum = Key_GetNum();
		
		if(KeyNum == 1){
			if(RunFlag == 0){
				PID_Init(&AnglePID);
				RunFlag = 1;
			}else{
				RunFlag = 0;
			}
			
		}
		
		// key=2：在“显示界面”和“编辑界面”之间切换
		if (KeyNum == 2){
			ui_mode ^= 1;     // 0 <-> 1 切换
			OLED_Clear();
			if (ui_mode == 1){
				// 进入编辑界面，重新画上 Kp/Ki/Kd 标签
				PIDAdj_Init();
			}else{
				PIDView_Init();
			}
				// 如果切回显示界面，下一轮循环里 Show_ParamViewScreen 会画出来
		}
		
		// 2. 根据模式显示界面 / 处理编辑
		if (ui_mode == 0){
			// 参数显示界面
			
			PIDView_Display(&AnglePID, Angle, AvePWM, RunFlag);
			
		}
		else{
			// PID 参数编辑界面，把同一个 KeyNum 传给 PIDAdj
			PIDAdj_Run(&AnglePID, KeyNum);
		}

		if(RunFlag){ LED_ON();}else{ LED_OFF();}
    
		
//		OLED_Printf(0, 0, OLED_8X16, "%+06d", AX);
//		OLED_Printf(0, 16, OLED_8X16, "%+06d", AY);
//		OLED_Printf(0, 32, OLED_8X16, "%+06d", AX);
//		OLED_Printf(64, 0, OLED_8X16, "%+06d", GX);
//		OLED_Printf(64, 16, OLED_8X16, "%+06d", GY);
//		OLED_Printf(64, 32, OLED_8X16, "%+06d", GZ);
//		OLED_Printf(0, 48, OLED_8X16, "Flag:%1d", TimerErrorFlag);
//		OLED_Printf(64, 48, OLED_8X16, "C:%f", Angle);
//		OLED_Update();
		
		Serial_Printf("[plot,%f,%f,%f]\r\n", AngleAcc, AngleGyro, Angle);
		
	}
}

void TIM3_IRQHandler(void)
{
	static uint16_t Count0;
	
	if (TIM_GetITStatus(TIM3, TIM_IT_Update) == SET)
	{
		TIM_ClearITPendingBit(TIM3, TIM_IT_Update);
		
		Key_Tick();
		PIDAdj_TimerTask(); // PID调参模块的定时任务 (用于光标闪烁)
		Count0 ++;
		
		if(Count0 >= 10){
			Count0 = 0;
			
			MPU6050_GetData(&AX, &AY, &AZ, &GX, &GY, &GZ);
			GY += 17;
			AngleAcc = -atan2(AX, AZ) / 3.14159 * 180; 
			//AngleAcc += 2 ;//中心校准
			AngleGyro = Angle + GY / 32768.0 * 2000 * 0.01;
			float Alpha = 0.05;
			Angle = Alpha * AngleAcc + (1 - Alpha) * AngleGyro;
			// 【可选：死区控制】如果角度偏差非常小（例如正负0.5度以内），则忽略误差
			if (Angle > -0.5 && Angle < 0.5) {
			AnglePID.Actual = 0; // 欺骗 PID 说现在是完美的 0 度
			} else {
				AnglePID.Actual = Angle;
			}
			if(Angle > 50 || Angle < -50){
				RunFlag = 0;
			}
			
			if(RunFlag){
				AnglePID.Actual = Angle;
				PID_Update(&AnglePID);
				AvePWM = -AnglePID.Out;
				
				LeftPWM = AvePWM + DifPWM / 2;
				RightPWM = AvePWM - DifPWM / 2;
				
				if(LeftPWM > 100){LeftPWM = 100;}else if(LeftPWM < -100){LeftPWM = -100;}
				if(RightPWM > 100){RightPWM = 100;}else if(RightPWM < -100){RightPWM = -100;}
				
				Motor_SetPWM(1, LeftPWM);
				Motor_SetPWM(2, RightPWM);
				
			}else{
				Motor_SetPWM(1, 0);
				Motor_SetPWM(2, 0);
			}
		}

		
		if (TIM_GetITStatus(TIM3, TIM_IT_Update) == SET){
			TimerErrorFlag = 1;
			TIM_ClearITPendingBit(TIM3, TIM_IT_Update);
		}
		TimerCount = TIM_GetCounter(TIM3);
	}
}

//int main(void){
//	OLED_Init();
//	BlueSerial_Init();
//	
//	BlueSerial_SendString("hello");
//	BlueSerial_Printf("hello");
//	
//	while(1){
//		if(BlueSerial_RxFlag == 1){
//			OLED_Printf(0, 56, OLED_6X8, "%s", BlueSerial_RxPacket);
//			OLED_Update();
//			BlueSerial_RxFlag = 0;
//		}
//	}
//}




