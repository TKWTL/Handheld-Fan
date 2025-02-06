/*SW6208操作库 V0.1
/TKWTL 2024/08/24
*/
#ifndef __SW6208_H__
#define __SW6208_H__

#ifdef __cplusplus
extern C {
#endif
    
/******************************用户设置区开始**********************************/
/*包含自己的I2C驱动库*/
#include "main.h"

#define SW6208_USE_PROTOTHREAD//配置与Protothread库一同使用

//外部库给出的I2C读写函数
#ifdef SW6208_USE_PROTOTHREAD//允许挂起    
    #define SW6208_I2C_Transmit(addr,reg,pdata,len,pflag)   ASYNC_I2C_Transmit(addr,reg,pdata,len,0,pflag)
    #define SW6208_I2C_Receive(addr,reg,pdata,len,pflag)    ASYNC_I2C_Receive(addr,reg,pdata,len,0,pflag)    
#else//不允许挂起
    
        
#endif
/******************************用户设置区结束**********************************/
//操作语法宏，方便添加freeRTOS之类的支持
#ifdef SW6208_USE_PROTOTHREAD
    #define SW6208_RET          char
    #define SW6208_NOARG        struct pt *pt
    #define SW6208_ARGS(...)    struct pt *pt, __VA_ARGS__
    #define SW6208_EXEC(cond)   if(cond == 0) THRD_YIELD                        //反复执行某函数直到返回1
    #define SW6208_UNTIL(cond)  THRD_UNTIL(cond)                                //条件不满足时出让CPU
    #define SW6208_SPAWN(func,...)\
                                THRD_SPAWN_ARGS(func, __VA_ARGS__)              //调用子线程/函数语句
    #define SW6208_FUNC_BEGIN   THRD_BEGIN
    #define SW6208_FUNC_END     THRD_END
#else
    #define SW6208_RET          void
    #define SW6208_NOARG        void
    #define SW6208_ARGS(...)    __VA_ARGS__
    #define SW6208_EXEC(cond)   cond
    #define SW6208_UNTIL(cond)  while(cond == 0)
    #define SW6208_SPAWN(func,...)\
                                func(__VA_ARGS__)
    #define SW6208_FUNC_BEGIN   {}
    #define SW6208_FUNC_END     {}
#endif    
        
struct SW6208_StatusTypedef{    
    uint8_t flag;                                                               //标识传输完成与传输状态用变量
    uint8_t online;                                                             //SW6208连接成功，表现为有设备响应0x3C的地址
    uint8_t initialized;                                                        //SW6208已初始化，当SW6208失去连接时自动回到未初始化状态
    uint8_t sendbuf[3];                                                         //传输缓冲用变量
    
/***************************寄存器内存镜像声明*********************************/
    //ADC读取数据区
    uint16_t vbat;
    uint16_t vbus;
    uint16_t tchip;
    uint16_t vntc;
    uint16_t icharge;
    uint16_t idischg;
    
    uint8_t display;//0x06
    uint8_t keystat;//0x07
    uint8_t portrmv;//0x08
    uint8_t portins;//0x09
    uint8_t batstat;//0x0A
    uint8_t fault0;//0x0B
    uint8_t sysstat;//0x0C
    uint8_t qcstat;//0x0F
    uint8_t qcled;//0x1F
    uint8_t vccov;//0x21
    uint8_t tpcstat;//0x29
    uint8_t lustat;//0x2E
    uint8_t ntcstat;//0x48
    uint16_t capval;//0x73,0x74
    uint8_t capacity;//0x7E
};

//SW6208 I2C 地址，为0x3C左移一位，最低位为0
#ifndef SW6208_I2C_ADDR
#define SW6208_I2C_ADDR                 0x78
#endif

/**************************SW6208 寄存器地址定义*******************************/
//命名规则：固定前缀(SW6208)_状态(ST)/控制(CT)+寄存器(RG)_(功能描述)
#define SW6208_CTRG_KEY                 0x03U//按键控制
#define SW6208_CTRG_KEYEVT              0x04U//短按键触发
#define SW6208_STRG_DISPLAY             0x06U//灯显状态
#define SW6208_STRG_KEY                 0x07U//按键状态
#define SW6208_STRG_PORTRMV             0x08U//端口拔出状态指示
#define SW6208_STRG_PORTINS             0x09U//端口插入状态指示
#define SW6208_STRG_BAT                 0x0AU//电池事件指示
#define SW6208_STRG_FAULT0              0x0BU//系统异常指示0
#define SW6208_STRG_SYS                 0x0CU//系统状态指示
#define SW6208_STRG_QC                  0x0FU//快充协议指示
#define SW6208_CTRG_ADC_SET             0x12U//ADC通道选择
#define SW6208_STRG_ADCH                0x13U//ADC数据高8位（设置完ADC通道后可立即读取，也必须设置完再读取）
#define SW6208_STRG_ADCL                0x14U//ADC数据低4位
#define SW6208_CTRG_INOUT               0x18U//输入输出控制
#define SW6208_CTRG_PORTEVT             0x19U//端口事件触发
#define SW6208_CTRG_QC0                 0x1AU//快充配置0
#define SW6208_CTRG_QC1                 0x1BU//快充配置1
#define SW6208_CTRG_QC2                 0x1CU//快充配置2
#define SW6208_CTRG_QC3                 0x1DU//快充配置3
#define SW6208_CTRG_QC4                 0x1EU//快充配置4
#define SW6208_STRG_QCLED               0x1FU//快充指示灯状态
#define SW6208_CTRG_WLED                0x20U//WLED配置
#define SW6208_STRG_FAULT1              0x21U//系统异常指示1
#define SW6208_CTRG_PDRST               0x22U//PD Reset命令
#define SW6208_CTRG_TYPEC               0x28U//TypeC配置
#define SW6208_STRG_TYPEC               0x29U//TypeC指示
#define SW6208_CTRG_PD0                 0x2AU//PD配置0
#define SW6208_CTRG_PD1                 0x2BU//PD配置1
#define SW6208_CTRG_PD2                 0x2CU//PD配置2
#define SW6208_CTRG_PD3                 0x2DU//PD配置4
#define SW6208_CTRG_LUSET               0x2EU//小电流放电控制
#define SW6208_CTRG_PORT0               0x30U//插拔检测配置0
#define SW6208_CTRG_PORT1               0x31U//插拔检测配置1
#define SW6208_CTRG_WLSS                0x32U//无线充模式配置
#define SW6208_CTRG_LUEN                0x33U//小电流放电使能
#define SW6208_CTRG_BST0                0x40U//Boost配置0
#define SW6208_CTRG_BST1                0x41U//Boost配置1
#define SW6208_CTRG_CHG0                0x42U//充电配置0
#define SW6208_CTRG_CHG1                0x43U//充电配置1
#define SW6208_CTRG_CHG2                0x44U//充电配置2
#define SW6208_CTRG_CHG3                0x45U//充电配置3
#define SW6208_CTRG_CHG4                0x46U//充电配置4
#define SW6208_CTRG_NTC0                0x47U//NTC配置0
#define SW6208_CTRG_NTC1                0x48U//NTC配置1
#define SW6208_CTRG_TEMP                0x49U//温度配置
#define SW6208_STRG_REV                 0x57U//芯片版本
#define SW6208_STRG_CAP0                0x72U//最终电量
#define SW6208_STRG_CAPL                0x73U//电池容量低8位
#define SW6208_STRG_CAPH                0x74U//电池容量高4位
#define SW6208_STRG_CAP1                0x7EU//最终处理电量（推荐使用该电量）

/******************************寄存器位定义************************************/
//0x03  SW6208_CTRG_KEY                 按键控制
#define SW6208_KEY_DC_LU_WLED           0xC0U//双击时优先触发小电流模式
#define SW6208_KEY_DC_WLED              0x80U//双击时打开WLED
#define SW6208_KEY_DC_LU                0x40U//双击时打开小电流模式
#define SW6208_KEY_DC_NOBST             0x00U//双击时关闭boost
#define SW6208_KEY_LP_WLED              0x30U//长按时打开WLED
#define SW6208_KEY_LP_LU                0x20U//长按时打开小电流模式
#define SW6208_KEY_LP_NOBST             0x10U//长按时关闭boost
#define SW6208_KEY_LP_LU_WLED           0x00U//长按时优先触发小电流模式
#define SW6208_KEY_SC_DISPLAY           0x0CU//短按键仅显示电量
#define SW6208_KEY_SC_A1A2              0x08U//短按键打开A1/A2口
#define SW6208_KEY_SC_A2                0x04U//短按键打开A2口
#define SW6208_KEY_SC_A1                0x00U//短按键打开A1口
#define SW6208_KEY_SC_500M              0x02U//大于32ms小于500ms的低电平识别为短按键
#define SW6208_KEY_SC_300M              0x00U//大于32ms小于300ms的低电平识别为短按键
#define SW6208_KEY_SC_RSP               0x01U//非无线充模式输出时按照短按键规则响应
#define SW6208_KEY_SC_NORSP             0x00U//非无线充模式输出时不响应按键

//0x04  SW6208_CTRG_KEYEVT              短按键触发
#define SW6208_KEYEVT_TRIG              0x01U//触发短按键事件（自动清零）

//0x06  SW6208_STRG_DISPLAY             灯显状态
#define SW6208_DISPLAY_IRQLOW           0x10U//IRQ脚为低电平
#define SW6208_DISPLAY_INITED           0x08U//用于指示芯片已初始化过
#define SW6208_DISPLAY_CHG              0x04U//充电状态
#define SW6208_DISPLAY_DIS              0x00U//放电状态
#define SW6208_DISPLAY_LEDON            0x02U//LED打开

//0x07  SW6208_STRG_KEY                 按键状态
#define SW6208_KEY_SC                   0x04U//短按键事件指示（写1清零）
#define SW6208_KEY_DC                   0x02U//双击事件指示（写1清零）
#define SW6208_KEY_LP                   0x01U//长按事件指示（写1清零）

//0x08  SW6208_STRG_PORTRMV             端口拔出状态指示
#define SW6208_PORTRMV_CSRC             0x20U//C口source设备拔出指示（写1清零）
#define SW6208_PORTRMV_L                0x10U//L口拔出指示（写1清零）
#define SW6208_PORTRMV_B                0x08U//B口拔出指示（写1清零）
#define SW6208_PORTRMV_CSNK             0x04U//C口sink设备拔出指示（写1清零）
#define SW6208_PORTRMV_A2               0x02U//A2口拔出指示（写1清零）
#define SW6208_PORTRMV_A1               0x01U//A1口拔出指示（写1清零）

//0x09  SW6208_STRG_PORTINS             端口插入状态指示
#define SW6208_PORTINS_CSRC             0x20U//C口source设备插入指示（写1清零）
#define SW6208_PORTINS_L                0x10U//L口插入指示（写1清零）
#define SW6208_PORTINS_B                0x08U//B口插入指示（写1清零）
#define SW6208_PORTINS_CSNK             0x04U//C口sink设备插入指示（写1清零）
#define SW6208_PORTINS_A2               0x02U//A2口插入指示（写1清零）
#define SW6208_PORTINS_A1               0x01U//A1口插入指示（写1清零）

//0x0A  SW6208_STRG_BAT                 电池事件指示
#define SW6208_BAT_QC_OV                0x10U//电池快充时过压事件指示（写1清零）
#define SW6208_BAT_NTC_OT               0x08U//NTC过温事件指示（写1清零）
#define SW6208_BAT_CHG_OT               0x04U//充电超时事件指示（写1清零）
#define SW6208_BAT_OV                   0x02U//电池过压事件指示（写1清零）
#define SW6208_BAT_FULL                 0x01U//充满事件指示（写1清零）

//0x0B  SW6208_STRG_FAULT0              系统异常指示0
#define SW6208_FAULT0_VDDOV_CLR         0x40U//VDD过压事件清零位
#define SW6208_FAULT0_L_VBUSOV          0x20U//L口Vbus过压事件（写1清零）
#define SW6208_FAULT0_C_VBUSOV          0x10U//C口Vbus过压事件（写1清零）
#define SW6208_FAULT0_B_VBUSOV          0x08U//B口Vbus过压事件（写1清零）
#define SW6208_FAULT0_UVLO              0x04U//UVLO事件（写1清零）
#define SW6208_FAULT0_OT                0x02U//过温事件（写1清零）
#define SW6208_FAULT0_OC                0x01U//过流事件（写1清零）

//0x0C  SW6208_STRG_SYS                 系统状态指示
#define SW6208_SYS_CHG_ON               0x80U//充电打开
#define SW6208_SYS_DIS_ON               0x40U//放电打开
#define SW6208_SYS_L_ON                 0x10U//L口通路打开
#define SW6208_SYS_B_ON                 0x08U//B口通路打开
#define SW6208_SYS_C_ON                 0x04U//C口通路打开
#define SW6208_SYS_A2_ON                0x02U//A2口通路打开
#define SW6208_SYS_A1_ON                0x01U//A1口通路打开

//0x0F  SW6208_STRG_QC                  快充协议指示
#define SW6208_QC_PD3                   0x80U//PD协议版本3.0
#define SW6208_QC_PE_SNK                0x70U//PE 1.1 sink
#define SW6208_QC_SCP_SNK               0x60U//SCP sink
#define SW6208_QC_FCP_SNK               0x50U//FCP sink
#define SW6208_QC_AFC_SNK               0x40U//AFC sink
#define SW6208_QC_HV_SNK                0x30U//HV sink
#define SW6208_QC_PD_SNK                0x10U//PD sink
#define SW6208_QC_NOQC_SNK              0x00U//非快充
#define SW6208_QC_SCP_SRC               0x09U//SCP source
#define SW6208_QC_AFC_SRC               0x08U//AFC source
#define SW6208_QC_SFCP_SRC              0x07U//AFCP source
#define SW6208_QC_PE_SRC                0x06U//PE1.1/2.0 source
#define SW6208_QC_FCP_SRC               0x05U//FCP source
#define SW6208_QC_QC3_SRC               0x04U//QC3.0 source
#define SW6208_QC_QC2_SRC               0x03U//QC2.0 source
#define SW6208_QC_PPS_SRC               0x02U//PD PPD source
#define SW6208_QC_PD_SRC                0x01U//PD FIX source
#define SW6208_QC_NOQC_SRC              0x00U//非快充

//0x12  SW6208_CTRG_ADC_SET             ADC通道选择
#define SW6208_ADC_SET_IDIS             0x05U//放电电流
#define SW6208_ADC_SET_ICHG             0x04U//充电电流
#define SW6208_ADC_SET_VNTC             0x03U//NTC电压
#define SW6208_ADC_SET_TCHIP            0x02U//芯片温度
#define SW6208_ADC_SET_VBUS             0x01U//端口电压
#define SW6208_ADC_SET_VBAT             0x00U//电池电压

//0x18  SW6208_CTRG_INOUT               输入输出控制
#define SW6208_INOUT_OUT_OFF            0x10U//关闭所有输出口（自动清零）
#define SW6208_INOUT_CHG_OFF            0x01U//关闭Charger（不自动清零）

//0x19  SW6208_CTRG_PORTEVT             端口事件触发（自动清零）
#define SW6208_PORTEVT_CRMV             0x20U//触发TypeC 口Sink设备拔出事件
#define SW6208_PORTEVT_CINS             0x10U//触发TypeC 口Sink设备插入事件
#define SW6208_PORTEVT_A2RMV            0x08U//触发A2口设备拔出事件
#define SW6208_PORTEVT_A2INS            0x04U//触发A2口设备插入事件
#define SW6208_PORTEVT_A1RMV            0x02U//触发A1口设备拔出事件
#define SW6208_PORTEVT_A1INS            0x01U//触发A1口设备插入事件

//0x1A  SW6208_CTRG_QC0                 快充配置0
#define SW6208_QC0_MSK                  0x9EU//快充配置0寄存器有效位
#define SW6208_QC0_C_DMINS_DIS          0x80U//不使能C口DM插入检测
#define SW6208_QC0_C_DMINS_EN           0x00U//使能C口DM插入检测
#define SW6208_QC0_AFC_12V_EN           0x10U//使能AFC 12V挡位
#define SW6208_QC0_AFC_12V_DIS          0x00U//不使能AFC 12V挡位
#define SW6208_QC0_FCP_12V_EN           0x08U//使能FCP 12V挡位
#define SW6208_QC0_FCP_12V_DIS          0x00U//不使能FCP 12V挡位
#define SW6208_QC0_VIN_12V_DIS          0x04U//不支持12V输入电压
#define SW6208_QC0_VIN_12V_EN           0x00U//支持12V输入电压
#define SW6208_QC0_VOUT_9V              0x02U//输出最高支持9V
#define SW6208_QC0_VOUT_12V             0x00U//输出最高支持12V

//0x1B  SW6208_CTRG_QC1                 快充配置1
#define SW6208_QC1_A1_DIS               0x80U//不使能A1口输出快充
#define SW6208_QC1_A2_DIS               0x40U//不使能A2口输出快充
#define SW6208_QC1_CO_DIS               0x20U//不使能C口输出快充
#define SW6208_QC1_B_DIS                0x10U//不使能B口输入快充
#define SW6208_QC1_CI_DIS               0x08U//不使能C口输入快充
#define SW6208_QC1_L_DIS                0x04U//不使能L口输入快充
#define SW6208_QC1_B_HV_DIS             0x02U//不使能B口高压输入协议1
#define SW6208_QC1_C_HV_DIS             0x01U//不使能C口高压输入协议1

//0x1C  SW6208_CTRG_QC2                 快充配置2
#define SW6208_QC2_MSK                  0xBBU//快充配置2寄存器有效位
#define SW6208_QC2_PSDLOAD_EN           0x80U//准负载检测机制使能
#define SW6208_QC2_PSDLOAD_DIS          0x00U//准负载检测机制不使能
#define SW6208_QC2_PDSRC_DIS            0x20U//不使能PD Source
#define SW6208_QC2_PDSNK_DIS            0x10U//不使能PD Sink
#define SW6208_QC2_CNOLOAD_EN           0x08U//PD处于高压时，C口检测空载
#define SW6208_QC2_CNOLOAD_DIS          0x00U//PD处于高压时，C口不检测空载
#define SW6208_QC2_A_QCSRC_DIS          0x02U//不使能A1/A2口的QC Source
#define SW6208_QC2_A_QCSRC_EN           0x00U//使能A1/A2口的QC Source
#define SW6208_QC2_FCPSRC_DIS           0x01U//不使能FCP Source
#define SW6208_QC2_FCPSRC_EN            0x00U//使能FCP Source

//0x1D  SW6208_CTRG_QC3                 快充配置3
#define SW6208_QC3_FCP_SNK_DIS          0x80U//不使能FCP Sink
#define SW6208_QC3_PE_SRC_DIS           0x40U//不使能PE Source
#define SW6208_QC3_PE_SNK_DIS           0x20U//不使能PE Sink
#define SW6208_QC3_AFC_SRC_DIS          0x10U//不使能AFC Source
#define SW6208_QC3_AFC_SNK_DIS          0x08U//不使能AFC Sink
#define SW6208_QC3_SCP_SRC_DIS          0x04U//不使能SCP Source
#define SW6208_QC3_SCP_SNK_DIS          0x02U//不使能SCP  Sink
#define SW6208_QC3_SFCP_SRC_DIS         0x01U//不使能SFCP Source

//0x1E  SW6208_CTRG_QC4                 快充配置4
#define SW6208_QC4_MSK                  0x12U//快充配置4寄存器有效位
#define SW6208_QC4_CHGDIS_DIS           0x10U//不支持边冲边放，充电优先
#define SW6208_QC4_CHGDIS_EN            0x00U//支持边冲边放
#define SW6208_QC4_TYPEC_QCSRC_DIS      0x02U//C口不支持QC Source
#define SW6208_QC4_TYPEC_QCSRC_EN       0x00U//C口支持QC Source

//0x1F  SW6208_STRG_QCLED               快充指示灯状态
#define SW6208_QCLED_ON                 0x1FU//快充指示灯亮

//0x20  SW6208_CTRG_WLED                WLED配置
#define SW6208_WLED_MSK                 0x11U//WLED配置寄存器有效位
#define SW6208_WLED_REGSET_EN           0x10U//允许MCU配置WLED模式开关
#define SW6208_WLED_REGSET_DIS          0x00U//不允许MCU配置WLED模式开关
#define SW6208_WLED_EN                  0x01U//WLED模式使能（两个bit都为1后仍需按键来打开WLED）
#define SW6208_WLED_DIS                 0x00U//WLED模式不使能

//0x21  SW6208_STRG_FAULT1              系统异常指示1
#define SW6208_FAULT1_VDD_OV            0x20U//VDD过压

//0x22  SW6208_CTRG_PDRST               PD Reset命令
#define SW6208_PDRST_SENT               0x01U//发送PD HardReset命令

//0x28  SW6208_CTRG_TYPEC               TypeC配置
#define SW6208_TYPEC_MSK                0x0CU//TypeC配置寄存器有效位
#define SW6208_TYPEC_SRC                0x08U//TypeC仅作为Source
#define SW6208_TYPEC_SNK                0x04U//TypeC仅作为Sink
#define SW6208_TYPEC_DRP                0x00U//TypeC作为Strong DRP

//0x29  SW6208_STRG_TYPEC               TypeC指示
#define SW6208_TYPEC_NOATTACH1          0x03U//TypeC未连接
#define SW6208_TYPEC_AS_SRC             0x02U//TypeC作为Source
#define SW6208_TYPEC_AS_SNK             0x01U//TypeC作为Sink
#define SW6208_TYPEC_NOATTACH0          0x00U//TypeC未连接

//0x2A  SW6208_CTRG_PD0                 PD配置0
#define SW6208_PD0_MSK                  0x60U//PD配置0寄存器有效位
#define SW6208_PD0_PD20                 0x40U//PD 2.0
#define SW6208_PD0_PD30                 0x00U//PD 3.0
#define SW6208_PD0_9VPPS                0x20U//PD PPS最高支持9V
#define SW6208_PD0_12VPPS               0x00U//PD PPS最高支持12V

//0x2B  SW6208_CTRG_PD1                 PD配置1
#define SW6208_PD1_MSK                  0xB8U//PD配置1寄存器有效位
#define SW6208_PD1_FIX9V                0x80U//PD Fixed最大9V
#define SW6208_PD1_FIX12V               0x00U//PD Fixed最大12V
#define SW6208_PD1_NOPPS0               0x20U//不使能PPS0
#define SW6208_PD1_ENPPS0               0x00U//使能PPS0
#define SW6208_PD1_R5VPDO_EN            0x10U//设备请求5VPDO后重新广播一次5V/2APDO
#define SW6208_PD1_R5VPDO_DIS           0x00U//设备请求5VPDO后不重新广播一次5V/2APDO
#define SW6208_PD1_NOPPS1               0x08U//不使能PPS1
#define SW6208_PD1_ENPPS1               0x00U//使能PPS1

//0x2C  SW6208_CTRG_PD2                 PD配置2
#define SW6208_PD2_MSK                  0x3FU//PD配置2寄存器有效位
#define SW6208_PD2_5V_2A0               0x30U//5V Fixed 2A
#define SW6208_PD2_5V_2A5               0x20U//5V Fixed 2.5A
#define SW6208_PD2_5V_2A4               0x10U//5V Fixed 2.4A
#define SW6208_PD2_5V_3A0               0x00U//5V Fixed 3A
#define SW6208_PD2_9V_2A4               0x0CU//9V Fixed 2.4A
#define SW6208_PD2_9V_2A33              0x08U//9V Fixed 2.33A
#define SW6208_PD2_9V_2A2               0x04U//9V Fixed 2.2A
#define SW6208_PD2_9V_2A0               0x00U//9V Fixed 2.0A
#define SW6208_PD2_12V_1A75             0x03U//12V Fixed 1.75A
#define SW6208_PD2_12V_1A7              0x02U//12V Fixed 1.7A
#define SW6208_PD2_12V_1A6              0x01U//12V Fixed 1.6A
#define SW6208_PD2_12V_1A5              0x00U//12V Fixed 1.5A

//0x2D  SW6208_CTRG_PD3                 PD配置3
#define SW6208_PD3_MSK                  0x21U//PD配置3寄存器有效位
#define SW6208_PD3_MPPD_DIS             0x20U//多口时不广播5V PDO
#define SW6208_PD3_MPPD_EN              0x00U//多口时广播5V PDO
#define SW6208_PD3_S1V2_DIS             0x01U//不使能三星1.2V
#define SW6208_PD3_S1V2_EN              0x00U//使能三星1.2V

//0x2E  SW6208_CTRG_LUSET               小电流放电控制
#define SW6208_LUSET_INOUT              0x10U//进入/退出小电流放电模式，自动清零
#define SW6208_LUSET_ON                 0x01U//处于小电流放电模式

//0x30  SW6208_CTRG_PORT0               插拔检测配置0
#define SW6208_PORT0_SPNL_64S           0xC0U//单口轻载延迟开启64s
#define SW6208_PORT0_SPNL_16S           0x80U//单口轻载延迟开启16s
#define SW6208_PORT0_SPNL_8S            0x40U//单口轻载延迟开启8s
#define SW6208_PORT0_SPNL_32S           0x00U//单口轻载延迟开启32s
#define SW6208_PORT0_MPNL_64S           0x30U//多口轻载延迟开启64s
#define SW6208_PORT0_MPNL_16S           0x20U//多口轻载延迟开启16s
#define SW6208_PORT0_MPNL_8S            0x10U//多口轻载延迟开启8s
#define SW6208_PORT0_MPNL_32S           0x00U//多口轻载延迟开启32s
#define SW6208_PORT0_INL_115M           0x0EU//轻载检测电流115mA
#define SW6208_PORT0_INL_100M           0x0CU//轻载检测电流100mA
#define SW6208_PORT0_INL_85M            0x0AU//轻载检测电流85mA
#define SW6208_PORT0_INL_70M            0x08U//轻载检测电流70mA
#define SW6208_PORT0_INL_40M            0x06U//轻载检测电流40mA
#define SW6208_PORT0_INL_25M            0x04U//轻载检测电流25mA
#define SW6208_PORT0_INL_10M            0x02U//轻载检测电流10mA
#define SW6208_PORT0_INL_55M            0x00U//轻载检测电流55mA
#define SW6208_PORT0_HVINL_EN           0x01U//高压下减小轻载检测电流
#define SW6208_PORT0_HVINL_DIS          0x00U//高压下不减小轻载检测电流

//0x31  SW6208_CTRG_PORT1               插拔检测配置1
#define SW6208_PORT1_MSK                0x3DU//插拔检测配置1寄存器有效位
#define SW6208_PORT1_A_DM_DIS           0x20U//不使能A口dm插入检测
#define SW6208_PORT1_A_DM_EN            0x00U//使能A口dm插入检测
#define SW6208_PORT1_LCOMP_DIS          0x10U//不使能线阻补偿
#define SW6208_PORT1_LCOMP_EN           0x00U//使能线阻补偿
#define SW6208_PORT1_A1_DIS             0x08U//不使能A1口负载插入检测
#define SW6208_PORT1_A1_EN              0x00U//使能A1口负载插入检测
#define SW6208_PORT1_A2_DIS             0x04U//不使能A2口负载插入检测
#define SW6208_PORT1_A2_EN              0x00U//使能A2口负载插入检测
#define SW6208_PORT1_CNL_DIS            0x01U//不使能C口轻载检测
#define SW6208_PORT1_CNL_EN             0x00U//使能C口轻载检测

//0x32  SW6208_CTRG_WLSS                无线充模式配置
#define SW6208_WLSS_MSK                 0x7FU//无线充模式配置寄存器有效位
#define SW6208_WLSS_NL_240M             0x70U//无线充模式下，A2口空载检测电流240mA
#define SW6208_WLSS_NL_210M             0x60U//无线充模式下，A2口空载检测电流210mA
#define SW6208_WLSS_NL_180M             0x50U//无线充模式下，A2口空载检测电流180mA
#define SW6208_WLSS_NL_150M             0x40U//无线充模式下，A2口空载检测电流150mA
#define SW6208_WLSS_NL_90M              0x30U//无线充模式下，A2口空载检测电流90mA
#define SW6208_WLSS_NL_60M              0x20U//无线充模式下，A2口空载检测电流60mA
#define SW6208_WLSS_NL_30M              0x10U//无线充模式下，A2口空载检测电流30mA
#define SW6208_WLSS_NL_120M             0x00U//无线充模式下，A2口空载检测电流120mA
#define SW6208_WLSS_NL_64S              0x0CU//无线充模式下，空载延迟开启64s
#define SW6208_WLSS_NL_32S              0x08U//无线充模式下，空载延迟开启32s
#define SW6208_WLSS_NL_16S              0x04U//无线充模式下，空载延迟开启16s
#define SW6208_WLSS_NL_120S             0x00U//无线充模式下，空载延迟开启120s
#define SW6208_WLSS_EN                  0x03U//使能无线充模式
#define SW6208_WLSS_DIS                 0x02U//不使能无线充模式

//0x33  SW6208_CTRG_LUEN                小电流放电使能
#define SW6208_LUEN_MSK                 0x03U//小电流放电使能寄存器有效位
#define SW6208_LUEN_EN                  0x03U//小电流放电使能
#define SW6208_LUEN_DIS                 0x02U//小电流放电不使能

//0x40  SW6208_CTRG_BST0                Boost配置0
#define SW6208_BST0_UVLO_3V4            0xE0U//UVLO关机阈值3.4V
#define SW6208_BST0_UVLO_3V3            0xC0U//UVLO关机阈值3.3V
#define SW6208_BST0_UVLO_3V2            0xA0U//UVLO关机阈值3.2V
#define SW6208_BST0_UVLO_3V1            0x80U//UVLO关机阈值3.1V
#define SW6208_BST0_UVLO_3V0            0x60U//UVLO关机阈值3.0V
#define SW6208_BST0_UVLO_2V9            0x40U//UVLO关机阈值2.9V
#define SW6208_BST0_UVLO_2V7            0x20U//UVLO关机阈值2.7V
#define SW6208_BST0_UVLO_2V8            0x00U//UVLO关机阈值2.8V
#define SW6208_BST0_UVLOHYS_V7          0x18U//UVLO迟滞0.7V
#define SW6208_BST0_UVLOHYS_V6          0x10U//UVLO迟滞0.6V
#define SW6208_BST0_UVLOHYS_V4          0x08U//UVLO迟滞0.4V
#define SW6208_BST0_UVLOHYS_V5          0x00U//UVLO迟滞0.5V
#define SW6208_BST0_600K                0x06U//Boost频率600K
#define SW6208_BST0_500K                0x04U//Boost频率500K
#define SW6208_BST0_300K                0x02U//Boost频率300K
#define SW6208_BST0_400K                0x00U//Boost频率400K
#define SW6208_BST0_POMAX_21            0x01U//最大输出功率21W
#define SW6208_BST0_POMAX_18            0x00U//最大输出功率18W

//0x41  SW6208_CTRG_BST1                Boost配置1
#define SW6208_BST1_MSK                 0x1DU//Boost配置1寄存器有效位
#define SW6208_BST1_VOS_150M            0x18U//Vout偏移150mV
#define SW6208_BST1_VOS_50M             0x10U//Vout偏移50mV
#define SW6208_BST1_VOS_0M              0x08U//Vout偏移0mV
#define SW6208_BST1_VOS_100M            0x00U//Vout偏移100mV
#define SW6208_BST1_CC_15P              0x04U//Boost恒流环路门限15%
#define SW6208_BST1_CC_5P               0x00U//Boost恒流环路门限5%
#define SW6208_BST1_MP_4A2              0x01U//多口最大输出4.2A
#define SW6208_BST1_MP_3A               0x00U//多口最大输出3A

//0x42  SW6208_CTRG_CHG0                充电配置0
#define SW6208_CHG0_MSK                 0xFCU//充电配置0寄存器有效位
#define SW6208_CHG0_9V_2A3              0xE0U//9V输入恒流2.3A
#define SW6208_CHG0_9V_2A2              0xC0U//9V输入恒流2.2A
#define SW6208_CHG0_9V_2A1              0xA0U//9V输入恒流2.1A
#define SW6208_CHG0_9V_1A9              0x80U//9V输入恒流1.9A
#define SW6208_CHG0_9V_1A8              0x60U//9V输入恒流1.8A
#define SW6208_CHG0_9V_1A7              0x40U//9V输入恒流1.7A
#define SW6208_CHG0_9V_1A6              0x20U//9V输入恒流1.6A
#define SW6208_CHG0_9V_2A0              0x00U//9V输入恒流2.0A
#define SW6208_CHG0_12V_1A8             0x1CU//12V输入恒流1.8A
#define SW6208_CHG0_12V_1A7             0x18U//12V输入恒流1.7A
#define SW6208_CHG0_12V_1A6             0x14U//12V输入恒流1.6A
#define SW6208_CHG0_12V_1A4             0x10U//12V输入恒流1.4A
#define SW6208_CHG0_12V_1A3             0x0CU//12V输入恒流1.3A
#define SW6208_CHG0_12V_1A2             0x08U//12V输入恒流1.2A
#define SW6208_CHG0_12V_1A1             0x04U//12V输入恒流1.1A
#define SW6208_CHG0_12V_1A5             0x00U//12V输入恒流1.5A

//0x43  SW6208_CTRG_CHG1                充电配置1
#define SW6208_CHG1_MSK                 0xFEU//充电配置1寄存器有效位
#define SW6208_CHG1_C_3A2               0xF0U//C口5V输入限流3.2A
#define SW6208_CHG1_C_3A1               0xE0U//C口5V输入限流3.1A
#define SW6208_CHG1_C_3A0               0xD0U//C口5V输入限流3.0A
#define SW6208_CHG1_C_2A9               0xC0U//C口5V输入限流2.9A
#define SW6208_CHG1_C_2A8               0xB0U//C口5V输入限流2.8A
#define SW6208_CHG1_C_2A7               0xA0U//C口5V输入限流2.7A
#define SW6208_CHG1_C_2A6               0x90U//C口5V输入限流2.6A
#define SW6208_CHG1_C_2A5               0x80U//C口5V输入限流2.5A
#define SW6208_CHG1_C_2A4               0x70U//C口5V输入限流2.4A
#define SW6208_CHG1_C_2A3               0x60U//C口5V输入限流2.3A
#define SW6208_CHG1_C_2A2               0x50U//C口5V输入限流2.2A
#define SW6208_CHG1_C_2A1               0x40U//C口5V输入限流2.1A
#define SW6208_CHG1_C_1A7               0x30U//C口5V输入限流1.7A
#define SW6208_CHG1_C_1A9               0x20U//C口5V输入限流1.9A
#define SW6208_CHG1_C_1A8               0x10U//C口5V输入限流1.8A
#define SW6208_CHG1_C_2A0               0x00U//C口5V输入限流2.0A
#define SW6208_CHG1_B_2A4               0x0EU//B口5V输入限流2.4A
#define SW6208_CHG1_B_2A3               0x0CU//B口5V输入限流2.3A
#define SW6208_CHG1_B_2A2               0x0AU//B口5V输入限流2.2A
#define SW6208_CHG1_B_2A1               0x08U//B口5V输入限流2.1A
#define SW6208_CHG1_B_1A7               0x06U//B口5V输入限流1.7A
#define SW6208_CHG1_B_1A9               0x04U//B口5V输入限流1.9A
#define SW6208_CHG1_B_1A8               0x02U//B口5V输入限流1.8A
#define SW6208_CHG1_B_2A0               0x00U//B口5V输入限流2.0A

//0x44  SW6208_CTRG_CHG2                充电配置2
#define SW6208_CHG2_MSK                 0x7FU//充电配置2寄存器有效位
#define SW6208_CHG2_HCUTOFF             0x40U//更大的充电截止电流
#define SW6208_CHG2_VL_45               0x38U//5V低压阈值4.5V
#define SW6208_CHG2_VL_44               0x30U//5V低压阈值4.4V
#define SW6208_CHG2_VL_43               0x28U//5V低压阈值4.3V
#define SW6208_CHG2_VL_42               0x20U//5V低压阈值4.2V
#define SW6208_CHG2_VL_49               0x18U//5V低压阈值4.9V
#define SW6208_CHG2_VL_48               0x10U//5V低压阈值4.8V
#define SW6208_CHG2_VL_47               0x08U//5V低压阈值4.7V
#define SW6208_CHG2_VL_46               0x00U//5V低压阈值4.6V
#define SW6208_CHG2_TH_95               0x07U//充电温度环路95°C
#define SW6208_CHG2_TH_90               0x06U//充电温度环路90°C
#define SW6208_CHG2_TH_85               0x05U//充电温度环路85°C
#define SW6208_CHG2_TH_80               0x04U//充电温度环路80°C
#define SW6208_CHG2_TH_115              0x03U//充电温度环路115°C
#define SW6208_CHG2_TH_110              0x02U//充电温度环路110°C
#define SW6208_CHG2_TH_105              0x01U//充电温度环路105°C
#define SW6208_CHG2_TH_100              0x00U//充电温度环路100°C

//0x45  SW6208_CTRG_CHG3                充电配置3
#define SW6208_CHG3_MSK                 0x3FU//充电配置3寄存器有效位
#define SW6208_CHG3_VL_11429            0x38U//12V低压阈值11.429V
#define SW6208_CHG3_VL_11321            0x30U//12V低压阈值11.321V
#define SW6208_CHG3_VL_11215            0x28U//12V低压阈值11.215V
#define SW6208_CHG3_VL_11881            0x18U//12V低压阈值11.881V
#define SW6208_CHG3_VL_11765            0x10U//12V低压阈值11.765V
#define SW6208_CHG3_VL_11650            0x08U//12V低压阈值11.650V
#define SW6208_CHG3_VL_11538            0x00U//12V低压阈值11.429V
#define SW6208_CHG3_VL_8392             0x07U//9V低压阈值8.392V
#define SW6208_CHG3_VL_8295             0x06U//9V低压阈值8.295V
#define SW6208_CHG3_VL_8182             0x05U//9V低压阈值8.182V
#define SW6208_CHG3_VL_8072             0x04U//9V低压阈值8.072V
#define SW6208_CHG3_VL_8867             0x03U//9V低压阈值8.867V
#define SW6208_CHG3_VL_8738             0x02U//9V低压阈值8.738V
#define SW6208_CHG3_VL_8612             0x01U//9V低压阈值8.612V
#define SW6208_CHG3_VL_8490             0x00U//9V低压阈值8.490V

//0x46  SW6208_CTRG_CHG4                充电配置4
#define SW6208_CHG4_MSK                 0xC0U//充电配置4寄存器有效位
#define SW6208_CHG4_500K                0xC0U//充电开关频率500K
#define SW6208_CHG4_800K                0x80U//充电开关频率800K
#define SW6208_CHG4_400K                0x40U//充电开关频率400K
#define SW6208_CHG4_600K                0x00U//充电开关频率600K

//0x47  SW6208_CTRG_NTC0                NTC配置0
#define SW6208_NTC0_MSK                 0xFCU//NTC配置0寄存器有效位
#define SW6208_NTC0_LM5                 0xC0U//放电NTC低温保护门限-5°C
#define SW6208_NTC0_LM10                0x80U//放电NTC低温保护门限-10°C
#define SW6208_NTC0_LM15                0x40U//放电NTC低温保护门限-15°C
#define SW6208_NTC0_LM20                0x00U//放电NTC低温保护门限-20°C
#define SW6208_NTC0_H65                 0x30U//放电NTC高温保护门限65°C
#define SW6208_NTC0_H55                 0x20U//放电NTC高温保护门限55°C
#define SW6208_NTC0_H50                 0x10U//放电NTC高温保护门限50°C
#define SW6208_NTC0_H60                 0x00U//放电NTC高温保护门限60°C
#define SW6208_NTC0_DIS                 0x08U//不使能NTC放电保护
#define SW6208_NTC0_NOAUTO              0x04U//不使能NTC放电温度自适应

//0x48  SW6208_CTRG_NTC1                NTC配置1
#define SW6208_NTC1_MSK                 0xFBU//NTC配置1寄存器有效位
#define SW6208_NTC1_JEITA               0x80U//JEITA规范使能
#define SW6208_NTC1_H60                 0x60U//充电NTC高温保护门限60°C
#define SW6208_NTC1_H55                 0x40U//充电NTC高温保护门限55°C
#define SW6208_NTC1_H45                 0x20U//充电NTC高温保护门限45°C
#define SW6208_NTC1_H50                 0x00U//充电NTC高温保护门限50°C
#define SW6208_NTC1_LM15                0x18U//充电NTC低温保护门限-15°C
#define SW6208_NTC1_LM10                0x10U//充电NTC低温保护门限-10°C
#define SW6208_NTC1_LM5                 0x08U//充电NTC低温保护门限-5°C
#define SW6208_NTC1_L0                  0x00U//充电NTC低温保护门限0°C
#define SW6208_NTC1_HYS10               0x02U//充电NTC阈值迟滞10°C
#define SW6208_NTC1_HYS5                0x00U//充电NTC阈值迟滞5°C
#define SW6208_NTC1_40U                 0x01U//NTC电流40微安
#define SW6208_NTC1_80U                 0x00U//NTC电流80微安

//0x49  SW6208_CTRG_TEMP                温度配置
#define SW6208_TEMP_MSK                 0x7FU//温度配置寄存器有效位
#define SW6208_TEMP_OTP_160             0x70U//过温保护阈值160°C
#define SW6208_TEMP_OTP_150             0x60U//过温保护阈值150°C
#define SW6208_TEMP_OTP_140             0x50U//过温保护阈值140°C
#define SW6208_TEMP_OTP_90              0x40U//过温保护阈值90°C
#define SW6208_TEMP_OTP_120             0x30U//过温保护阈值120°C
#define SW6208_TEMP_OTP_110             0x20U//过温保护阈值110°C
#define SW6208_TEMP_OTP_100             0x10U//过温保护阈值100°C
#define SW6208_TEMP_OTP_130             0x00U//过温保护阈值130°C
#define SW6208_TEMP_BST_DIS             0x08U//不使能boost温度自适应
#define SW6208_TEMP_BST_95              0x07U//boost温度自适应阈值95°C
#define SW6208_TEMP_BST_90              0x06U//boost温度自适应阈值90°C
#define SW6208_TEMP_BST_85              0x05U//boost温度自适应阈值85°C
#define SW6208_TEMP_BST_80              0x04U//boost温度自适应阈值80°C
#define SW6208_TEMP_BST_115             0x03U//boost温度自适应阈值115°C
#define SW6208_TEMP_BST_110             0x02U//boost温度自适应阈值110°C
#define SW6208_TEMP_BST_105             0x01U//boost温度自适应阈值105°C
#define SW6208_TEMP_BST_100             0x00U//boost温度自适应阈值100°C
/*******************************函数声明区*************************************/
//基本操作区
SW6208_RET SW6208_ByteWrite(SW6208_ARGS(uint8_t reg, uint8_t data));
SW6208_RET SW6208_ByteRead(SW6208_ARGS(uint8_t reg, volatile uint8_t *data));
SW6208_RET SW6208_ByteModify(SW6208_ARGS(uint8_t reg, uint8_t mask, uint8_t data));
//数据读取区
SW6208_RET SW6208_ADCLoad(SW6208_NOARG);//从芯片读取ADC数据与NTC寄存器（0x13,0x14,0x48）
float SW6208_ReadVBUS(void);//读取BUS电压
float SW6208_ReadVBAT(void);//读取BAT电压
float SW6208_ReadTCHIP(void);//读取芯片温度
float SW6208_ReadVNTC(void);//读取NTC电压
float SW6208_ReadICharge(void);//读取充电电流（单位：mA）
float SW6208_ReadIDischarge(void);//读取放电电流
float SW6208_ReadNTCValue(void);//读取NTC阻值，单位kΩ
//端口数据
SW6208_RET SW6208_PortLoad(SW6208_NOARG);//载入与端口状态有关的寄存器(0x08,0x09,0x0C,0x29)
uint8_t SW6208_IsLPortON(void);//检查L口通路是否打开
uint8_t SW6208_IsBPortON(void);//检查B口通路是否打开
uint8_t SW6208_IsCPortON(void);//检查C口通路是否打开
uint8_t SW6208_IsA2PortON(void);//检查A2口通路是否打开
uint8_t SW6208_IsA1PortON(void);//检查A1口通路是否打开
//系统状态
SW6208_RET SW6208_StatusLoad(SW6208_NOARG);//更新状态相关寄存器（0x06,0x07,0x0A,0x0B,0x0C,0x0F,0x1F,0x21,0x2E）
uint8_t SW6208_IsCharging(void);//检查是否处于充电
uint8_t SW6208_IsDischarging(void);//检查是否处于放电
uint8_t SW6208_IsFullCharge(void);//检查是否充满
uint8_t SW6208_IsNTCOverTemp(void);//检查是否发生NTC过温事件
const char *SW6208_ReadProtocol(void);//返回当前快充协议名称字符串
//容量状态
SW6208_RET SW6208_CapacityLoad(SW6208_NOARG);//读取芯片容量相关寄存器（0x73,0x74,0x7E）
uint8_t SW6208_ReadCapacity(void);//读取SW6208计算出的当前电量
float SW6208_ReadCapValue(void);//读取电池能量，单位Wh
//芯片操作
SW6208_RET SW6208_Click(SW6208_NOARG);//触发短按键事件
SW6208_RET SW6208_PortCRemove(SW6208_NOARG);//触发TypeC 口Sink设备拔出事件
SW6208_RET SW6208_PortCInsert(SW6208_NOARG);//触发TypeC 口Sink设备插入事件
SW6208_RET SW6208_PortA2Remove(SW6208_NOARG);//触发A2口设备拔出事件
SW6208_RET SW6208_PortA2Insert(SW6208_NOARG);//触发A2口设备插入事件
SW6208_RET SW6208_PortA1Remove(SW6208_NOARG);//触发A1口设备拔出事件
SW6208_RET SW6208_PortA1Insert(SW6208_NOARG);//触发A1口设备插入事件
SW6208_RET SW6208_PDEnableSet(SW6208_ARGS(uint8_t en));//是否使能PD协议
SW6208_RET SW6208_SCPEnableSet(SW6208_ARGS(uint8_t en));//是否使能SCP协议
SW6208_RET SW6208_FCPEnableSet(SW6208_ARGS(uint8_t en));//是否使能FCP协议
SW6208_RET SW6208_AFCEnableSet(SW6208_ARGS(uint8_t en));//是否使能AFC协议
SW6208_RET SW6208_PEEnableSet(SW6208_ARGS(uint8_t en));//是否使能PE协议
SW6208_RET SW6208_PortCRoleSet(SW6208_ARGS(uint8_t role));//设置C口角色，0：DRP，1：SINK，2：SOURCE);
//初始化
SW6208_RET SW6208_Init(SW6208_NOARG);                      //初始化，最好系统上电后立刻执行
uint8_t SW6208_IsInitialized(void);             //检测SW6208是否已初始化过，须在SW6208_PortLoad()后执行


#ifdef __cplusplus
}
#endif

#endif