#ifndef __HARDWARE_CONFIG_H__
#define __HARDWARE_CONFIG_H__

/* Includes ------------------------------------------------------------------*/
#include "py32f0xx_ll_rcc.h"
#include "py32f0xx_ll_bus.h"
#include "py32f0xx_ll_system.h"
#include "py32f0xx_ll_cortex.h"
#include "py32f0xx_ll_utils.h"
#include "py32f0xx_ll_pwr.h"
#include "py32f0xx_ll_exti.h"
#include "py32f0xx_ll_adc.h"
#include "py32f0xx_ll_dma.h"
#include "py32f0xx_ll_gpio.h"
#include "py32f0xx_ll_i2c.h"
#include "py32f0xx_ll_usart.h"
#include "py32f0xx_ll_tim.h"


#define CoreClk     24000000U

//通信接口
#define SDA1_PIN LL_GPIO_PIN_0
#define SDA1_PORT GPIOF
#define SCL1_PIN LL_GPIO_PIN_1
#define SCL1_PORT GPIOF

#define DEBUG_USART_RX_GPIO_PORT                GPIOA
#define DEBUG_USART_RX_GPIO_CLK_ENABLE()        LL_IOP_GRP1_EnableClock(LL_IOP_GRP1_PERIPH_GPIOA)
#define DEBUG_USART_RX_PIN                      LL_GPIO_PIN_3
#define DEBUG_USART_RX_AF                       LL_GPIO_AF_1
#define DEBUG_USART_TX_GPIO_PORT                GPIOA
#define DEBUG_USART_TX_GPIO_CLK_ENABLE()        LL_IOP_GRP1_EnableClock(LL_IOP_GRP1_PERIPH_GPIOA)
#define DEBUG_USART_TX_PIN                      LL_GPIO_PIN_2
#define DEBUG_USART_TX_AF                       LL_GPIO_AF_1

//输入/EXTI
#define INT_PIN                                 LL_GPIO_PIN_5
#define INT_PORT                                GPIOA
#define INT_EXTI                                LL_EXTI_LINE_5
#define INT_EXTIPIN                             LL_EXTI_CONFIG_LINE5
#define INT_EXTIPORT                            LL_EXTI_CONFIG_PORTA
#define INT_IRQn                                EXTI4_15_IRQn

#define K_UP_PIN                                LL_GPIO_PIN_7
#define K_UP_PORT                               GPIOA
#define K_UP_EXTI                               LL_EXTI_LINE_7
#define K_UP_EXTIPIN                            LL_EXTI_CONFIG_LINE7
#define K_UP_EXTIPORT                           LL_EXTI_CONFIG_PORTA
#define K_UP_IRQn                               EXTI4_15_IRQn

#define K_DOWN_PIN                              LL_GPIO_PIN_2
#define K_DOWN_PORT                             GPIOB
#define K_DOWN_EXTI                             LL_EXTI_LINE_2
#define K_DOWN_EXTIPIN                          LL_EXTI_CONFIG_LINE2
#define K_DOWN_EXTIPORT                         LL_EXTI_CONFIG_PORTB
#define K_DOWN_IRQn                             EXTI2_3_IRQn

#define K_LEFT_PIN                              LL_GPIO_PIN_0
#define K_LEFT_PORT                             GPIOB
#define K_LEFT_EXTI                             LL_EXTI_LINE_0
#define K_LEFT_EXTIPIN                          LL_EXTI_CONFIG_LINE0
#define K_LEFT_EXTIPORT                         LL_EXTI_CONFIG_PORTB
#define K_LEFT_IRQn                             EXTI0_1_IRQn

#define K_RIGHT_PIN                             LL_GPIO_PIN_3
#define K_RIGHT_PORT                            GPIOB
#define K_RIGHT_EXTI                            LL_EXTI_LINE_3
#define K_RIGHT_EXTIPIN                         LL_EXTI_CONFIG_LINE3
#define K_RIGHT_EXTIPORT                        LL_EXTI_CONFIG_PORTB
#define K_RIGHT_IRQn                            EXTI2_3_IRQn

#define K_CLICK_PIN                             LL_GPIO_PIN_6//与PF4复用，初始化时关闭
#define K_CLICK_PORT                            GPIOB
#define K_CLICK_EXTI                            LL_EXTI_LINE_6
#define K_CLICK_EXTIPIN                         LL_EXTI_CONFIG_LINE6
#define K_CLICK_EXTIPORT                        LL_EXTI_CONFIG_PORTB
#define K_CLICK_IRQn                            EXTI4_15_IRQn

//输出/初始状态
#define CP_EN_PIN LL_GPIO_PIN_1
#define CP_EN_PORT GPIOB
#define CP_EN_INITSTA LL_GPIO_SetOutputPin

#define QC_EN_PIN LL_GPIO_PIN_6
#define QC_EN_PORT GPIOA
#define QC_EN_INITSTA LL_GPIO_ResetOutputPin


#define PWM0_TIM                                TIM14
#define PWM0_TIM_CLK                            LL_APB1_GRP2_PERIPH_TIM14
#define PWM0_CH                                 LL_TIM_CHANNEL_CH1
#define PWM0_PIN                                LL_GPIO_PIN_4
#define PWM0_PORT                               GPIOA
#define PWM0_AF                                 LL_GPIO_AF_4
#define PWM0_SET(val)                           LL_TIM_OC_SetCompareCH1(PWM0_TIM,val)

#define SPD0_TIM                                TIM1
#define SPD0_TIM_CLK                            LL_APB1_GRP2_PERIPH_TIM1
#define SPD0_CH                                 LL_TIM_CHANNEL_CH3
#define SPD0_PIN                                LL_GPIO_PIN_0
#define SPD0_PORT                               GPIOA
#define SPD0_AF                                 LL_GPIO_AF_13

#define SPD1_TIM                                TIM1
#define SPD1_TIM_CLK                            LL_APB1_GRP2_PERIPH_TIM1
#define SPD1_CH                                 LL_TIM_CHANNEL_CH4
#define SPD1_PIN                                LL_GPIO_PIN_1
#define SPD1_PORT                               GPIOA
#define SPD1_AF                                 LL_GPIO_AF_13

#define SPD_UPD_IRQn                            TIM1_BRK_UP_TRG_COM_IRQn
#define SPD_ICP_IRQn                            TIM1_CC_IRQn
#define SPD_UPD_IRQHandler                      TIM1_BRK_UP_TRG_COM_IRQHandler
#define SPD_ICP_IRQHandler                      TIM1_CC_IRQHandler

#endif