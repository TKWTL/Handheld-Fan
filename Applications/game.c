#include "game.h"


FIFO_Create(FIFO_word_t, snake_fifo, SNAKE_FIFO_SIZE);//调用宏定义储存蛇体坐标信息的队列
uint16_t Snake_Score = 0;//分数
uint16_t Snake_Top = 0;//最高分
uint8_t Snake_Speed = 5;//蛇速度
uint8_t Speed_AutoInc = 0;//蛇速度自动增加
uint8_t Snake_Dir;//蛇方向，0向上，1向右，2向下，3向左，4蛇不移动（用于进入游戏）
uint8_t Map_Border = 1;//地图边界开关
uint8_t fruit_x,fruit_y;

Game_Status_t Snake_Game_Status = Game_Run;//游戏状态枚举变量
Game_Menu_t Snake_Game_Menu = State_Resume;//菜单项枚举变量

//蛇身碰撞检测
uint8_t Coll_Detect(uint8_t x, uint8_t y){
    uint16_t i, j = GetFIFOLength_Word(&snake_fifo)/2;
    for(i = 0;i < j;i++){
        if(FIFO_QueryfromHead_Word_NoCheck(&snake_fifo, i*2) == x && FIFO_QueryfromHead_Word_NoCheck(&snake_fifo, i*2+1) == y){
            return 1;//有碰撞
        }
    }
    return 0;//无碰撞
}

//水果生成
void Game_Fruit_Spawn(){
    do{
        fruit_x = rand()% MAP_WIDTH;
        fruit_y = rand()% MAP_HEIGHT;
    }while(Coll_Detect(fruit_x, fruit_y) == 1);
}

//填充蛇的初始位置：3格长，位置与方向随机
void Game_Snake_Init(){
    uint8_t init_x = 2+ rand()% (MAP_WIDTH- 4);
    uint8_t init_y = 2+ rand()% (MAP_HEIGHT- 4);
    uint8_t init_dir = rand()%4;
    int8_t x_offset = (init_dir&0x01U)? ((init_dir&0x02U)? 1:-1):0;
    int8_t y_offset = (init_dir&0x01U)? 0:((init_dir&0x02U)? 1:-1);
    FIFO_In_Word(&snake_fifo, init_y+ y_offset*2);
    FIFO_In_Word(&snake_fifo, init_x+ x_offset*2);//尾，数据结构中处于FIFO的尾部，最先被删除
    FIFO_In_Word(&snake_fifo, init_y+ y_offset);
    FIFO_In_Word(&snake_fifo, init_x+ x_offset);//身
    FIFO_In_Word(&snake_fifo, init_y);
    FIFO_In_Word(&snake_fifo, init_x);//头
    Snake_Dir = 4;
    Game_Fruit_Spawn();
    Snake_Score = 0;//重置分数
    if(Speed_AutoInc) Snake_Speed = 1;//速度自增模式下重新开始时重置蛇的速度
}

