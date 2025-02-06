/*
 *主要程序文件
 *BUG：在风扇开启的情况下由于巨大电磁干扰，读取的各种数据会有随机跳动现象
 *硬件信息：https://oshwhub.com/jeremy_li/shou-chi-pu-li-shan
 *Version: 1.0 Releasse      TKWTL 2025/02/02
 */
#include "main.h"
#include "menu.h"

THRD_DECLARE(thread_draw){                                                      //屏幕绘制、刷新线程
    THRD_BEGIN;
    THRD_SPAWN_NOARG(SSD1306_Init);
    while(1){
        SSD1306_Fill(0);
        THRD_YIELD;//暂时出让CPU
        current_table->draw();
        THRD_SPAWN_NOARG(SSD1306_UpdateScreen);
        THRD_YIELD;//暂时出让CPU
    }
    THRD_END;
}

THRD_DECLARE(thread_update){                                                    //数据更新线程
    THRD_BEGIN;
    extern uint32_t pw_spd0,pw_spd1;//外部文件提供的测量脉宽
    static uint16_t lastrpm0,lastrpm1;//滤波变量
    THRD_SPAWN_NOARG(SW6208_Init);
    while(1){ 
        THRD_SPAWN_NOARG(SW6208_ADCLoad);
        THRD_DELAY(32);
        THRD_SPAWN_NOARG(SW6208_PortLoad);
        THRD_DELAY(32);
        THRD_SPAWN_NOARG(SW6208_CapacityLoad);
        THRD_DELAY(32);
        THRD_SPAWN_NOARG(SW6208_StatusLoad);
        THRD_DELAY(32);
        THRD_SPAWN_NOARG(SW6208_ADCLoad);
        if(SW6208_IsInitialized() == 0 ||
            SW6208_ReadVBUS() > 16.0f ||
            SW6208_ReadICharge() > 4000.0f ||
            SW6208_ReadIDischarge() > 4000.0f
        ) THRD_SPAWN_NOARG(SW6208_Init);        
        
        #if 0 //信息输出，可以禁用
        uprintf("Sleep Countdown:%d\n",sleep_cd);
        uprintf("PWSPD0:%ld\tPWSPD1:%ld\n",pw_spd0,pw_spd1);
        uprintf("Capacity:%d%%\n",SW6208_ReadCapacity());
        
        if(SW6208_IsCharging()) uprintf("Charging.\n");
        if(SW6208_IsDischarging()) uprintf("Discharging.\n");
        
        if(SW6208_IsLPortON()) uprintf("Port L is ON.\n");
        if(SW6208_IsBPortON()) uprintf("Port B is ON.\n");
        if(SW6208_IsCPortON()) uprintf("Port C is ON.\n");
        if(SW6208_IsA2PortON()) uprintf("Port A2 is ON.\n");
        if(SW6208_IsA1PortON()) uprintf("Port A1 is ON.\n");
        
        uprintf("Port Voltage:%.3fV\t\t",SW6208_ReadVBUS());
        uprintf("Battery Voltage:%.3fV\n",SW6208_ReadVBAT());
        uprintf("Charge Current:%.1fmA\t\t",SW6208_ReadICharge());
        uprintf("Discharge Current:%.1fmA\n",SW6208_ReadIDischarge());
        uprintf("Junction Temprature:%.1f'C\n\n",junction_temp);
        #endif
        THRD_DELAY(32);
        //转速计算
        if(fan_is_on){
            if(pw_spd0 > 200U){//幅值滤波
                rpm0 = (22500000U/pw_spd0 + lastrpm0)/ 2;//平均滤波
                lastrpm0 = rpm0;
            }
            if(pw_spd1 > 300U){
                rpm1 = (22500000U/pw_spd1 + lastrpm1)/ 2;
                lastrpm1 = rpm1;
            }
        THRD_DELAY(32);
        }
    }    
    THRD_END;
}

THRD_DECLARE(thread_keylogic){                                                  //按键逻辑处理线程，兼作I2C看门狗
    static uint16_t wdt_cnt;
    static uint32_t lastmillis;
    THRD_BEGIN; 
    Key_Init();
    while(1){
        if(lastmillis != millis){
            //每tick触发一次
            lastmillis = millis;
            //睡眠倒计时条件判断
            if(sleepable && 
                current_table->sleepable && 
                portc_is_on == 0 &&
                fan_is_on == 0 &&
                sleep_cd
                )sleep_cd--;
            //按键逻辑
            Key_DebounceService_10ms();
            Key_Scand();
            current_table->key();
            //I2C看门狗
            wdt_cnt++;
            if(i2c_mutex.count) wdt_cnt = 0;
            else if(wdt_cnt > 100 && GetI2CStatus() == I2C_IDLE){
                PT_SEM_SIGNAL(pt, &i2c_mutex);
            }
        }
        THRD_YIELD;
    }
    THRD_END;
}

