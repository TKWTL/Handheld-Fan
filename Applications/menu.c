/*
/菜单行为定义
*/

#include "menu.h"

/*******************************开机主菜单*************************************/
enum {
    H_OFF,
    H_FAN,
    H_MENU
}start_menu_highlight;
void Start_Menu_Key(){
    switch(start_menu_highlight){
        case H_FAN:
            if(Key_EdgeDetect(KeyIndex_KEY_DOWN) == KeyEdge_Rising)start_menu_highlight = H_OFF;//向下切换
            if(KEY_LEFT_GetIO() && fanset > 200) {fanset-= 4; PWM0_SET(fanset);}//减小风力
            else if(KEY_RIGHT_GetIO() && fanset < 1024) {fanset+= 4; PWM0_SET(fanset);}//增加风力
        
            if(fan_is_on == fan_is_on_last){//切换过程不作响应
                if(fan_is_on){
                    if(Key_EdgeDetect(KeyIndex_KEY_CLICK) == KeyEdge_Rising){
                        fan_is_on = 0;//关风扇
                    }           
                }
                else{
                    if(KEY_GetClickTimes(KeyIndex_KEY_CLICK, 2)){                   //中键双击启动               
                        if(portc_is_on ||
                            SW6208_ReadCapacity() == 0 ||
                            SW6208_ReadVBAT() < VBAT_LOW
                            ) current_table = menu_table + 6;//跳转警告页面
                        else fan_is_on = 1;//开风扇
                    }
                } 
            }
            break;
        case H_OFF:
            if(Key_EdgeDetect(KeyIndex_KEY_UP) == KeyEdge_Rising) start_menu_highlight = H_FAN;
            if(Key_EdgeDetect(KeyIndex_KEY_LEFT) == KeyEdge_Rising) start_menu_highlight = H_MENU;
            if(Key_EdgeDetect(KeyIndex_KEY_CLICK) == KeyEdge_Rising){
                if(fan_is_on == fan_is_on_last){
                    sleep_cd = 0;
                    fan_is_on = 0;//关风扇，执行休眠
                }
            }
            break;
        case H_MENU:
            if(Key_EdgeDetect(KeyIndex_KEY_UP) == KeyEdge_Rising) start_menu_highlight = H_FAN;
            if(Key_EdgeDetect(KeyIndex_KEY_RIGHT) == KeyEdge_Rising) start_menu_highlight = H_OFF;
            if(Key_EdgeDetect(KeyIndex_KEY_CLICK) == KeyEdge_Rising) current_table = menu_table+ 1;
            break;
    }    
}
const uint8_t img_battery[] = {/*长，宽*/0x10,0x10, 0xC0,0x03,0x40,0x02,0xF0,0x0F,0x10,0x08,0x10,0x09,0x90,0x09,0x90,0x09,0xD0,0x0F,0xD0,0x0B,0xF0,0x0B,0x90,0x09,0x90,0x09,0x90,0x08,0x10,0x08,0x10,0x08,0xE0,0x07};
void Start_Menu_Draw(){    
    SSD1306_GotoXY(16, 0);//显示标题
    SSD1306_Puts("Handheld Fan", &Font_8x16, 1);     
    
    SSD1306_GotoXY(0, 16);//显示主页状态
    if(portc_is_on){//typeC口已连接
        if(start_menu_highlight == H_FAN) start_menu_highlight = H_OFF;
        if(SW6208_IsDischarging()) SSD1306_Puts("Discharging", &Font_8x16, 1);
        else if(SW6208_IsCharging()) SSD1306_Puts("Charging", &Font_8x16, 1);
        else SSD1306_Puts("Charged", &Font_8x16, 1);
    }
    else{//未连接
        SSD1306_Puts("FAN:", &Font_8x16, (start_menu_highlight == H_FAN)? 0 : 1);
        SSD1306_GotoXY(32, 16);
        SSD1306_Printf(&Font_8x16, 1, "%d%%", 100U* fanset/ 1024U); 
    }
    if(fan_is_on){//显示附加信息
        SSD1306_GotoXY(80, 16);
        SSD1306_Printf(&Font_6x8, 1, "%5drpm", rpm0);
        SSD1306_GotoXY(80, 24);
        SSD1306_Printf(&Font_6x8, 1, "%5drpm", rpm1);
    }
    else if(SW6208_IsCPortON()){
        SSD1306_GotoXY(88, 16);
        SSD1306_Puts((char*)SW6208_ReadProtocol(), &Font_8x16, 1);  
    }
    else{
        SSD1306_GotoXY(104, 16);
        SSD1306_Puts("OFF", &Font_8x16, 1);  
    }
    //系统功率
    float buspower;
    if(SW6208_ReadICharge() > SW6208_ReadIDischarge()) buspower = SW6208_ReadICharge();
    else buspower = SW6208_ReadIDischarge();
    buspower = buspower * SW6208_ReadVBUS() / 1000;
    SSD1306_GotoXY(0, 32);
    if(buspower>=10.0f) SSD1306_Printf(&Font_8x16, 1, "%.2fW",buspower);    
    else SSD1306_Printf(&Font_8x16, 1, "%.3fW",buspower);
    SSD1306_Image((uint8_t*)img_battery, 0, 80, 32);
    //电池电量
    SSD1306_GotoXY(96, 32);
    SSD1306_Printf(&Font_8x16, 1, "%d%% ", SW6208_ReadCapacity());  
    
    SSD1306_GotoXY(0, 48);
    SSD1306_Puts(" MENU ", &Font_8x16, (start_menu_highlight == H_MENU)? 0 : 1); 
    SSD1306_GotoXY(88, 48);
    SSD1306_Puts("SLEEP", &Font_8x16, (start_menu_highlight == H_OFF)? 0 : 1); 
}