void Game_Snake_Key(){
    static uint8_t first_enter = 1;
    static uint32_t step_millis;
    uint8_t trgt_x, trgt_y;
    uint8_t neck_dir,coll = 0;
    
    if(first_enter){
        srand(millis+ vref_conv+ temp_conv);//随机种子来源于进入游戏的时间与ADC采样值
        Game_Snake_Init();
        Snake_Game_Status = Game_Run;
        Snake_Game_Menu = State_Resume;
        first_enter = 0;
    }
    
    if(Snake_Game_Status == Game_Run){//正常游戏 
        //改变蛇方向
        //判断蛇头无法向哪个方向移动
        trgt_x = FIFO_QueryfromHead_Word_NoCheck(&snake_fifo, 0);
        trgt_y = FIFO_QueryfromHead_Word_NoCheck(&snake_fifo, 1);
        if(FIFO_QueryfromHead_Word_NoCheck(&snake_fifo, 2) == trgt_x){//第二节在蛇头同一列
            if(FIFO_QueryfromHead_Word_NoCheck(&snake_fifo, 3) > trgt_y){
                if(FIFO_QueryfromHead_Word_NoCheck(&snake_fifo, 3) == trgt_y+ 1) neck_dir = 2;
                else neck_dir = 0;
            }
            else{
                if(FIFO_QueryfromHead_Word_NoCheck(&snake_fifo, 3) == trgt_y- 1) neck_dir = 0;
                else neck_dir = 2;
            }
        }
        else{
            if(FIFO_QueryfromHead_Word_NoCheck(&snake_fifo, 2) > trgt_x){
                if(FIFO_QueryfromHead_Word_NoCheck(&snake_fifo, 2) == trgt_x+ 1) neck_dir = 1;
                else neck_dir = 3;
            }
            else{
                if(FIFO_QueryfromHead_Word_NoCheck(&snake_fifo, 2) == trgt_x- 1) neck_dir = 3;
                else neck_dir = 1;
            }
        }
        //按键改变方向，不能控制回头
        if     (Key_EdgeDetect(KeyIndex_KEY_UP) == KeyEdge_Rising && neck_dir != 0) Snake_Dir = 0;
        else if(Key_EdgeDetect(KeyIndex_KEY_RIGHT) == KeyEdge_Rising && neck_dir != 1) Snake_Dir = 1;
        else if(Key_EdgeDetect(KeyIndex_KEY_DOWN) == KeyEdge_Rising && neck_dir != 2) Snake_Dir = 2;
        else if(Key_EdgeDetect(KeyIndex_KEY_LEFT) == KeyEdge_Rising && neck_dir != 3) Snake_Dir = 3;
        else if(Key_EdgeDetect(KeyIndex_KEY_CLICK) == KeyEdge_Rising) Snake_Game_Status = Game_Pause;
        
        if(millis > step_millis && Snake_Dir != 4){//蛇移动
            step_millis = millis+ 750/ (Snake_Speed+ 4);//控制速度
            
            switch(Snake_Dir){//控制蛇的移动/计算蛇下一格的坐标，用于移动，吃水果与碰撞检测
                case 0://向上移动，x不变，y减小
                    if(trgt_y) trgt_y--;
                    else if(Map_Border) coll = 1;
                    else trgt_y = MAP_HEIGHT- 1;
                    break;
                case 1://向右移动，y不变，x增大
                    if(trgt_x < MAP_WIDTH- 1) trgt_x++;
                    else if(Map_Border) coll = 1;
                    else trgt_x = 0;
                    break;
                case 2://向下移动，x不变，y增大
                    if(trgt_y < MAP_HEIGHT- 1) trgt_y++;
                    else if(Map_Border) coll = 1;
                    else trgt_y = 0;
                    break;
                case 3://向左移动，y不变，x减小
                    if(trgt_x) trgt_x--;
                    else if(Map_Border) coll = 1;
                    else trgt_x = MAP_WIDTH- 1;
                    break;
                default:
                    break;
            }
            //撞蛇判断，吃水果判断
            if(coll || Coll_Detect(trgt_x, trgt_y)){//游戏结束了嗷
                Snake_Game_Status = Game_Over;
                if(Snake_Top < Snake_Score) Snake_Top = Snake_Score;
                FIFO_DeletefromTail_Word(&snake_fifo, GetFIFOLength_Word(&snake_fifo));
                return;
            }
            else{//移动蛇
                FIFO_In_Word(&snake_fifo, trgt_y);
                FIFO_In_Word(&snake_fifo, trgt_x);
                if(trgt_y == fruit_y && trgt_x == fruit_x){
                    Snake_Score++;
                    if(Snake_Score == WIN_SCORE){
                        Snake_Game_Status = Game_Win;
                        Snake_Top = Snake_Score;
                        FIFO_DeletefromTail_Word(&snake_fifo, GetFIFOLength_Word(&snake_fifo));
                        return;
                    }
                    if(Speed_AutoInc && Snake_Speed != 10) Snake_Speed++;//速度自增
                    Game_Fruit_Spawn();
                }
                else FIFO_DeletefromTail_Word(&snake_fifo, 2);
            }
        }
    }
    else{
        switch(Snake_Game_Menu){
            case State_Resume:
                if(Key_EdgeDetect(KeyIndex_KEY_CLICK) == KeyEdge_Rising){
                    step_millis = millis+ 750/ (Snake_Speed+ 4);
                    if(Snake_Game_Status != Game_Pause) Game_Snake_Init();
                    Snake_Game_Status = Game_Run;
                }
                else if(KEY_GetDASClick(KeyIndex_KEY_UP)) Snake_Game_Menu = State_SpeedInc;
                else if(KEY_GetDASClick(KeyIndex_KEY_DOWN)) Snake_Game_Menu = State_Exit;
                break;
            case State_Exit:
                if(Key_EdgeDetect(KeyIndex_KEY_CLICK) == KeyEdge_Rising) current_table = menu_table + 1;
                else if(KEY_GetDASClick(KeyIndex_KEY_UP)) Snake_Game_Menu = State_Resume;
                else if(KEY_GetDASClick(KeyIndex_KEY_DOWN)) Snake_Game_Menu = State_Border;
                break;
            case State_Border:
                if(Key_EdgeDetect(KeyIndex_KEY_CLICK) == KeyEdge_Rising) Map_Border = Map_Border? 0 : 1;
                else if(KEY_GetDASClick(KeyIndex_KEY_UP)) Snake_Game_Menu = State_Exit;
                else if(KEY_GetDASClick(KeyIndex_KEY_DOWN)) Snake_Game_Menu = State_Speed;
                break;
            case State_Speed:
                if(KEY_GetDASClick(KeyIndex_KEY_LEFT) && Snake_Speed) Snake_Speed--;
                else if(KEY_GetDASClick(KeyIndex_KEY_RIGHT) && Snake_Speed != 10) Snake_Speed++;
                else if(KEY_GetDASClick(KeyIndex_KEY_UP)) Snake_Game_Menu = State_Border;
                else if(KEY_GetDASClick(KeyIndex_KEY_DOWN)) Snake_Game_Menu = State_SpeedInc;
                break;
            case State_SpeedInc:
                if(Key_EdgeDetect(KeyIndex_KEY_CLICK) == KeyEdge_Rising) Speed_AutoInc = Speed_AutoInc? 0 : 1;
                else if(KEY_GetDASClick(KeyIndex_KEY_UP)) Snake_Game_Menu = State_Speed;
                else if(KEY_GetDASClick(KeyIndex_KEY_DOWN)) Snake_Game_Menu = State_Resume;
                break;
        }
    }
    if(current_table == menu_table+ 1){//退出游戏
        first_enter = 1;
        FIFO_DeletefromTail_Word(&snake_fifo, GetFIFOLength_Word(&snake_fifo));
    }
}

