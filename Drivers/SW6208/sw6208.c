/*
 *SW6208驱动库本体，支持基本的数据读取与一些操作
 *SW6208仅支持单字节读写，多字节写会给NACK，多字节读给0xFF
 */
#include "sw6208.h"

volatile struct SW6208_StatusTypedef SW6208_Status = {0x01,0x00};//SW6208状态全局变量

/*******************************基本操作区*************************************/
SW6208_RET SW6208_ByteWrite(SW6208_ARGS(uint8_t reg, uint8_t data)){
    PT_BEGIN(pt);
    PT_SEM_WAIT(pt, &i2c_mutex);
    SW6208_Status.sendbuf[2] = data;
    SW6208_EXEC(SW6208_I2C_Transmit(SW6208_I2C_ADDR, reg, (uint8_t*)SW6208_Status.sendbuf+ 2, 1, (uint8_t*)&SW6208_Status.flag));
    SW6208_UNTIL(SW6208_Status.flag);
    PT_SEM_SIGNAL(pt, &i2c_mutex);
    PT_END(pt);
}

SW6208_RET SW6208_ByteRead(SW6208_ARGS(uint8_t reg, volatile uint8_t *data)){
    PT_BEGIN(pt);
    PT_SEM_WAIT(pt, &i2c_mutex);
    SW6208_EXEC(SW6208_I2C_Receive(SW6208_I2C_ADDR, reg, (uint8_t*)data, 1, (uint8_t*)&SW6208_Status.flag));
    SW6208_UNTIL(SW6208_Status.flag);
    PT_SEM_SIGNAL(pt, &i2c_mutex);
    PT_END(pt);
}
  
/*SW6208修改对应寄存器的特定位，不对寄存器组作检查
 *遵循读-修改-写的顺序
 *将mask中为1的位按data中的对应位修改成1或0，mask中为0的位不被修改
 */
SW6208_RET SW6208_ByteModify(SW6208_ARGS(uint8_t reg, uint8_t mask, uint8_t data)){
    SW6208_FUNC_BEGIN;
    SW6208_SPAWN(SW6208_ByteRead,reg,SW6208_Status.sendbuf);
    SW6208_Status.sendbuf[0] = (SW6208_Status.sendbuf[0] &(~mask)) | (data & mask);
    SW6208_SPAWN(SW6208_ByteWrite,reg,SW6208_Status.sendbuf[0]);
    SW6208_FUNC_END;
}

