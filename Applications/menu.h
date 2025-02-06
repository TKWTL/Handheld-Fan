#ifndef __MENU_H__
#define __MENU_H__

#ifdef __cplusplus
extern "C" {
#endif
/******************************包含用户库**************************************/
    #include "debounce_key.h"                                                   //按键读取依赖库
    #include "ssd1306.h"                                                        //显示依赖库
    #include "sw6208.h"
    #include "game.h"


struct Menutable_t{                                                             //菜单大项注册结构体
    uint8_t index;                                                              //菜单标号（无实意）
    char *name;                                                                 //菜单名称（无实意）
    void (*key)(void);                                                          //按键处理函数指针
    void (*draw)(void);                                                         //显示函数指针
    char sleepable;                                                             //当前菜单是否允许休眠 0：不允许休眠
};                 

struct jump_menu_t {                                                            //多项跳转类菜单 菜单项定义结构体
    char *text;                                                                 //跳转表显示文本
    const struct Menutable_t *targetmenu;                                       //跳转目标菜单地址
};

typedef enum {
    BOOL_SET = 0,                                                               //二值类变量设置
    VALUE_SET                                                                   //值域类变量设置
}Setting_type_e;

typedef enum {
    SINGLE_TRIG = 0,                                                            //单次触发，仅按下时修改一次值
    IMMEDIATE_TRIG,                                                             //立即触发，按下可以一直修改值
    DELAY_TRIG                                                                  //延时触发，按下到达长按时间才开始修改
}Trigger_type_e;

struct Setting_option_t {                                                       //参数设置型菜单设置项结构体
    volatile uint16_t *parameter;                                               //参数本体
    const char *text;                                                           //设置项显示
    Setting_type_e setting_type;                                                //设置项类型
    uint16_t top_value;                                                         //最大值，对于布尔型，按中键在两个值之间切换
    uint16_t bottom_value;                                                      //最小值，对于值型，减小到该值后不再减小
    uint16_t setting_step;                                                      //切换步长，仅对值型生效
    Trigger_type_e trigger_type;                                                //按键触发方式，仅对值型生效
};

extern const struct Menutable_t menu_table[];
extern const struct Menutable_t *current_table;

#ifdef __cplusplus
}
#endif
#endif
    