void Game_Snake_Draw(){
    uint16_t snake_length, i, j;
    uint8_t curr_x,curr_y;
    
    if(Snake_Game_Status == Game_Run){//正常游戏        
        SSD1306_GotoXY(1, 0);//显示分数
        SSD1306_Printf(&Font_6x8, 1, "Score:%d", Snake_Score);    
        SSD1306_GotoXY(72, 0);
        SSD1306_Printf(&Font_6x8, 1, "Top:%d", Snake_Top);
        //绘制地图边界
        SSD1306_DrawLine(1,   9,  1,   63, 1, !Map_Border);
        SSD1306_DrawLine(1,   9,  127, 9,  1, !Map_Border);
        SSD1306_DrawLine(1,   63, 127, 63, 1, !Map_Border);
        SSD1306_DrawLine(127, 9,  127, 63, 1, !Map_Border);
        //画水果
        SSD1306_DrawPixel(MAP_ORIGIN_X+ fruit_x*4+ 1,MAP_ORIGIN_Y+ fruit_y*4, 1);
        SSD1306_DrawPixel(MAP_ORIGIN_X+ fruit_x*4,   MAP_ORIGIN_Y+ fruit_y*4+ 1, 1);
        SSD1306_DrawPixel(MAP_ORIGIN_X+ fruit_x*4+ 2,MAP_ORIGIN_Y+ fruit_y*4+ 1, 1);
        SSD1306_DrawPixel(MAP_ORIGIN_X+ fruit_x*4+ 1,MAP_ORIGIN_Y+ fruit_y*4+ 2, 1);
        //画蛇
        snake_length = GetFIFOLength_Word(&snake_fifo)/2;//蛇长
        //画头
        curr_x = 4*FIFO_QueryfromHead_Word_NoCheck(&snake_fifo, 0);
        curr_y = 4*FIFO_QueryfromHead_Word_NoCheck(&snake_fifo, 1);
        for(j = 0;j < 9;j++) SSD1306_DrawPixel(curr_x+ MAP_ORIGIN_X+ j/3,curr_y+ MAP_ORIGIN_Y+ j%3, j == 4 ? 0 : 1);
        //身体
        for(i = 1;i < snake_length;i++){
            curr_x = 4*FIFO_QueryfromHead_Word_NoCheck(&snake_fifo, i*2);
            curr_y = 4*FIFO_QueryfromHead_Word_NoCheck(&snake_fifo, i*2+1);
            for(j = 0;j < 9;j++) SSD1306_DrawPixel(curr_x+ MAP_ORIGIN_X+ j/3,curr_y+ MAP_ORIGIN_Y+ j%3, 1);
        }
    }
    else{//暂停、结束、胜利
        //标题界面
        if(Snake_Game_Status == Game_Pause){SSD1306_GotoXY(40,0); SSD1306_Puts("Paused", &Font_8x16, 1);}
        else if(Snake_Game_Status == Game_Over) {SSD1306_GotoXY(24,0); SSD1306_Puts("Game Over!", &Font_8x16, 1);}
        else if(Snake_Game_Status == Game_Win) {SSD1306_GotoXY(32,0); SSD1306_Puts("You Win!", &Font_8x16, 1);}
        //分数
        SSD1306_GotoXY(1, 16);
        SSD1306_Printf(&Font_6x8, 1, "Score:%d", Snake_Score);    
        SSD1306_GotoXY(72, 16);
        SSD1306_Printf(&Font_6x8, 1, "Top:%d", Snake_Top);
        SSD1306_GotoXY(1, 24);
        if(Snake_Game_Status == Game_Pause) SSD1306_Puts(" Resume ", &Font_6x8, Snake_Game_Menu == State_Resume? 0:1);
        else SSD1306_Puts(" Restart ", &Font_6x8, Snake_Game_Menu == State_Resume? 0:1);
        SSD1306_GotoXY(1, 32);
        SSD1306_Puts(" Exit ", &Font_6x8, Snake_Game_Menu == State_Exit? 0:1);
        SSD1306_GotoXY(1, 40);
        SSD1306_Puts(" Border:", &Font_6x8, Snake_Game_Menu == State_Border? 0:1);
        SSD1306_GotoXY(1, 48);
        SSD1306_Puts(" Speed:", &Font_6x8, Snake_Game_Menu == State_Speed? 0:1);
        SSD1306_GotoXY(1, 56);
        SSD1306_Puts(" Speed Increase:", &Font_6x8, Snake_Game_Menu == State_SpeedInc? 0:1);
        
        SSD1306_GotoXY(109, 40);
        if(Map_Border) SSD1306_Puts("ON", &Font_6x8, 1);
        else SSD1306_Puts("OFF", &Font_6x8, 1);
        SSD1306_GotoXY(109, 48); SSD1306_Printf(&Font_6x8, 1, "%d", Snake_Speed);
        SSD1306_GotoXY(109, 56);
        if(Speed_AutoInc) SSD1306_Puts("ON", &Font_6x8, 1);
        else SSD1306_Puts("OFF", &Font_6x8, 1);
    }
}