/********************************状态读取区************************************/
SW6208_RET SW6208_ADCLoad(SW6208_NOARG){//读取ADC数据与NTC寄存器（0x13,0x14,0x48）
    uint16_t temp;
    SW6208_FUNC_BEGIN;
    SW6208_SPAWN(SW6208_ByteWrite,SW6208_CTRG_ADC_SET, SW6208_ADC_SET_VBAT);
    SW6208_SPAWN(SW6208_ByteRead,SW6208_STRG_ADCH, SW6208_Status.sendbuf);
    SW6208_SPAWN(SW6208_ByteRead,SW6208_STRG_ADCL, SW6208_Status.sendbuf+ 1);
    temp = (SW6208_Status.sendbuf[1]&0x0FU) | ((uint16_t)SW6208_Status.sendbuf[0]<<4);
    if(temp > 400U && temp < 4000U) SW6208_Status.vbat = temp;
    SW6208_SPAWN(SW6208_ByteWrite,SW6208_CTRG_ADC_SET, SW6208_ADC_SET_VBUS);
    SW6208_SPAWN(SW6208_ByteRead,SW6208_STRG_ADCH, SW6208_Status.sendbuf);
    SW6208_SPAWN(SW6208_ByteRead,SW6208_STRG_ADCL, SW6208_Status.sendbuf+ 1);
    temp = (SW6208_Status.sendbuf[1]&0x0FU) | ((uint16_t)SW6208_Status.sendbuf[0]<<4);
    if(temp > 200U && temp < 4000U) SW6208_Status.vbus = temp;
    SW6208_SPAWN(SW6208_ByteWrite,SW6208_CTRG_ADC_SET, SW6208_ADC_SET_TCHIP);
    SW6208_SPAWN(SW6208_ByteRead,SW6208_STRG_ADCH, SW6208_Status.sendbuf);
    SW6208_SPAWN(SW6208_ByteRead,SW6208_STRG_ADCL, SW6208_Status.sendbuf+ 1);
    temp = (SW6208_Status.sendbuf[1]&0x0FU) | ((uint16_t)SW6208_Status.sendbuf[0]<<4);
    if(temp > 1500U && temp < 3000U) SW6208_Status.tchip = temp;
    SW6208_SPAWN(SW6208_ByteWrite,SW6208_CTRG_ADC_SET, SW6208_ADC_SET_VNTC);
    SW6208_SPAWN(SW6208_ByteRead,SW6208_STRG_ADCH, SW6208_Status.sendbuf);
    SW6208_SPAWN(SW6208_ByteRead,SW6208_STRG_ADCL, SW6208_Status.sendbuf+ 1);
    temp = (SW6208_Status.sendbuf[1]&0x0FU) | ((uint16_t)SW6208_Status.sendbuf[0]<<4);
    if(temp > 18U && temp < 2000U) SW6208_Status.vntc = temp;
    SW6208_SPAWN(SW6208_ByteWrite,SW6208_CTRG_ADC_SET, SW6208_ADC_SET_ICHG);
    SW6208_SPAWN(SW6208_ByteRead,SW6208_STRG_ADCH, SW6208_Status.sendbuf);
    SW6208_SPAWN(SW6208_ByteRead,SW6208_STRG_ADCL, SW6208_Status.sendbuf+ 1);
    temp = (SW6208_Status.sendbuf[1]&0x0FU) | ((uint16_t)SW6208_Status.sendbuf[0]<<4);
    if(temp < 2000U) SW6208_Status.icharge = temp;
    SW6208_SPAWN(SW6208_ByteWrite,SW6208_CTRG_ADC_SET, SW6208_ADC_SET_IDIS);
    SW6208_SPAWN(SW6208_ByteRead,SW6208_STRG_ADCH, SW6208_Status.sendbuf);
    SW6208_SPAWN(SW6208_ByteRead,SW6208_STRG_ADCL, SW6208_Status.sendbuf+ 1);
    temp = (SW6208_Status.sendbuf[1]&0x0FU) | ((uint16_t)SW6208_Status.sendbuf[0]<<4);
    if(temp < 2000U) SW6208_Status.idischg = temp;
    SW6208_SPAWN(SW6208_ByteRead,SW6208_CTRG_NTC1, &SW6208_Status.ntcstat);
    SW6208_FUNC_END;
}
float SW6208_ReadVBUS(void)//读取BUS电压
{
    return SW6208_Status.vbus*0.004f;
}
float SW6208_ReadVBAT(void)//读取BAT电压
{
    return SW6208_Status.vbat*0.0012f;
}
float SW6208_ReadTCHIP(void)//读取并转换芯片温度
{
    return ((int16_t)SW6208_Status.tchip-1863)/6.82f;
}
float SW6208_ReadVNTC(void)//读取并转换NTC电压
{
    return SW6208_Status.vntc*1.1f;
}
float SW6208_ReadICharge(void)//读取充电电流
{
    return SW6208_Status.icharge*2.24f;
}
float SW6208_ReadIDischarge(void)//读取放电电流
{
    return SW6208_Status.idischg*2.24f;
}
float SW6208_ReadNTCValue(void)//读取NTC阻值，单位kΩ
{
    if(SW6208_Status.ntcstat&SW6208_NTC1_40U) return SW6208_ReadVNTC()/40;//40uA
    else return SW6208_ReadVNTC()/80;
}

SW6208_RET SW6208_PortLoad(SW6208_NOARG){//载入与端口状态有关的寄存器(0x08,0x09,0x0C,0x29)
    SW6208_FUNC_BEGIN;
    SW6208_SPAWN(SW6208_ByteRead,SW6208_STRG_PORTRMV, &SW6208_Status.portrmv);
    SW6208_SPAWN(SW6208_ByteRead,SW6208_STRG_PORTINS, &SW6208_Status.portins);
    SW6208_SPAWN(SW6208_ByteRead,SW6208_STRG_SYS, &SW6208_Status.sysstat);
    SW6208_SPAWN(SW6208_ByteRead,SW6208_STRG_TYPEC, &SW6208_Status.tpcstat);
    SW6208_FUNC_END;
}
uint8_t SW6208_IsLPortON(void)//检查L口通路是否打开
{
    return SW6208_Status.sysstat & SW6208_SYS_L_ON;
}
uint8_t SW6208_IsBPortON(void)//检查B口通路是否打开
{
    return SW6208_Status.sysstat & SW6208_SYS_B_ON;
}
uint8_t SW6208_IsCPortON(void)//检查C口通路是否打开
{
    return SW6208_Status.sysstat & SW6208_SYS_C_ON;
}
uint8_t SW6208_IsA2PortON(void)//检查A2口通路是否打开
{
    return SW6208_Status.sysstat & SW6208_SYS_A2_ON;
}
uint8_t SW6208_IsA1PortON(void)//检查A1口通路是否打开
{
    return SW6208_Status.sysstat & SW6208_SYS_A1_ON;
}

