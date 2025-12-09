#include "stm32f10x.h"                  // Device header
#include "PID.h"

void PID_Init(PID_t *p){
	p->Target = 0;
	p->Actual = 0;
	p->Out = 0;
	p->Error0 = 0;
	p->Error1 = 0;
	p->ErrorInt = 0;
}

void PID_Update(PID_t *p)
{
	p->Error1 = p->Error0;
	p->Error0 = p->Target - p->Actual;
	
	if (p->Ki != 0)
	{
		p->ErrorInt += p->Error0;
		//【新增代码：积分限幅】 ====================
		// 防止积分项无限累积导致“飞车”
		// 这里的限幅值可以根据你的 OutMax 和 Ki 来估算。
		// 例如：如果你希望积分项最多贡献满速(100)的输出，且 Ki=0.1，
		// 那么积分限幅 = 100 / 0.1 = 1000。这里取一个安全值 2000。
//		if (p->ErrorInt > 2000)  p->ErrorInt = 2000;
//		if (p->ErrorInt < -2000) p->ErrorInt = -2000;
	}
	else
	{
		p->ErrorInt = 0;
	}
	
	p->Out = p->Kp * p->Error0
		   + p->Ki * p->ErrorInt
		   + p->Kd * (p->Error0 - p->Error1);
	
	if (p->Out > p->OutMax) {p->Out = p->OutMax;}
	if (p->Out < p->OutMin) {p->Out = p->OutMin;}
}
