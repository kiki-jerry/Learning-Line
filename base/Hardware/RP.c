#include "stm32f10x.h"  // STM32 标准外设库头文件 stm32f10x_header
#include "RP.h"

//================ 配置参数宏 =================//

// 每一格（“咔哒”一下）对应的脉冲数量（你现在实测是4个脉冲）
// rp_pulses_per_step_macro
#define RP_PULSES_PER_STEP   4  

// 每一格对应的“值 × 10”，1 就是 0.1，2 就是 0.2，以此类推
// 每一格对应的“值 × 100”，1 就是 0.01，2 就是 0.02，以此类推
// rp_step_value_times10_macro
#define RP_STEP_VALUE_X100    1  

//================ 模块内部全局变量 =================//

// 旋转编码器原始脉冲计数值（中断中递增或递减，反映旋转方向和多少步）
// g_rp_pulse_count 全局旋转编码器脉冲计数 global_rp_pulse_count
volatile int16_t g_rp_pulse_count = 0;

// 上一次 A/B 两相组合成的2bit状态（00/01/10/11），用于查表判断方向
// g_rp_last_state 上一次AB相状态编码 global_rp_last_state
static uint8_t g_rp_last_state = 0;

//================ 内部函数声明 =================//

// 配置GPIO和EXTI中断（PA6/PA7 → 编码器A/B相）
// rp_gpio_exti_config_function
static void RP_GPIO_EXTI_Config(void);

//================ 对外接口实现 =================//

void RP_Init(void)
{
    RP_GPIO_EXTI_Config();  // 调用内部函数完成GPIO与EXTI配置 rp_gpio_exti_config_call
}

void RP_Reset(void)
{
    g_rp_pulse_count = 0;   // 清零原始脉冲计数 rp_pulse_count_reset
}

int16_t RP_GetPulseCount(uint8_t encoder_index)
{
    (void)encoder_index;    // 当前只有1路编码器，忽略索引参数 encoder_index_unused
    return g_rp_pulse_count; // 返回原始脉冲计数 rp_pulse_count_return
}

float RP_GetValue(uint8_t encoder_index)
{
    (void)encoder_index;    // 当前只有1路编码器，忽略索引参数 encoder_index_unused

    int16_t pulse_count = g_rp_pulse_count;              // pulse_count 当前原始脉冲计数值 encoder_pulse_count
    int16_t step_count  = pulse_count / RP_PULSES_PER_STEP; // step_count 换算后的步数（每步=一格）encoder_step_count

    // 每步 = 0.1，对应 放大10倍后为1，所以：数值×10 = 步数 * 1
    int16_t value_x100 = step_count * RP_STEP_VALUE_X100;  // value_x10 当前数值放大10倍后的整数表示 encoder_value_times10

    float value = value_x100 / 100.0f;                     // value 实际浮点数值（每步0.1）encoder_value /value 实际浮点数值（每步0.01）encoder_value

    return value;                                        // 返回旋钮数值 return_encoder_value
}

//================ 内部函数实现 =================//

