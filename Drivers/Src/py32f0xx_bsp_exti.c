#include "py32f0xx_bsp_exti.h"

void BSP_EXTI_Config()
{
    LL_EXTI_InitTypeDef EXTI_InitStruct;
    
    EXTI_InitStruct.Line = INT_EXTI;
    EXTI_InitStruct.LineCommand = ENABLE;
    EXTI_InitStruct.Mode = LL_EXTI_MODE_IT;
    EXTI_InitStruct.Trigger = LL_EXTI_TRIGGER_FALLING;
    LL_EXTI_Init(&EXTI_InitStruct);
    LL_EXTI_SetEXTISource(INT_EXTIPORT,INT_EXTIPIN);
    
    EXTI_InitStruct.Line = K_UP_EXTI;
    EXTI_InitStruct.LineCommand = ENABLE;
    EXTI_InitStruct.Mode = LL_EXTI_MODE_IT;
    EXTI_InitStruct.Trigger = LL_EXTI_TRIGGER_RISING;
    LL_EXTI_Init(&EXTI_InitStruct);
    LL_EXTI_SetEXTISource(K_UP_EXTIPORT,K_UP_EXTIPIN);
    
    EXTI_InitStruct.Line = K_DOWN_EXTI;
    EXTI_InitStruct.LineCommand = ENABLE;
    EXTI_InitStruct.Mode = LL_EXTI_MODE_IT;
    EXTI_InitStruct.Trigger = LL_EXTI_TRIGGER_RISING;
    LL_EXTI_Init(&EXTI_InitStruct);
    LL_EXTI_SetEXTISource(K_DOWN_EXTIPORT,K_DOWN_EXTIPIN);
    
    EXTI_InitStruct.Line = K_LEFT_EXTI;
    EXTI_InitStruct.LineCommand = ENABLE;
    EXTI_InitStruct.Mode = LL_EXTI_MODE_IT;
    EXTI_InitStruct.Trigger = LL_EXTI_TRIGGER_RISING;
    LL_EXTI_Init(&EXTI_InitStruct);
    LL_EXTI_SetEXTISource(K_LEFT_EXTIPORT,K_LEFT_EXTIPIN);
    
    EXTI_InitStruct.Line = K_RIGHT_EXTI;
    EXTI_InitStruct.LineCommand = ENABLE;
    EXTI_InitStruct.Mode = LL_EXTI_MODE_IT;
    EXTI_InitStruct.Trigger = LL_EXTI_TRIGGER_RISING;
    LL_EXTI_Init(&EXTI_InitStruct);
    LL_EXTI_SetEXTISource(K_RIGHT_EXTIPORT,K_RIGHT_EXTIPIN);
    
    EXTI_InitStruct.Line = K_CLICK_EXTI;
    EXTI_InitStruct.LineCommand = ENABLE;
    EXTI_InitStruct.Mode = LL_EXTI_MODE_IT;
    EXTI_InitStruct.Trigger = LL_EXTI_TRIGGER_RISING;
    LL_EXTI_Init(&EXTI_InitStruct);
    LL_EXTI_SetEXTISource(K_CLICK_EXTIPORT,K_CLICK_EXTIPIN);
    
    NVIC_SetPriority(K_LEFT_IRQn, 1);
    NVIC_EnableIRQ(K_LEFT_IRQn);
    NVIC_SetPriority(K_RIGHT_IRQn, 1);
    NVIC_EnableIRQ(K_RIGHT_IRQn);
    NVIC_SetPriority(K_CLICK_IRQn, 1);
    NVIC_EnableIRQ(K_CLICK_IRQn);
 /**
   * Enable interrupt:
   * - EXTI0_1_IRQn for PA/PB/PC[0,1]
   * - EXTI2_3_IRQn for PA/PB/PC[2,3]
   * - EXTI4_15_IRQn for PA/PB/PC[4,15]
  */
}


void EXTI0_1_IRQHandler(void){
    if(LL_EXTI_IsActiveFlag(K_LEFT_EXTI)){
        LL_EXTI_ClearFlag(K_LEFT_EXTI);
    }
    sleep_cd = sleep_time;
}

void EXTI2_3_IRQHandler(void){
    if(LL_EXTI_IsActiveFlag(K_DOWN_EXTI)){
        LL_EXTI_ClearFlag(K_DOWN_EXTI);
    }
    if(LL_EXTI_IsActiveFlag(K_RIGHT_EXTI)){
        LL_EXTI_ClearFlag(K_RIGHT_EXTI);
    }
    sleep_cd = sleep_time;
}

uint8_t f_int;
void EXTI4_15_IRQHandler(void){
    if(LL_EXTI_IsActiveFlag(INT_EXTI)){
        LL_EXTI_ClearFlag(INT_EXTI);
        if(fan_is_on_last && fan_is_on == 0);
        else sleep_cd = sleep_time;
        f_int = 1;
    }
    if(LL_EXTI_IsActiveFlag(K_UP_EXTI)){
        LL_EXTI_ClearFlag(K_UP_EXTI);
        sleep_cd = sleep_time;
    }
    if(LL_EXTI_IsActiveFlag(K_CLICK_EXTI)){
        LL_EXTI_ClearFlag(K_CLICK_EXTI);
        sleep_cd = sleep_time;
    }
}
