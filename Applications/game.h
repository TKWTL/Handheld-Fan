#ifndef __GAME_H__
#define __GAME_H__

#ifdef __cplusplus
extern "C" {
#endif
    
    #include "fifo.h"
    #include "menu.h"
    
    /*
    /贪吃蛇数据结构：一个FIFO存储蛇节点坐标，偶数x，奇数y
    /按键按下后储存起来，蛇头移动时使用这个值，
    /有墙和无墙两种模式
    */
    /*
    /地图：128*64 31*15个3*3方格
    /方格间距1px
    /零点位于左上角
    */
    #define MAP_WIDTH   31
    #define MAP_HEIGHT  13
    #define MAP_ORIGIN_X    3
    #define MAP_ORIGIN_Y    11
    #define SNAKE_FIFO_SIZE 400*2
    #define WIN_SCORE   365                                                     //判定胜利分数
    
    typedef enum{
        Game_Run,//游戏主界面
        Game_Pause,//暂停
        Game_Over,//游戏结束
        Game_Win//吃的太多判定胜利
    }Game_Status_t;
    
    typedef enum{
        State_Resume,
        State_Exit,
        State_Border,
        State_Speed,
        State_SpeedInc
    }Game_Menu_t;
    
    //供外部调用的接口函数
    void Game_Snake_Key();
    void Game_Snake_Draw();
    
#ifdef __cplusplus
}
#endif
#endif