/********************************次级跳转菜单**********************************/
const struct jump_menu_t jump_menu[] = {
    {"*Return",menu_table},
    {"*Setting",menu_table + 5},
    {"*System Stat",menu_table + 4},
    {"*Game Snake",menu_table + 7},
    {"*Burn Test",menu_table + 2},
    {"*About",menu_table + 3}
};
uint16_t jump_menu_top = sizeof(jump_menu)/sizeof(struct jump_menu_t);          //菜单总项数
uint16_t menu_index = 0;                                                        //当前选中项
uint8_t  menu_highlight = 0;                                                    //高亮项位置
uint8_t  box_height0 = 60*sizeof(struct jump_menu_t)/sizeof(jump_menu);         //滚动条高度

void Main_Menu_Key(){
    if(Key_EdgeDetect(KeyIndex_KEY_CLICK) == KeyEdge_Rising) 
        current_table = jump_menu[menu_index].targetmenu;
    if(KEY_GetDASClick(KeyIndex_KEY_UP)){
        if(menu_highlight && menu_index != jump_menu_top - 1) 
            menu_highlight--;
        if(menu_index) 
            menu_index--;
        else{
            menu_index = jump_menu_top - 1;
            menu_highlight = 1;
        }
    }
    if(KEY_GetDASClick(KeyIndex_KEY_DOWN)){
        if(menu_highlight != 1 && menu_index) 
            menu_highlight++;
        if(menu_index != jump_menu_top - 1) 
            menu_index++;
        else{
            menu_index = 0;
            menu_highlight = 0;
        }
    }
}
void Main_Menu_Draw(){ 
    uint8_t i,j;
    //绘制滚动条
    SSD1306_DrawLine(0,0,0,63,1,0);
    SSD1306_DrawLine(3,0,3,63,1,0);
    SSD1306_DrawLine(0,0,3,0,1,0);
    SSD1306_DrawLine(0,63,3,63,1,0);
    SSD1306_DrawLine(1,2+menu_index*box_height0,1,1+(menu_index+1)*box_height0,1,0);
    SSD1306_DrawLine(2,2+menu_index*box_height0,2,1+(menu_index+1)*box_height0,1,0);
    //根据menu_index与menu_highlight决定显示位置
    if(menu_index == 0) j = 0;//列表顶
    else if(menu_index == jump_menu_top - 1) j = jump_menu_top - 4;//列表底
    else j = menu_index -1 - menu_highlight;
    for(i = j;i < j+4;i++){
        SSD1306_GotoXY(8, 16* (i - j));
        SSD1306_Puts(jump_menu[i].text, &Font_8x16, (i == menu_index)? 0 : 1); 
    }
}
    