SW6208_RET SW6208_StatusLoad(SW6208_NOARG){//更新状态相关寄存器（0x06,0x07,0x0A,0x0B,0x0C,0x0F,0x1F,0x21,0x2E）
    SW6208_FUNC_BEGIN;
    SW6208_SPAWN(SW6208_ByteRead,SW6208_STRG_DISPLAY, &SW6208_Status.display);
    SW6208_SPAWN(SW6208_ByteRead,SW6208_STRG_KEY, &SW6208_Status.keystat);
    SW6208_SPAWN(SW6208_ByteRead,SW6208_STRG_BAT, &SW6208_Status.batstat);
    SW6208_SPAWN(SW6208_ByteRead,SW6208_STRG_FAULT0, &SW6208_Status.fault0);
    SW6208_SPAWN(SW6208_ByteRead,SW6208_STRG_SYS, &SW6208_Status.sysstat);
    SW6208_SPAWN(SW6208_ByteRead,SW6208_STRG_QC, &SW6208_Status.qcstat);
    SW6208_SPAWN(SW6208_ByteRead,SW6208_STRG_QCLED, &SW6208_Status.qcled);
    SW6208_SPAWN(SW6208_ByteRead,SW6208_STRG_FAULT1, &SW6208_Status.vccov);
    SW6208_SPAWN(SW6208_ByteRead,SW6208_CTRG_LUSET, &SW6208_Status.lustat);
    SW6208_FUNC_END;
}
uint8_t SW6208_IsCharging(void){//检查是否处于充电
    return SW6208_Status.sysstat & SW6208_SYS_CHG_ON;
}
uint8_t SW6208_IsDischarging(void){//检查是否处于放电
    return SW6208_Status.sysstat & SW6208_SYS_DIS_ON;
}
uint8_t SW6208_IsFullCharge(void){//检查是否充满
    return SW6208_Status.batstat & SW6208_BAT_FULL;
}
uint8_t SW6208_IsNTCOverTemp(void){//检查是否发生NTC过温事件
    return SW6208_Status.batstat & SW6208_BAT_NTC_OT;
}

const char *quickcharge[] = {"PD2.0","PD3.0","PPS","QC2.0","QC3.0","FCP","PE","SFCP","AFC","SCP","NO QC"};//协议名称字符数组
const char *SW6208_ReadProtocol(void){//协议名称读取函数
    switch(SW6208_Status.qcstat & 0x7F){
        case SW6208_QC_PD_SNK:
        case SW6208_QC_PD_SRC:
            if(SW6208_Status.qcstat & 0x80) return quickcharge[1];//PD3.0
            else return quickcharge[0];//PD2.0
            break;
        case SW6208_QC_PPS_SRC:
            return quickcharge[2];
            break;//PPS
        case SW6208_QC_AFC_SNK:
        case SW6208_QC_AFC_SRC:
            return quickcharge[8];
            break;//AFC
        case SW6208_QC_FCP_SNK:
        case SW6208_QC_FCP_SRC:
            return quickcharge[5];
            break;//FCP
        case SW6208_QC_SCP_SNK:
        case SW6208_QC_SCP_SRC:
            return quickcharge[9];
            break;//SCP
        case SW6208_QC_PE_SNK:
        case SW6208_QC_PE_SRC:
            return quickcharge[6];
            break;//PE
        case SW6208_QC_QC2_SRC:
            return quickcharge[3];
            break;//QC2.0
        case SW6208_QC_QC3_SRC:
            return quickcharge[4];
            break;//QC3.0
        case SW6208_QC_SFCP_SRC:
            return quickcharge[7];
            break;//SFCP
        default:
            return quickcharge[10];
            break;
    }    
}

