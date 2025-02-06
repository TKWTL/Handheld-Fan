#include "parameters.h"
volatile uint16_t fanset = 512;                                                 //风扇转速

volatile uint16_t indensity = 63;                                               //OLED亮度
volatile uint16_t indensity_last = 63;                                          //OLED亮度的旧值
volatile uint16_t portc_discharge = 1;                                          //C口放电功能
volatile uint16_t portc_discharge_last = 1;                                     //C口放电功能的旧值
volatile uint16_t portc_charge = 1;                                             //C口充电功能
volatile uint16_t portc_charge_last = 1;                                        //C口充电功能的旧值
volatile uint16_t protocol_scp_en = 1;                                          //SCP快充协议使能
volatile uint16_t protocol_scp_en_last = 1;                                     //SCP快充协议使能的旧值
volatile uint16_t protocol_pd_en = 1;                                           //PD快充协议使能
volatile uint16_t protocol_pd_en_last = 1;                                      //PD快充协议使能的旧值
volatile uint16_t protocol_pe_en = 1;                                           //MTK PE快充协议使能
volatile uint16_t protocol_pe_en_last = 1;                                      //MTK PE快充协议使能的旧值
volatile uint16_t protocol_fcp_en = 1;                                          //FCP快充协议使能
volatile uint16_t protocol_fcp_en_last = 1;                                     //FCP快充协议使能的旧值
volatile uint16_t protocol_afc_en = 1;                                          //AFC快充协议使能
volatile uint16_t protocol_afc_en_last = 1;                                     //AFC快充协议使能的旧值

volatile uint16_t system_reset = 0;                                             //系统复位变量

volatile uint16_t sleepable = 1;                                                //可睡眠标志，0无视具体休眠设定禁止休眠
volatile uint16_t sleep_time = 2000;                                            //睡眠时间设定值，默认1000ticks = 10秒

/*****************************用户不可设定区***********************************/
volatile uint16_t sleep_cd = 2000;                                              //睡眠计数器
volatile uint8_t portc_is_on = 0;                                               //C口开启标志
volatile uint16_t fan_is_on = 0;                                                //风扇开启标志
volatile uint16_t fan_is_on_last = 0;                                           //风扇开启标志旧值
volatile uint16_t vref_conv = 1500;                                             //Vref值缓存
volatile uint16_t temp_conv = 925;                                              //温度缓存，约等于25度
volatile float vcc_voltage;                                                     //芯片供电电压
volatile float junction_temp;                                                   //芯片结温

volatile uint16_t rpm0,rpm1;