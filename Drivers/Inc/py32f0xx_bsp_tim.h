#ifndef __PY32F0XX_BSP_TIM_H__
#define __PY32F0XX_BSP_TIM_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "defines.h"
#include "coroOS.h"

void BSP_TIM_config(void);
void BSP_PWMChannelConfig(void);

void FAN_TimerOn(void);//打开风扇相关定时器操作
void FAN_TimerOff(void);//关闭风扇相关定时器操作
    
#ifdef __cplusplus
}
#endif

#endif
