#include "py32f0xx_bsp_tim.h"

void BSP_TIM_config(void)
{
    LL_TIM_InitTypeDef TIMCountInit;

    LL_APB1_GRP2_EnableClock(PWM0_TIM_CLK);
 
    TIMCountInit.ClockDivision       = LL_TIM_CLOCKDIVISION_DIV1;
    TIMCountInit.CounterMode         = LL_TIM_COUNTERMODE_UP;
    TIMCountInit.Prescaler           = 0;
    TIMCountInit.Autoreload          = 1024-1;
    TIMCountInit.RepetitionCounter   = 0;
    LL_TIM_Init(PWM0_TIM,&TIMCountInit);

        
    
    LL_APB1_GRP2_EnableClock(SPD0_TIM_CLK);
 
    TIMCountInit.ClockDivision       = LL_TIM_CLOCKDIVISION_DIV1;
    TIMCountInit.CounterMode         = LL_TIM_COUNTERMODE_UP;
    TIMCountInit.Prescaler           = 16-1;
    TIMCountInit.Autoreload          = 65536-1;
    TIMCountInit.RepetitionCounter   = 0;
    LL_TIM_Init(SPD0_TIM,&TIMCountInit);

    LL_TIM_EnableAllOutputs(SPD0_TIM);
    //使能溢出中断
    LL_TIM_EnableIT_UPDATE(SPD0_TIM);
    NVIC_SetPriority(SPD_UPD_IRQn,2);
}
    
void BSP_PWMChannelConfig(void)
{
    //PWM通道与GPIO配置
    LL_GPIO_InitTypeDef GPIO_InitTypeDef;

    GPIO_InitTypeDef.Pin = PWM0_PIN;
    GPIO_InitTypeDef.Mode = LL_GPIO_MODE_ALTERNATE;
    GPIO_InitTypeDef.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
    GPIO_InitTypeDef.Speed = LL_GPIO_SPEED_FREQ_HIGH;
    GPIO_InitTypeDef.Alternate = PWM0_AF;
    GPIO_InitTypeDef.Pull = LL_GPIO_PULL_NO;
    LL_GPIO_Init(PWM0_PORT, &GPIO_InitTypeDef);

    LL_TIM_OC_InitTypeDef TIM_OC_Initstruct;
    
    TIM_OC_Initstruct.OCMode = LL_TIM_OCMODE_FORCED_INACTIVE;
    TIM_OC_Initstruct.OCState = LL_TIM_OCSTATE_ENABLE;
    TIM_OC_Initstruct.OCPolarity = LL_TIM_OCPOLARITY_HIGH;
    TIM_OC_Initstruct.OCIdleState = LL_TIM_OCIDLESTATE_LOW;
    LL_TIM_OC_Init(PWM0_TIM, PWM0_CH, &TIM_OC_Initstruct);
    
    PWM0_SET(fanset);

    //输入捕获GPIO与通道配置
    GPIO_InitTypeDef.Pin = SPD0_PIN;
    GPIO_InitTypeDef.Mode = LL_GPIO_MODE_ALTERNATE;
    GPIO_InitTypeDef.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
    GPIO_InitTypeDef.Speed = LL_GPIO_SPEED_FREQ_HIGH;
    GPIO_InitTypeDef.Alternate = SPD0_AF;
    GPIO_InitTypeDef.Pull = LL_GPIO_PULL_NO;
    LL_GPIO_Init(SPD0_PORT, &GPIO_InitTypeDef);

    GPIO_InitTypeDef.Pin = SPD1_PIN;
    GPIO_InitTypeDef.Mode = LL_GPIO_MODE_ALTERNATE;
    GPIO_InitTypeDef.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
    GPIO_InitTypeDef.Speed = LL_GPIO_SPEED_FREQ_HIGH;
    GPIO_InitTypeDef.Alternate = SPD1_AF;
    GPIO_InitTypeDef.Pull = LL_GPIO_PULL_NO;
    LL_GPIO_Init(SPD1_PORT, &GPIO_InitTypeDef);
    
    LL_TIM_IC_InitTypeDef TIM_IC_Initstruct;
    
    TIM_IC_Initstruct.ICPolarity = LL_TIM_IC_POLARITY_FALLING;
    TIM_IC_Initstruct.ICActiveInput = LL_TIM_ACTIVEINPUT_DIRECTTI;
    TIM_IC_Initstruct.ICPrescaler = LL_TIM_ICPSC_DIV1;
    TIM_IC_Initstruct.ICFilter = LL_TIM_IC_FILTER_FDIV1;
    LL_TIM_IC_Init(SPD0_TIM, SPD0_CH, &TIM_IC_Initstruct);
    LL_TIM_IC_Init(SPD1_TIM,SPD1_CH, &TIM_IC_Initstruct);
    //使能捕获中断
    LL_TIM_EnableIT_CC3(SPD0_TIM);
    LL_TIM_EnableIT_CC4(SPD1_TIM);
    NVIC_SetPriority(SPD_ICP_IRQn,1);
}