SW6208_RET SW6208_CapacityLoad(SW6208_NOARG){//读取芯片容量相关寄存器（0x73,0x74,0x7E）
    SW6208_FUNC_BEGIN;
    SW6208_SPAWN(SW6208_ByteRead, SW6208_STRG_CAP1, &SW6208_Status.capacity);
    SW6208_SPAWN(SW6208_ByteRead, SW6208_STRG_CAPL, SW6208_Status.sendbuf);
    SW6208_SPAWN(SW6208_ByteRead, SW6208_STRG_CAPH, SW6208_Status.sendbuf+1);
    SW6208_Status.capval = ((uint16_t)SW6208_Status.sendbuf[1]<<8 | SW6208_Status.sendbuf[0]);
    SW6208_FUNC_END;
}
uint8_t SW6208_ReadCapacity(void){//读取SW6208计算出的当前电量
    return SW6208_Status.capacity;
}
float SW6208_ReadCapValue(void){//读取电池能量，单位Wh
    return SW6208_Status.capval*0.1695f;
}

/*****************************芯片操作区***************************************/
SW6208_RET SW6208_Click(SW6208_NOARG){//触发短按键事件
    SW6208_FUNC_BEGIN;
    SW6208_SPAWN(SW6208_ByteWrite,SW6208_CTRG_KEYEVT, SW6208_KEYEVT_TRIG);
    SW6208_FUNC_END;
}
SW6208_RET SW6208_PortCRemove(SW6208_NOARG){//触发TypeC 口Sink设备拔出事件
    SW6208_FUNC_BEGIN;
    SW6208_SPAWN(SW6208_ByteWrite,SW6208_CTRG_PORTEVT, SW6208_PORTEVT_CRMV);
    SW6208_FUNC_END;
}
SW6208_RET SW6208_PortCInsert(SW6208_NOARG){//触发TypeC 口Sink设备插入事件
    SW6208_FUNC_BEGIN;
    SW6208_SPAWN(SW6208_ByteWrite,SW6208_CTRG_PORTEVT, SW6208_PORTEVT_CINS);
    SW6208_FUNC_END;
}
SW6208_RET SW6208_PortA2Remove(SW6208_NOARG){//触发A2口设备拔出事件
    SW6208_FUNC_BEGIN;
    SW6208_SPAWN(SW6208_ByteWrite,SW6208_CTRG_PORTEVT, SW6208_PORTEVT_A2RMV);
    SW6208_FUNC_END;
}
SW6208_RET SW6208_PortA2Insert(SW6208_NOARG){//触发A2口设备插入事件
    SW6208_FUNC_BEGIN;
    SW6208_SPAWN(SW6208_ByteWrite,SW6208_CTRG_PORTEVT, SW6208_PORTEVT_A2INS);
    SW6208_FUNC_END;
}
SW6208_RET SW6208_PortA1Remove(SW6208_NOARG){//触发A1口设备拔出事件
    SW6208_FUNC_BEGIN;
    SW6208_SPAWN(SW6208_ByteWrite,SW6208_CTRG_PORTEVT, SW6208_PORTEVT_A1RMV);
    SW6208_FUNC_END;
}
SW6208_RET SW6208_PortA1Insert(SW6208_NOARG){//触发A1口设备插入事件
    SW6208_FUNC_BEGIN;
    SW6208_SPAWN(SW6208_ByteWrite,SW6208_CTRG_PORTEVT, SW6208_PORTEVT_A1INS);
    SW6208_FUNC_END;
}
SW6208_RET SW6208_PDEnableSet(SW6208_ARGS(uint8_t en)){//是否使能PD协议
    SW6208_FUNC_BEGIN;
    SW6208_SPAWN(SW6208_ByteModify,SW6208_CTRG_QC2, SW6208_QC2_PDSRC_DIS|SW6208_QC2_PDSNK_DIS, en? 0x00:SW6208_QC2_PDSRC_DIS|SW6208_QC2_PDSNK_DIS);
    SW6208_FUNC_END;
}
SW6208_RET SW6208_SCPEnableSet(SW6208_ARGS(uint8_t en)){//是否使能SCP协议
    SW6208_FUNC_BEGIN;
    SW6208_SPAWN(SW6208_ByteModify,SW6208_CTRG_QC3, SW6208_QC3_SCP_SRC_DIS|SW6208_QC3_SCP_SNK_DIS, en? 0x00:SW6208_QC3_SCP_SRC_DIS|SW6208_QC3_SCP_SNK_DIS);
    SW6208_FUNC_END;
}
SW6208_RET SW6208_FCPEnableSet(SW6208_ARGS(uint8_t en)){//是否使能FCP协议
    SW6208_FUNC_BEGIN;
    SW6208_SPAWN(SW6208_ByteModify,SW6208_CTRG_QC2, SW6208_QC2_FCPSRC_DIS, en? 0x00:SW6208_QC2_FCPSRC_DIS);
    SW6208_FUNC_END;
}
SW6208_RET SW6208_AFCEnableSet(SW6208_ARGS(uint8_t en)){//是否使能AFC协议
    SW6208_FUNC_BEGIN;
    SW6208_SPAWN(SW6208_ByteModify,SW6208_CTRG_QC3, SW6208_QC3_AFC_SRC_DIS|SW6208_QC3_AFC_SNK_DIS, en? 0x00:SW6208_QC3_AFC_SRC_DIS|SW6208_QC3_AFC_SNK_DIS);
    SW6208_FUNC_END;
}
SW6208_RET SW6208_PEEnableSet(SW6208_ARGS(uint8_t en)){//是否使能PE协议
    SW6208_FUNC_BEGIN;
    SW6208_SPAWN(SW6208_ByteModify,SW6208_CTRG_QC3, SW6208_QC3_PE_SRC_DIS|SW6208_QC3_PE_SNK_DIS, en? 0x00:SW6208_QC3_PE_SRC_DIS|SW6208_QC3_PE_SNK_DIS);
    SW6208_FUNC_END;
}
SW6208_RET SW6208_PortCRoleSet(SW6208_ARGS(uint8_t role)){//设置C口角色，0：DRP，1：SINK，2：SOURCE
    SW6208_FUNC_BEGIN;
    SW6208_SPAWN(SW6208_ByteModify,SW6208_CTRG_TYPEC, SW6208_TYPEC_MSK, (role<<2)& SW6208_TYPEC_MSK);
    SW6208_FUNC_END;
}

    
/********************************初始化区**************************************/
//用户需要根据需求对该函数参照头文件中的寄存器位定义进行修改
SW6208_RET SW6208_Init(SW6208_NOARG){//SW6208不支持连续写，无法使用初始化表，只能一个一个赋值
    SW6208_FUNC_BEGIN;
    SW6208_SPAWN(SW6208_ByteWrite,SW6208_CTRG_KEY, SW6208_KEY_DC_NOBST|SW6208_KEY_LP_LU_WLED|SW6208_KEY_SC_A1|SW6208_KEY_SC_500M|SW6208_KEY_SC_NORSP);
    SW6208_SPAWN(SW6208_ByteWrite,SW6208_STRG_DISPLAY, SW6208_DISPLAY_INITED);
    SW6208_SPAWN(SW6208_ByteModify,SW6208_CTRG_QC0, SW6208_QC0_MSK, SW6208_QC0_C_DMINS_EN|SW6208_QC0_AFC_12V_EN|SW6208_QC0_FCP_12V_EN|SW6208_QC0_VIN_12V_DIS|SW6208_QC0_VOUT_12V);
    SW6208_SPAWN(SW6208_ByteWrite,SW6208_CTRG_QC1, 0x00);
    SW6208_SPAWN(SW6208_ByteModify,SW6208_CTRG_QC2, SW6208_QC2_MSK, SW6208_QC2_PSDLOAD_DIS|SW6208_QC2_CNOLOAD_DIS|SW6208_QC2_A_QCSRC_EN|SW6208_QC2_FCPSRC_EN);
    SW6208_SPAWN(SW6208_ByteWrite,SW6208_CTRG_QC3, 0x00);
    SW6208_SPAWN(SW6208_ByteModify,SW6208_CTRG_QC4, SW6208_QC4_MSK, SW6208_QC4_CHGDIS_DIS|SW6208_QC4_TYPEC_QCSRC_EN);
    SW6208_SPAWN(SW6208_ByteModify,SW6208_CTRG_WLED, SW6208_WLED_MSK, SW6208_WLED_REGSET_DIS|SW6208_WLED_DIS);
    SW6208_SPAWN(SW6208_ByteModify,SW6208_CTRG_TYPEC, SW6208_TYPEC_MSK, SW6208_TYPEC_DRP);
    SW6208_SPAWN(SW6208_ByteModify,SW6208_CTRG_PD0, SW6208_PD0_MSK, SW6208_PD0_PD30|SW6208_PD0_12VPPS);
    SW6208_SPAWN(SW6208_ByteModify,SW6208_CTRG_PD1, SW6208_PD1_MSK, SW6208_PD1_FIX12V|SW6208_PD1_ENPPS0|SW6208_PD1_R5VPDO_DIS|SW6208_PD1_ENPPS1);
    SW6208_SPAWN(SW6208_ByteModify,SW6208_CTRG_PD2, SW6208_PD2_MSK, SW6208_PD2_5V_3A0|SW6208_PD2_9V_2A0|SW6208_PD2_12V_1A5);
    SW6208_SPAWN(SW6208_ByteModify,SW6208_CTRG_PD3, SW6208_PD3_MSK, SW6208_PD3_MPPD_EN|SW6208_PD3_S1V2_EN);
    SW6208_SPAWN(SW6208_ByteWrite,SW6208_CTRG_PORT0, SW6208_PORT0_SPNL_32S|SW6208_PORT0_MPNL_32S|SW6208_PORT0_INL_55M|SW6208_PORT0_HVINL_EN);
    SW6208_SPAWN(SW6208_ByteModify,SW6208_CTRG_PORT1, SW6208_PORT1_MSK, SW6208_PORT1_A_DM_EN|SW6208_PORT1_LCOMP_EN|SW6208_PORT1_A1_DIS|SW6208_PORT1_A2_DIS|SW6208_PORT1_CNL_DIS);
    SW6208_SPAWN(SW6208_ByteModify,SW6208_CTRG_WLSS, SW6208_WLSS_MSK, SW6208_WLSS_NL_120M|SW6208_WLSS_NL_120S|SW6208_WLSS_DIS);
    SW6208_SPAWN(SW6208_ByteModify,SW6208_CTRG_LUEN, SW6208_LUEN_MSK, SW6208_LUEN_DIS);
    SW6208_SPAWN(SW6208_ByteWrite,SW6208_CTRG_BST0, SW6208_BST0_UVLO_2V8|SW6208_BST0_UVLOHYS_V5|SW6208_BST0_400K|SW6208_BST0_POMAX_21);
    SW6208_SPAWN(SW6208_ByteModify,SW6208_CTRG_BST1, SW6208_BST1_MSK, SW6208_BST1_VOS_100M|SW6208_BST1_CC_15P|SW6208_BST1_MP_4A2);
    SW6208_SPAWN(SW6208_ByteModify,SW6208_CTRG_CHG0, SW6208_CHG0_MSK, SW6208_CHG0_9V_1A8|SW6208_CHG0_12V_1A2);
    SW6208_SPAWN(SW6208_ByteModify,SW6208_CTRG_CHG1, SW6208_CHG1_MSK, SW6208_CHG1_C_3A2|SW6208_CHG1_B_2A4);
    SW6208_SPAWN(SW6208_ByteModify,SW6208_CTRG_CHG2, SW6208_CHG2_MSK, SW6208_CHG2_VL_43|SW6208_CHG2_TH_115);
    SW6208_SPAWN(SW6208_ByteModify,SW6208_CTRG_CHG3, SW6208_CHG3_MSK, SW6208_CHG3_VL_11321|SW6208_CHG3_VL_8392);
    SW6208_SPAWN(SW6208_ByteModify,SW6208_CTRG_CHG4, SW6208_CHG4_MSK, SW6208_CHG4_400K);
    SW6208_SPAWN(SW6208_ByteModify,SW6208_CTRG_NTC0, SW6208_NTC0_MSK, SW6208_NTC0_LM20|SW6208_NTC0_H65|SW6208_NTC0_NOAUTO);
    SW6208_SPAWN(SW6208_ByteModify,SW6208_CTRG_NTC1, SW6208_NTC1_MSK, SW6208_NTC1_H60|SW6208_NTC1_LM15|SW6208_NTC1_HYS5);
    SW6208_SPAWN(SW6208_ByteModify,SW6208_CTRG_TEMP, SW6208_TEMP_MSK, SW6208_TEMP_OTP_160|SW6208_TEMP_BST_DIS|SW6208_TEMP_BST_115);
    SW6208_SPAWN(SW6208_ByteWrite,SW6208_CTRG_PORTEVT, SW6208_PORTEVT_CRMV|SW6208_PORTEVT_A2RMV|SW6208_PORTEVT_A1RMV);
    SW6208_Status.initialized = 1;
    SW6208_FUNC_END;
}

uint8_t SW6208_IsInitialized(void){
    if(SW6208_Status.initialized && (SW6208_Status.display&SW6208_DISPLAY_INITED)) return 1;
    else return 0;
}
        