THRD_DECLARE(thread_parameters_submit){                                         //设置项提交线程与中断读取
    extern uint8_t f_int;//外部中断标志位
    THRD_BEGIN; 
    while(1){
        if(system_reset){                                                       //提交复位
            uprintf("\n\nResetting......\n\n");
            LL_mDelay(10);
            NVIC_SystemReset();
        }
        if(indensity_last != indensity){                                        //提交亮度
            THRD_SPAWN_ARGS(SSD1306_SetIndensity,indensity);
            indensity_last = indensity;
        }
        if(portc_discharge_last != portc_discharge){                            //提交C口放电
            if(portc_discharge == 0 && portc_charge == 0) portc_discharge = 1;  //不允许同时关闭充放电
            else THRD_SPAWN_ARGS(SW6208_PortCRoleSet,portc_discharge? 0:1);     //设置C口角色，0：DRP，1：SINK，2：SOURCE);
            portc_discharge_last = portc_discharge;
        }
        if(portc_charge_last != portc_charge){                                  //提交C口充电
            if(portc_discharge == 0 && portc_charge == 0) portc_charge = 1;     //不允许同时关闭充放电
            else THRD_SPAWN_ARGS(SW6208_PortCRoleSet,portc_charge? 0:2);        //设置C口角色，0：DRP，1：SINK，2：SOURCE);
            portc_charge_last = portc_charge;
        }
        if(protocol_pd_en_last != protocol_pd_en){                              //提交PD协议使能
            THRD_SPAWN_ARGS(SW6208_PDEnableSet,protocol_pd_en);
            protocol_pd_en_last = protocol_pd_en;
        }
        if(protocol_scp_en_last != protocol_scp_en){                            //提交SCP协议使能
            THRD_SPAWN_ARGS(SW6208_SCPEnableSet,protocol_scp_en);
            protocol_scp_en_last = protocol_scp_en;
        }
        if(protocol_fcp_en_last != protocol_fcp_en){                            //提交FCP协议使能
            THRD_SPAWN_ARGS(SW6208_FCPEnableSet,protocol_fcp_en);
            protocol_fcp_en_last = protocol_fcp_en;
        }
        if(protocol_afc_en_last != protocol_afc_en){                            //提交AFC协议使能
            THRD_SPAWN_ARGS(SW6208_AFCEnableSet,protocol_afc_en);
            protocol_afc_en_last = protocol_afc_en;
        }
        if(protocol_pe_en_last != protocol_pe_en){                              //提交MTK PE协议使能
            THRD_SPAWN_ARGS(SW6208_PEEnableSet,protocol_pe_en);
            protocol_pe_en_last = protocol_pe_en;
        }
        
        if(f_int){
            uprintf("\nINT Event Occured!!\n\n");
            //THRD_SPAWN_NOARG(SW6208_PortLoad);
            //THRD_SPAWN_NOARG(SW6208_StatusLoad);
            //if(SW6208_IsFullCharge()) uprintf("\nFull Charged.\n");
            //if(SW6208_IsNTCOverTemp()) uprintf("\nNTC OverTemprature!!!\n\n");
            //THRD_SPAWN_ARGS(SW6208_ByteWrite,SW6208_STRG_KEY, 0xFF);
            //THRD_SPAWN_ARGS(SW6208_ByteWrite,SW6208_STRG_PORTRMV, 0xFF);
            //THRD_SPAWN_ARGS(SW6208_ByteWrite,SW6208_STRG_PORTINS, 0xFF);
            //THRD_SPAWN_ARGS(SW6208_ByteWrite,SW6208_STRG_BAT, 0xFF);
            f_int = 0;
        }
        
        if(SW6208_IsCPortON()) portc_is_on = 1;
        else portc_is_on = 0;
        
        THRD_YIELD;
    }
    THRD_END;
}

THRD_DECLARE(thread_fan_fsm){                                                   //风扇操作状态机
    THRD_BEGIN; 
    while(1){
        if(fan_is_on){//风扇打开了
            if(fan_is_on_last != fan_is_on){//需要切换状态
                LL_GPIO_ResetOutputPin(QC_EN_PORT, QC_EN_PIN);
                THRD_SPAWN_NOARG(SW6208_PortA1Insert);
                FAN_TimerOn();
                THRD_DELAY(270);//延时等待进入快充模式
                LL_GPIO_SetOutputPin(QC_EN_PORT, QC_EN_PIN);
                THRD_UNTIL(SW6208_ReadVBUS() > 11.0f);
                fan_is_on_last = fan_is_on;
            }
            else{//检查一下状态
                if(portc_is_on || SW6208_ReadCapacity() == 0 || SW6208_ReadVBUS() < 11.0f){
                    fan_is_on = 0;
                    current_table = menu_table + 6;//显示警告画面
                }
                if(SW6208_IsA2PortON()) THRD_SPAWN_NOARG(SW6208_PortA2Remove);
            }
        }
        else{
            if(fan_is_on_last != fan_is_on){//需要切换状态
                LL_GPIO_ResetOutputPin(QC_EN_PORT, QC_EN_PIN);
                FAN_TimerOff();
                THRD_DELAY(50);
                THRD_SPAWN_NOARG(SW6208_PortA1Remove);
                THRD_UNTIL(SW6208_ReadVBUS() < 4.6f);
                fan_is_on_last = fan_is_on;
            }
            else{//检查一下状态
                if(SW6208_IsA1PortON()){
                    FAN_TimerOff();
                    LL_GPIO_ResetOutputPin(QC_EN_PORT, QC_EN_PIN);
                    THRD_SPAWN_NOARG(SW6208_PortA1Remove);
                }
            }
        }
        THRD_YIELD;
    }
    THRD_END;    
}