void Any_Key(){
    if(Key_EdgeDetect(KeyIndex_KEY_CLICK)   == KeyEdge_Rising
    || Key_EdgeDetect(KeyIndex_KEY_LEFT)    == KeyEdge_Rising
    || Key_EdgeDetect(KeyIndex_KEY_RIGHT)   == KeyEdge_Rising
    || Key_EdgeDetect(KeyIndex_KEY_UP)      == KeyEdge_Rising
    || Key_EdgeDetect(KeyIndex_KEY_DOWN)    == KeyEdge_Rising
    ) current_table = menu_table + 1;
}//按任意键离开
void Burn_Test_Draw(){
    SSD1306_Fill(1);
}//烧屏测试界面只需要点亮全屏

void About_Page_Draw(){
    SSD1306_GotoXY(0, 0);
    SSD1306_Puts("Handheld Fan by TKWTL", &Font_6x8, 1); 
    SSD1306_GotoXY(0, 16);
    SSD1306_Puts("V0.1 Beta 2025/02/02", &Font_6x8, 1); 
    SSD1306_GotoXY(0, 24);
    SSD1306_Puts("Data on OSHWHUB:", &Font_6x8, 1); 
    SSD1306_GotoXY(0, 32);
    SSD1306_Puts("https://oshwhub.com/", &Font_6x8, 1); 
    SSD1306_GotoXY(0, 40);
    SSD1306_Puts(" jeremy_li/shou-chi-", &Font_6x8, 1); 
    SSD1306_GotoXY(0, 48);
    SSD1306_Puts(" pu-li-shan", &Font_6x8, 1); 
    SSD1306_GotoXY(0, 56);
    SSD1306_Puts("Press Any Key to Quit", &Font_6x8, 1); 
}