static void RP_GPIO_EXTI_Config(void)
{
    // 1. 开启GPIOA和AFIO时钟
    // rcc_apb2_clock_cmd_value 时钟使能值
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_AFIO, ENABLE);

    // 2. 配置 PA6 / PA7 为上拉输入（编码器 A/B 相）
    GPIO_InitTypeDef gpio_init_struct;                        // gpio_init_struct GPIO初始化结构体 gpio_init_structure
    gpio_init_struct.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7;      // GPIO_Pin A/B相对应的PA6和PA7 gpio_pins_pa6_pa7
    gpio_init_struct.GPIO_Mode = GPIO_Mode_IPU;               // GPIO_Mode 上拉输入模式 input_pullup_mode
    gpio_init_struct.GPIO_Speed = GPIO_Speed_50MHz;           // GPIO_Speed IO速度50MHz gpio_speed_50mhz
    GPIO_Init(GPIOA, &gpio_init_struct);

    // 读取初始A/B状态，防止第一次中断产生大步长
    uint8_t a_state = GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_6); // a_state 初始A相电平 encoder_a_state
    uint8_t b_state = GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_7); // b_state 初始B相电平 encoder_b_state
    g_rp_last_state = (a_state << 1) | b_state;                 // g_rp_last_state 初始AB组合状态 encoder_ab_last_state

    // 3. 将 PA6 / PA7 连接到 EXTI6 / EXTI7
    GPIO_EXTILineConfig(GPIO_PortSourceGPIOA, GPIO_PinSource6); // EXTI_Line6 连接到PA6 exti_line6_config
    GPIO_EXTILineConfig(GPIO_PortSourceGPIOA, GPIO_PinSource7); // EXTI_Line7 连接到PA7 exti_line7_config

    // 4. 配置 EXTI：Line6 / Line7 上升沿+下降沿都触发
    EXTI_InitTypeDef exti_init_struct;               // exti_init_struct EXTI初始化结构体 exti_init_structure
    EXTI_StructInit(&exti_init_struct);              // 填充默认值 exti_struct_init

    exti_init_struct.EXTI_Mode = EXTI_Mode_Interrupt;            // EXTI_Mode 中断模式 interrupt_mode
    exti_init_struct.EXTI_Trigger = EXTI_Trigger_Rising_Falling; // EXTI_Trigger 上升沿+下降沿触发 rising_falling_trigger
    exti_init_struct.EXTI_LineCmd = ENABLE;                      // EXTI_LineCmd 使能中断线 line_enable

    // 配置 EXTI Line6（对应PA6 / A相）
    exti_init_struct.EXTI_Line = EXTI_Line6;           // EXTI_Line 使用6号中断线 exti_line6
    EXTI_Init(&exti_init_struct);

    // 配置 EXTI Line7（对应PA7 / B相）
    exti_init_struct.EXTI_Line = EXTI_Line7;           // EXTI_Line 使用7号中断线 exti_line7
    EXTI_Init(&exti_init_struct);

    // 5. NVIC 配置 EXTI9_5 中断
    NVIC_InitTypeDef nvic_init_struct;                          // nvic_init_struct NVIC初始化结构体 nvic_init_structure
    nvic_init_struct.NVIC_IRQChannel = EXTI9_5_IRQn;            // NVIC_IRQChannel EXTI9_5中断通道 irqchannel_exti9_5
    nvic_init_struct.NVIC_IRQChannelPreemptionPriority = 2;     // PreemptionPriority 抢占优先级2 preemption_priority_2
    nvic_init_struct.NVIC_IRQChannelSubPriority = 0;            // SubPriority 子优先级0 sub_priority_0
    nvic_init_struct.NVIC_IRQChannelCmd = ENABLE;               // NVIC_IRQChannelCmd 使能中断通道 irq_enable
    NVIC_Init(&nvic_init_struct);
}

//================ 中断服务函数 =================//
// 如果你的工程里已经有 EXTI9_5_IRQHandler，记得把下面逻辑合并进去

void EXTI9_5_IRQHandler(void)
{
    // 如果是 PA6 或 PA7 对应的中断线触发
    if (EXTI_GetITStatus(EXTI_Line6) != RESET || EXTI_GetITStatus(EXTI_Line7) != RESET)
    {
        // 1. 读取当前A/B相状态
        uint8_t a_state = GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_6); // a_state 当前A相电平 encoder_a_state_now
        uint8_t b_state = GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_7); // b_state 当前B相电平 encoder_b_state_now
        uint8_t new_state = (a_state << 1) | b_state;               // new_state 当前AB组合状态 encoder_ab_new_state

        // 2. 使用16项查表法解析方向与步数
        // index = (上一次状态 << 2) | 当前状态
        // 返回值：+1 = 正向一小步；-1 = 反向一小步；0 = 抖动/非法跳变
        static const int8_t trans_table[16] = {
            0, -1,  1,  0,
            1,  0,  0, -1,
           -1,  0,  0,  1,
            0,  1, -1,  0
        }; // trans_table 旋转编码器状态转换查表 encoder_transition_table

        uint8_t index = (g_rp_last_state << 2) | new_state; // index 查表索引 encoder_transition_index
        int8_t  delta = trans_table[index];                 // delta 本次脉冲增量 encoder_pulse_delta

        g_rp_pulse_count += delta;                          // 累加原始脉冲计数 encoder_pulse_count_accumulate

        g_rp_last_state = new_state;                        // 更新上一次AB状态 encoder_ab_last_state_update

        // 3. 清除中断标志位
        EXTI_ClearITPendingBit(EXTI_Line6 | EXTI_Line7);    // 清除6与7号中断线标志 exti_pendingbit_clear
    }
}