THRD_DECLARE(thread_adc){                                                       //ADC操作线程
    static float k;
    THRD_BEGIN;
    k = (float)(TEMPSENSOR_CAL2_TEMP - TEMPSENSOR_CAL1_TEMP)/(float)(*TEMPSENSOR_CAL2_ADDR- *TEMPSENSOR_CAL1_ADDR);
    while(1){
        LL_ADC_REG_SetSequencerChannels(ADC1, LL_ADC_CHANNEL_12);
        LL_ADC_Enable(ADC1);//单次转换后自动关闭了ADC！
        THRD_DELAY(1);//等待ADC启动
        LL_ADC_REG_StartConversion(ADC1);
        THRD_DELAY(1);//硬等等待转换结束
        vref_conv = LL_ADC_REG_ReadConversionData12(ADC1);
        vcc_voltage = 4914.0/vref_conv;//计算VCC
        THRD_DELAY(98);
        
        LL_ADC_REG_SetSequencerChannels(ADC1, LL_ADC_CHANNEL_11);
        LL_ADC_Enable(ADC1);
        THRD_DELAY(1);
        LL_ADC_REG_StartConversion(ADC1);
        THRD_DELAY(1);
        temp_conv = LL_ADC_REG_ReadConversionData12(ADC1);
        junction_temp = k* (vcc_voltage/ 3.3f* temp_conv - *TEMPSENSOR_CAL1_ADDR)+ TEMPSENSOR_CAL1_TEMP;//计算结温        
        THRD_DELAY(98);//共延时1s
    }
    THRD_END;
}

THRD_DECLARE(thread_sleep){                                                     //低功耗休眠线程，放在线程函数注册表的末尾
    THRD_BEGIN;
    while(1){        
        if(sleep_cd == 0 && fan_is_on_last == 0){                               //满足休眠条件：CD归零，风扇已关
            //睡前准备
            THRD_SPAWN_NOARG(SSD1306_OFF);
            while(LL_USART_IsEnabledIT_TXE(DEBUG_USART));
            LL_ADC_SetCommonPathInternalCh(__LL_ADC_COMMON_INSTANCE(ADC1), LL_ADC_PATH_INTERNAL_NONE);
            LL_ADC_REG_StopConversion(ADC1);
            LL_mDelay(1);
            LL_ADC_Disable(ADC1);
            LL_GPIO_ResetOutputPin(CP_EN_PORT, CP_EN_PIN);
            current_table = menu_table;
            while(KEY_CLICK_GetIO());                                           //中键抬起前卡住进程
            LL_mDelay(1);
            LL_SYSTICK_DisableIT();
            LL_LPM_EnableDeepSleep();
            //正式休眠
            __WFI();
            //睡醒了
            LL_SYSTICK_EnableIT();
            LL_LPM_EnableSleep();
            LL_ADC_SetCommonPathInternalCh(__LL_ADC_COMMON_INSTANCE(ADC1), LL_ADC_PATH_INTERNAL_VREFINT|LL_ADC_PATH_INTERNAL_TEMPSENSOR);
            LL_GPIO_SetOutputPin(CP_EN_PORT, CP_EN_PIN);
            sleep_cd = sleep_time;
            THRD_SPAWN_NOARG(SSD1306_ON);
        }
        else __WFI();                                                           //不满足休眠条件
        THRD_YIELD;
    }
    THRD_END;
}

char (*threads[])(struct pt *pt) = {                                            //线程函数指针数组，在这里注册要运行的线程函数名字
    thread_adc,
    thread_keylogic,
    thread_draw,
    thread_parameters_submit,
    thread_update,
    thread_fan_fsm,
    thread_sleep
};
uint8_t thread_num = sizeof(threads)/sizeof(char(*)(struct pt *pt));            //线程数目指示


int main(void){
    SysInit();
    OS_INIT(threads);
    uprintf("\nHandheld Fan   V1.0\n");
    uprintf("Powered by PY32 & SW6208\n");
    uprintf("by TKWTL %s\n\n",__DATE__);
    
    while(1){
        OS_RUN(threads);
    }
}


/**
  * @brief  This function is executed in case of error occurrence.
  * @param  None
  * @retval None
  */
void APP_ErrorHandler(void)
{
    /* Infinite loop */
    while (1)
    {
    }
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* User can add his own implementation to report the file name and line number,
     for example: printf("Wrong parameters value: file %s on line %d\r\n", file, line)  */
  /* Infinite loop */
  while (1)
  {
  }
}
#endif /* USE_FULL_ASSERT */