/****************************信息显示页面**************************************/
//定义每行的信息显示函数，一行显示不超过20字符
void Quit_Show()            {SSD1306_Puts("Center Click to Quit", &Font_6x8, 1);}
void Port_Vol_Show()        {SSD1306_Printf(&Font_6x8, 1, "Port Voltage:%.3fV",SW6208_ReadVBUS());}
void Batt_Vol_Show()        {SSD1306_Printf(&Font_6x8, 1, "Batt Voltage:%.3fV",SW6208_ReadVBAT());}
void Chip_Temp_Show()       {SSD1306_Printf(&Font_6x8, 1, "SW6208 Temp:%.2f'C",SW6208_ReadTCHIP());}
void NTC_Vol_Show()         {SSD1306_Printf(&Font_6x8, 1, "NTC Voltage:%.2fmV",SW6208_ReadVNTC());}
void Charge_Curr_Show()     {SSD1306_Printf(&Font_6x8, 1, "Charge Curr:%.1fmA",SW6208_ReadICharge());}
void Discharge_Curr_Show()  {SSD1306_Printf(&Font_6x8, 1, "Dischg Curr:%.1fmA",SW6208_ReadIDischarge());}
void CapEnergy_Show()       {SSD1306_Printf(&Font_6x8, 1, "Batt Energy:%.3fWh",SW6208_ReadCapValue());}
void Protocol_Show()        {SSD1306_Printf(&Font_6x8, 1, "Protocol: %s",SW6208_ReadProtocol());}
void Millis_Show()          {SSD1306_Printf(&Font_6x8, 1, "%dms After Reset",millis* 5);}
void Sleep_Countdown_Show() {SSD1306_Printf(&Font_6x8, 1, "%dms Before Sleep",sleep_cd* 5);}
void Sleep_Time_Show()      {SSD1306_Printf(&Font_6x8, 1, "No op %dms Sleep",sleep_time* 5);}
void Contrast_Show()        {SSD1306_Printf(&Font_6x8, 1, "OLED Contrast:%d",indensity);}
void VCC_Show()             {SSD1306_Printf(&Font_6x8, 1, "MCU Vcc:%.3fV",vcc_voltage);}
void TJunction_Show()       {SSD1306_Printf(&Font_6x8, 1, "MCU Temp:%.1f'C",junction_temp);}
void Chip_UID_Show0()       {SSD1306_Printf(&Font_6x8, 1, "MCU UID: 0x%X", LL_GetUID_Word0());}
void Chip_UID_Show1()       {SSD1306_Printf(&Font_6x8, 1, "     %X%X", LL_GetUID_Word1(),LL_GetUID_Word2());}
void Flash_Size_Show()      {SSD1306_Printf(&Font_6x8, 1, "Flash size:%ldByte", LL_GetFlashSize());}
void Sram_Size_Show()       {SSD1306_Printf(&Font_6x8, 1, "Sram size :%ld Byte", LL_GetSramSize());}
void (*ptitle[])(void) = {//在这里登记stat页面要显示的信息的实现函数名，按初始化顺序分配，0列为空白
    Quit_Show,
    0,
    Port_Vol_Show,
    Batt_Vol_Show,
    Chip_Temp_Show,
    NTC_Vol_Show,
    Charge_Curr_Show,
    Discharge_Curr_Show,
    CapEnergy_Show,
    Protocol_Show,
    0,
    Millis_Show,
    Sleep_Countdown_Show,
    Sleep_Time_Show,
    Contrast_Show,
    0,
    VCC_Show,
    TJunction_Show,
    Chip_UID_Show0,
    Chip_UID_Show1,
    Flash_Size_Show,
    Sram_Size_Show
};                                                                              //展示项函数指针数组
uint16_t stat_num_p = 0;                                                        //展示项指示变量
uint16_t stat_num_top = sizeof(ptitle)/sizeof(void(*)()) - 8;                   //最大滚动次数
uint16_t box_height = 60/(sizeof(ptitle)/sizeof(void(*)()) - 7);                //滚动条高度
void Stat_Page_Key(){
    if(KEY_GetDASClick(KeyIndex_KEY_UP)){                                       //上一项
        if(stat_num_p) stat_num_p--;
        else stat_num_p = stat_num_top;
    }
    if(Key_EdgeDetect(KeyIndex_KEY_CLICK) == KeyEdge_Rising){                   //退出
        stat_num_p = 0;
        current_table = menu_table + 1;
    }
    if(KEY_GetDASClick(KeyIndex_KEY_DOWN)){                                     //下一项
        if(stat_num_p == stat_num_top) stat_num_p = 0;
        else stat_num_p++;
    }
}
void Stat_Page_Draw(){
    uint8_t i;
    //绘制滚动条
    SSD1306_DrawLine(0,0,0,63,1,0);
    SSD1306_DrawLine(3,0,3,63,1,0);
    SSD1306_DrawLine(0,0,3,0,1,0);
    SSD1306_DrawLine(0,63,3,63,1,0);
    SSD1306_DrawLine(1,2+stat_num_p*box_height,1,2+(stat_num_p+1)*box_height,1,0);
    SSD1306_DrawLine(2,2+stat_num_p*box_height,2,2+(stat_num_p+1)*box_height,1,0);
    //绘制信息项
    for(i = 0;i < 8;i++){
        if((*ptitle[i + stat_num_p]) != 0){
            SSD1306_GotoXY(8, 8*i);
            (*ptitle[i + stat_num_p])();
        }
    }
}