//SPD0、SPD1脉宽计算结果
uint32_t pw_spd0,pw_spd1;
//SPD0、SPD1脉宽计算变量
uint32_t cnt_spd0,cnt_spd1;
uint16_t lasticp_spd0,lasticp_spd1;

//一个周期：2/3us
//rpm = f*30
void SPD_UPD_IRQHandler(void){
    LL_TIM_ClearFlag_UPDATE(SPD0_TIM);
    cnt_spd0 += 65536U;
    cnt_spd1 += 65536U;
}

void SPD_ICP_IRQHandler(void){
    if(LL_TIM_IsActiveFlag_CC3(SPD0_TIM)){
        LL_TIM_ClearFlag_CC3(SPD0_TIM);
        pw_spd0 = cnt_spd0 + LL_TIM_IC_GetCaptureCH3(SPD0_TIM) - lasticp_spd0;
        cnt_spd0 = 0;
        lasticp_spd0 = LL_TIM_IC_GetCaptureCH3(SPD0_TIM);
    }
    if(LL_TIM_IsActiveFlag_CC4(SPD1_TIM)){
        LL_TIM_ClearFlag_CC4(SPD1_TIM);
        pw_spd1 = cnt_spd1 + LL_TIM_IC_GetCaptureCH4(SPD1_TIM) - lasticp_spd1;
        cnt_spd1 = 0;
        lasticp_spd1 = LL_TIM_IC_GetCaptureCH4(SPD1_TIM);
    }
}

void FAN_TimerOn(void){//打开风扇相关定时器操作
    //开输出口
    LL_GPIO_SetPinPull(SPD0_PORT, SPD0_PIN, LL_GPIO_PULL_UP);
    LL_GPIO_SetPinPull(SPD1_PORT, SPD1_PIN, LL_GPIO_PULL_UP);
    LL_GPIO_SetPinMode(PWM0_PORT, PWM0_PIN, LL_GPIO_MODE_ALTERNATE);
    LL_TIM_OC_SetMode(PWM0_TIM, PWM0_CH, LL_TIM_OCMODE_PWM1);
    //开定时器
    LL_TIM_EnableCounter(PWM0_TIM);
    LL_TIM_EnableAllOutputs(PWM0_TIM);
    LL_TIM_EnableCounter(SPD0_TIM);
    //开中断
    NVIC_EnableIRQ(SPD_ICP_IRQn);
    NVIC_EnableIRQ(SPD_UPD_IRQn);
}

void FAN_TimerOff(void){//关闭风扇相关定时器操作
    //关中断
    NVIC_DisableIRQ(SPD_ICP_IRQn);
    NVIC_DisableIRQ(SPD_UPD_IRQn);
    //关输出口
    LL_TIM_OC_SetMode(PWM0_TIM, PWM0_CH, LL_TIM_OCMODE_FORCED_INACTIVE);
    LL_GPIO_SetPinMode(PWM0_PORT, PWM0_PIN, LL_GPIO_MODE_INPUT);
    LL_GPIO_SetPinPull(SPD0_PORT, SPD0_PIN, LL_GPIO_PULL_NO);
    LL_GPIO_SetPinPull(SPD1_PORT, SPD1_PIN, LL_GPIO_PULL_NO);
    //关定时器
    LL_TIM_DisableCounter(PWM0_TIM);
    LL_TIM_DisableCounter(SPD0_TIM);
}