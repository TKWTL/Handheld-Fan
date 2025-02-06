#ifndef __PARAMETERS_H__
#define __PARAMETERS_H__

/*
系统参数文件
用于定义全局变量以便使用
需用volatile修饰，并赋予适当的初值
*/

#ifdef __cplusplus
extern "C" {
#endif
    
    #include <stdint.h>
    //需提交的设置变量    
    extern volatile uint16_t indensity;                                         //OLED亮度
    extern volatile uint16_t indensity_last;                                    //OLED亮度的旧值
    extern volatile uint16_t portc_discharge;                                   //C口放电功能
    extern volatile uint16_t portc_discharge_last;                              //C口放电功能的旧值
    extern volatile uint16_t portc_charge;                                      //C口充电功能
    extern volatile uint16_t portc_charge_last;                                 //C口充电功能的旧值
    extern volatile uint16_t protocol_scp_en;                                   //SCP快充协议使能
    extern volatile uint16_t protocol_scp_en_last;                              //SCP快充协议使能的旧值
    extern volatile uint16_t protocol_pd_en;                                    //PD快充协议使能
    extern volatile uint16_t protocol_pd_en_last;                               //PD快充协议使能的旧值
    extern volatile uint16_t protocol_pe_en;                                    //MTK PE快充协议使能
    extern volatile uint16_t protocol_pe_en_last;                               //MTK PE快充协议使能的旧值
    extern volatile uint16_t protocol_fcp_en;                                   //FCP快充协议使能
    extern volatile uint16_t protocol_fcp_en_last;                              //FCP快充协议使能的旧值
    extern volatile uint16_t protocol_afc_en;                                   //AFC快充协议使能
    extern volatile uint16_t protocol_afc_en_last;                              //AFC快充协议使能的旧值
    //复位标志变量
    extern volatile uint16_t system_reset;
    //无需提交的设置变量
    extern volatile uint16_t fanset;                                            //风扇转速
    extern volatile uint16_t sleepable;                                         //睡眠允许标志
    extern volatile uint16_t sleep_time;                                        //睡眠倒计时上限
    //非设置用变量
    extern volatile uint16_t sleep_cd;
    extern volatile uint16_t rpm0,rpm1;
    extern volatile uint8_t portc_is_on;                                        //C口开启标志
    extern volatile uint16_t fan_is_on;                                         //风扇开启标志
    extern volatile uint16_t fan_is_on_last;                                    //风扇开启标志的旧值
    extern volatile uint16_t vref_conv;                                         //Vref值缓存
    extern volatile uint16_t temp_conv;                                         //温度缓存，约等于25度
    extern volatile float vcc_voltage;                                          //芯片供电电压
    extern volatile float junction_temp;                                        //芯片结温
    
    #define VBAT_LOW    2.8f                                                    //低电检测标准电压
    
#ifdef __cplusplus
}
#endif
#endif