/******************************设置页面****************************************/
const struct Setting_option_t setting_options[] = {
    {0,"Return",BOOL_SET,1,0,0,0},//第一个必须为返回项且如此定义
    {&indensity,"Screen Light",VALUE_SET,255,1,2,IMMEDIATE_TRIG},
    {&sleepable,"Auto Sleep",BOOL_SET,1,0,0,0},
    {&sleep_time,"Sleep Time",VALUE_SET,60000,500,20,IMMEDIATE_TRIG},
    {&portc_discharge,"USB Discharge",BOOL_SET,1,0,0,0},
    {&portc_charge,"USB Charge",BOOL_SET,1,0,0,0},
    {&protocol_pd_en,"PD Protocol",BOOL_SET,1,0,0,0},
    {&protocol_scp_en,"SCP Protocol",BOOL_SET,1,0,0,0},
    {&protocol_fcp_en,"FCP Protocol",BOOL_SET,1,0,0,0},
    {&protocol_afc_en,"AFC Protocol",BOOL_SET,1,0,0,0},
    {&protocol_pe_en,"MTK PE Protocol",BOOL_SET,1,0,0,0},
    {&system_reset,"System Reset",BOOL_SET,1,0,0,0}
};
uint8_t setting_num = sizeof(setting_options)/sizeof(struct Setting_option_t);  //设置列表项数
uint8_t setting_index = 0;                                                      //当前设置项指针
uint8_t  box_height1 = 60*sizeof(struct Setting_option_t)/sizeof(setting_options);//滚动条高度

void Setting_Page_Key(){
    if(setting_options[setting_index].setting_type == BOOL_SET){                //二值类型
        if(Key_EdgeDetect(KeyIndex_KEY_CLICK) == KeyEdge_Rising){
            if(setting_options[setting_index].parameter == 0) current_table = menu_table + 1;//碰到return
            else{
                if(*setting_options[setting_index].parameter == setting_options[setting_index].top_value) *setting_options[setting_index].parameter = setting_options[setting_index].bottom_value;
                else *setting_options[setting_index].parameter = setting_options[setting_index].top_value;
            }
        }
    }
    else if(setting_options[setting_index].setting_type == VALUE_SET){          //整数类型
        uint8_t sign = 0;
        switch(setting_options[setting_index].trigger_type){
            case SINGLE_TRIG:
                if(Key_EdgeDetect(KeyIndex_KEY_LEFT) == KeyEdge_Rising) sign = 127;
                else if(Key_EdgeDetect(KeyIndex_KEY_RIGHT) == KeyEdge_Rising) sign = 1;
                break;
            case IMMEDIATE_TRIG:
                if(KEY_LEFT_GetIO()) sign = 127;
                else if(KEY_RIGHT_GetIO()) sign = 1;
                break;
            case DELAY_TRIG:
                if(KEY_GetState(KeyIndex_KEY_LEFT) == KeyState_LongPress) sign = 127;
                else if(KEY_GetState(KeyIndex_KEY_RIGHT) == KeyState_LongPress) sign = 1;
                break;
        }        
        if(sign == 1){
            if(setting_options[setting_index].top_value - *setting_options[setting_index].parameter < setting_options[setting_index].setting_step) *setting_options[setting_index].parameter = setting_options[setting_index].top_value;
            else *setting_options[setting_index].parameter += setting_options[setting_index].setting_step;
        }
        else if(sign == 127){
            if(*setting_options[setting_index].parameter - setting_options[setting_index].bottom_value < setting_options[setting_index].setting_step) *setting_options[setting_index].parameter = setting_options[setting_index].bottom_value;
            else *setting_options[setting_index].parameter -= setting_options[setting_index].setting_step;
        }
    }
    //项间切换
    if(KEY_GetDASClick(KeyIndex_KEY_UP)){
        if(setting_index) setting_index--;
        else setting_index = setting_num - 1;
    }
    else if(KEY_GetDASClick(KeyIndex_KEY_DOWN)){
        if(setting_index == setting_num - 1) setting_index = 0;
        else setting_index++;
    }
}
void Setting_Title_Draw(){
    if(setting_options[setting_index].setting_type == BOOL_SET){                //二值类型
        if(*setting_options[setting_index].parameter == setting_options[setting_index].top_value) SSD1306_Puts(" >    ON    <", &Font_8x16, 1);
        else SSD1306_Puts(" >    OFF    <", &Font_8x16, 1);
    }
    else if(setting_options[setting_index].setting_type == VALUE_SET){          //整数类型
        SSD1306_Printf(&Font_8x16, 1, "-<  %5d    >+", *setting_options[setting_index].parameter);
    }
}
void Setting_Page_Draw(){    
    uint8_t i,j;
    //绘制滚动条
    SSD1306_DrawLine(0,0,0,63,1,0);
    SSD1306_DrawLine(3,0,3,63,1,0);
    SSD1306_DrawLine(0,0,3,0,1,0);
    SSD1306_DrawLine(0,63,3,63,1,0);
    SSD1306_DrawLine(1,2+setting_index*box_height1,1,1+(setting_index+1)*box_height1,1,0);
    SSD1306_DrawLine(2,2+setting_index*box_height1,2,1+(setting_index+1)*box_height1,1,0);
    //绘制菜单项，除return与最后一项外，其余数值设置都在中心
    if(setting_index == 0){//列表顶
        for(i = 0;i < 4;i++){
            SSD1306_GotoXY(8, 16*i);
            SSD1306_Puts((char *)setting_options[i].text, &Font_8x16, (i == 0)? 0 : 1); 
        }
    }
    else if(setting_index == setting_num - 1){//列表底
        for(i = 0;i < 3;i++){
            SSD1306_GotoXY(8, 16*i);
            SSD1306_Puts((char *)setting_options[setting_num - 3 + i].text, &Font_8x16, (i == 2)? 0 : 1); 
        }
        SSD1306_GotoXY(8, 48);
        Setting_Title_Draw();
    }
    else{
        SSD1306_GotoXY(8, 0);
        SSD1306_Puts((char *)setting_options[setting_index - 1].text, &Font_8x16, 1);
        SSD1306_GotoXY(8, 16);
        SSD1306_Puts((char *)setting_options[setting_index].text, &Font_8x16, 0);
        SSD1306_GotoXY(8, 32);
        Setting_Title_Draw();
        SSD1306_GotoXY(8, 48);
        SSD1306_Puts((char *)setting_options[setting_index + 1].text, &Font_8x16, 1);
    }
}

