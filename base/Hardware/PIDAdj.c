#include "PIDAdj.h"
#include "OLED.h"
#include "Key.h"
#include "RP.h"
#include "PID.h"
#include <stdio.h> // for NULL check if needed


// --- 内部静态变量 (私有变量) ---
static uint8_t select_index  = 0;    // 当前选中的参数: 0:Kp, 1:Ki, 2:Kd
static uint8_t edit_mode     = 0;       // 模式: 0:选择模式(闪烁), 1:修改模式(常亮)
static uint8_t blink_state   = 0;  // 闪烁标志位: 0=显示, 1=隐藏

// 用于增量修改的基准值
static float base_param_value = 0.0f; // 进入编辑时的参数基准值 base_param_value
static float base_rp_value    = 0.0f; // 进入编辑时的旋钮基准值   base_rp_value

// 显示单个参数的内部函数
static void PIDAdj_ShowItem(uint8_t row_pos, float value,
                            uint8_t is_selected, uint8_t is_editing);

// --- 函数实现 ---

void PIDAdj_Init(void)
{
	select_index = 0;
	edit_mode    = 0;
	blink_state  = 0;
	// 初始化 OLED 上的静态标签
	// 注意：OLED_Init() 需在 main 中先调用
	// 这里不清屏，由 main 在切换界面时负责 OLED_Clear()
	OLED_ShowString(0, 0,  "PID Edit", OLED_6X8);
	OLED_ShowString(0, 8,  "Kp:",      OLED_6X8);
	OLED_ShowString(0, 16, "Ki:",      OLED_6X8);
	OLED_ShowString(0, 24, "Kd:",      OLED_6X8);
	OLED_ShowString(0, 40, "Mode:View",OLED_6X8);
	OLED_Update();
}

void PIDAdj_TimerTask(void)
{
	static uint16_t blink_count  = 0;
    
	// 控制闪烁频率
	blink_count ++;
	if(blink_count  >= 300){ // 约 20 次中断翻转一次
	blink_state  = !blink_state ;
	blink_count  = 0;
	}
}
// 内部显示函数：显示某一行的参数值
// row_pos   行位置（y坐标，如 8 / 16 / 24）
// value     要显示的数值
// is_selected 是否为当前选中项
// is_editing  是否处于编辑模式
static void PIDAdj_ShowItem(uint8_t row_pos, float value,
                            uint8_t is_selected, uint8_t is_editing)
{
    if (is_selected)
    {
        if (is_editing)
        {
            // 编辑模式：常亮+前面显示 '>'
            OLED_ShowChar(24, row_pos, '>', OLED_6X8);
            OLED_Printf(32, row_pos, OLED_6X8, "%1.2f", value);
        }
        else
        {
            // 选中但未编辑：闪烁数值，前面空格
            OLED_ShowChar(24, row_pos, ' ', OLED_6X8);
            if (blink_state == 0)
            {
                OLED_Printf(32, row_pos, OLED_6X8, "%1.2f", value);
            }
            else
            {
                // 显示空白实现闪烁效果
                OLED_ShowString(32, row_pos, "      ", OLED_6X8);
            }
        }
    }
    else
    {
        // 未选中：常亮，无引导符
        OLED_ShowChar(24, row_pos, ' ', OLED_6X8);
        OLED_Printf(32, row_pos, OLED_6X8, "%1.2f", value);
    }
}


// 调参主逻辑：现在按键从 main 传进来，不再自己调用 Key_GetNum()
void PIDAdj_Run(PID_t *PID_Ptr, uint8_t key_number)
{
    uint8_t key_number_local = key_number; // 当前按键编号 key_number_local

    // 1. 处理按键：key=3 切换选中项
    if (key_number_local == 3)
    {
        // 如果正在编辑，先退出编辑模式
        if (edit_mode == 1)
        {
            edit_mode = 0;
            OLED_ShowString(0, 40, "Mode:View", OLED_6X8);
        }
        else
        {
            // 在 Kp/Ki/Kd 之间循环选择
            select_index++;
            if (select_index > 2) select_index = 0;
        }
    }

    // 2. 处理按键：key=4 进入/退出编辑模式
    if (key_number_local == 4)
    {
        if (edit_mode == 0)
        {
            // 进入编辑模式：记录基准旋钮值 & 基准参数值
            edit_mode      = 1;
            base_rp_value  = (float)RP_GetValue(1); // 当前旋钮值 base_rp_value
            if (select_index == 0)      base_param_value = PID_Ptr->Kp;
            else if (select_index == 1) base_param_value = PID_Ptr->Ki;
            else if (select_index == 2) base_param_value = PID_Ptr->Kd;

            OLED_ShowString(0, 40, "Mode:Edit", OLED_6X8);
        }
        else
        {
            // 退出编辑模式
            edit_mode = 0;
            OLED_ShowString(0, 40, "Mode:View", OLED_6X8);
        }
    }

    // 3. 旋钮调节逻辑（仅在编辑模式下生效）
    if (edit_mode == 1)
    {
        float current_rp_value = (float)RP_GetValue(1); // 当前旋钮值 current_rp_value
        float delta_value      = current_rp_value - base_rp_value; // 变化量 delta_value

        // 这里仍然是“基准值 + 变化量”，保持你之前的逻辑；
        // 如果你之前已经做了“每一格=0.01”的映射，可以把下面改成:
        // float new_value = base_param_value + delta_value * 0.01f;
        float new_value = base_param_value + delta_value;

        // 限制到 0~1
        if (new_value > 10.0f) new_value = 10.0f;
        if (new_value < 0.0f) new_value = 0.0f;

        // 写回 PID 结构体
        if (select_index == 0)      PID_Ptr->Kp = new_value;
        else if (select_index == 1) PID_Ptr->Ki = new_value;
        else if (select_index == 2) PID_Ptr->Kd = new_value;
    }

    // 4. 刷新编辑界面显示（只画编辑界面的区域）
    PIDAdj_ShowItem(8,  PID_Ptr->Kp, (select_index == 0), edit_mode);
    PIDAdj_ShowItem(16, PID_Ptr->Ki, (select_index == 1), edit_mode);
    PIDAdj_ShowItem(24, PID_Ptr->Kd, (select_index == 2), edit_mode);
    OLED_Update();
}