void Delay_Quit_Key(){
    static uint8_t first_enter = 1;
    static uint32_t endmillis;
    if(first_enter){//进入
        first_enter = 0;
        endmillis = millis + 200;//200tick = 1秒后退出
    }
    if(endmillis < millis){//退出
        first_enter = 1;
        current_table = menu_table;
    }
}

void Warning_Page_Draw(){//仅在试图打开风扇时进入
    if(SW6208_ReadCapacity() == 0 || SW6208_ReadVBAT() < VBAT_LOW){//低电量情况
        SSD1306_GotoXY(16, 24);
        SSD1306_Puts("Battery Low!", &Font_8x16, 1);
    }
    else if(portc_is_on){
        SSD1306_GotoXY(0, 24);
        if(SW6208_IsCharging()) SSD1306_Puts("  USB Charging!", &Font_8x16, 1);//C口充电情况
        else if(SW6208_IsDischarging()) SSD1306_Puts("USB Discharging!", &Font_8x16, 1);//C口放电情况
    }
    else{//其他情况
        SSD1306_GotoXY(8, 24);
        SSD1306_Puts("Unknown Error!", &Font_8x16, 1);
    }
}

const struct Menutable_t menu_table[] = {                                       //菜单项结构体数组
    {0,"Start Menu",Start_Menu_Key,Start_Menu_Draw,1},
    {1,"Main Menu",Main_Menu_Key,Main_Menu_Draw,1},
    {2,"Burn Test",Any_Key,Burn_Test_Draw,1},
    {3,"About Page",Any_Key,About_Page_Draw,0},
    {4,"Device Statistic",Stat_Page_Key,Stat_Page_Draw,0},
    {5,"Setting",Setting_Page_Key,Setting_Page_Draw,0},
    {6,"Warning",Delay_Quit_Key,Warning_Page_Draw,0},
    {7,"Snake",Game_Snake_Key,Game_Snake_Draw,0}
};
const struct Menutable_t *current_table = menu_table;//修改此行来改变起始页

