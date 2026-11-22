#include "show.h"
#include "OLEDSHOW.h"   
#include "Serial.h"
#include "DHT11.h"
#include "DHT11_2.h"    
#include "YFS401.h"
#include "Motor.h"
#include "Solar.h"      
#include "SG90.h"       
#include "BlindMotor.h" 
#include "Sonar.h"      
#include "Delay.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

extern const uint8_t BMP_ICON[]; 

#define PUMP_POWER_LOW      20   //通过yb661 vm接12v 进行pwm分频   
#define PUMP_POWER_MID      35    
#define PUMP_POWER_HIGH     45   

// ==================== 全局状态变量 ====================
static uint8_t  s_SystemStarted = 0; 

static int      s_Temp;              
static uint8_t  s_Humi;
static uint8_t  s_WaterLevel = 0;    
static uint8_t  s_LightPercent;      
static uint32_t s_Flow, s_Total;
static uint8_t  s_PumpMode = 0;   
static uint8_t  s_LEDBright = 0;  
static uint8_t  s_LedMode = 0;       
static uint8_t  s_FanSpeed = 0;      
static uint8_t  s_DoorAngle = 0;     
static char     s_CurtainState[10] = "STOP"; 
static uint8_t  s_ShowPage = 0;      
static char     s_LastRx[17] = "None"; 
static char     s_LastTx[17] = "None"; 
static uint16_t s_SonarDist = 0;     // 保存超声波测得的原始距离

// ==================== 自动化与定时控制变量 ====================
static uint16_t s_CurtainTargetTicks = 0; 
static uint16_t s_CurtainRunTicks = 0;    

// 【新增】：储水控制状态变量
static uint8_t  s_AutoFillActive = 0;     // 自动储水状态 (0:关, 1:工作中)
static uint8_t  s_TargetWaterLevel = 0;   // 目标水位百分比 (0-100)
static uint16_t s_FillConfirmTicks = 0;   // 延时防抖确认计时器 (坚持一下)

// ==================== 警报状态控制变量 ====================
static uint8_t  s_AlarmState = 0;     
static uint16_t s_AlarmTicks = 0;     
static int8_t   s_BreathDir = 1;      
static int16_t  s_BreathVal = 0;      

static void _Show_ControlAutoLogic(void);

void Show_Init(void)
{
    OLEDSHOW_Init();            
    OLEDSHOW_ShowBMP(0, 0, 64, 64, BMP_ICON, 1); 
    Delay_ms(2000);             
    OLEDSHOW_Clear();           

    Serial_Init();          
    DHT11_Init();           
        
    YFS201_Init();
    Pump_Init();            
    Voltage_Sensor_Init();       
    Sonar_Init();           
    Servo_Init();
    Servo_SetAngle(0);
    BlindMotor_Init();
}

static void Show_ReadSensors(void)
{
    uint8_t dummy; 

    DHT11_Read_Data(&dummy, &s_Humi); 
    
    uint8_t temp_val;
    if (DHT11_2_Read_Data(&temp_val, &dummy) == 0) {
        s_Temp = (int)temp_val; 
    }

    s_LightPercent = Get_Light_Intensity_Percent(); 
    s_Flow  = (uint32_t)YFS201_GetFlowMlMin(); 
    s_Total = (uint32_t)YFS201_GetTotalMl();   
    
    s_SonarDist = Sonar_GetDistance_cm();       
    s_WaterLevel = Get_WaterLevel_Percent(s_SonarDist);
}

static void _Show_ControlAutoLogic(void)
{
    if (s_SystemStarted == 0) return;

    if (s_LedMode == 0 && s_AlarmState == 0) 
    {
        int raw_inv = 100 - s_LightPercent;
        if (raw_inv < 0) raw_inv = 0;
        
        s_LEDBright = (uint8_t)((raw_inv / 10) * 10);
        LED_SetBrightness(s_LEDBright); 
    }
}

static void Show_RefreshDisplay(void)
{
    if (s_SystemStarted == 0)
    {
        OLEDSHOW_ShowString(1, 1, "                    ");
        OLEDSHOW_ShowCN(2, 3, 0, 1); 
        OLEDSHOW_ShowCN(2, 4, 1, 1); 
        OLEDSHOW_ShowCN(2, 5, 2, 1); 
        OLEDSHOW_ShowCN(2, 6, 3, 1); 
        OLEDSHOW_ShowString(3, 1, "                    ");
        OLEDSHOW_ShowString(4, 2, "Waiting @start"); 
        return; 
    }

    if (s_ShowPage == 0) 
    {
        OLEDSHOW_ShowCN(1, 1, 101, 1); OLEDSHOW_ShowCN(1, 2, 80, 1); OLEDSHOW_ShowString(1, 5, ":"); OLEDSHOW_ShowNum(1, 6, s_Temp, 2); OLEDSHOW_ShowString(1, 8, "C   ");
        OLEDSHOW_ShowCN(2, 1, 102, 1); OLEDSHOW_ShowCN(2, 2, 80, 1); OLEDSHOW_ShowString(2, 5, ":"); OLEDSHOW_ShowNum(2, 6, s_Humi, 2); OLEDSHOW_ShowString(2, 8, "%   ");
        OLEDSHOW_ShowCN(3, 1, 33, 1); OLEDSHOW_ShowCN(3, 2, 4, 1); OLEDSHOW_ShowString(3, 5, ":"); OLEDSHOW_ShowNum(3, 6, s_LightPercent, 3); OLEDSHOW_ShowString(3, 9, "%   ");
        OLEDSHOW_ShowCN(4, 1, 4, 1); OLEDSHOW_ShowCN(4, 2, 5, 1); OLEDSHOW_ShowString(4, 5, ":"); OLEDSHOW_ShowNum(4, 6, s_LEDBright, 3); OLEDSHOW_ShowString(4, 9, "% ");
        if (s_LedMode == 0) { OLEDSHOW_ShowCN(4, 7, 31, 1); OLEDSHOW_ShowString(4, 15, "  "); } 
        else                { OLEDSHOW_ShowCN(4, 7, 35, 1); OLEDSHOW_ShowString(4, 15, "  "); }
    }
    else if (s_ShowPage == 1)
    {
        OLEDSHOW_ShowCN(1, 1, 73, 1); OLEDSHOW_ShowCN(1, 2, 74, 1); OLEDSHOW_ShowString(1, 5, ":"); 
        if(s_PumpMode == 0)      { OLEDSHOW_ShowCN(1, 4, 15, 1); OLEDSHOW_ShowCN(1, 5, 13, 1); OLEDSHOW_ShowCN(1, 6, 14, 1); OLEDSHOW_ShowString(1, 13, "  "); } 
        else if(s_PumpMode == 1) { OLEDSHOW_ShowCN(1, 4, 24, 1); OLEDSHOW_ShowCN(1, 5, 25, 1); OLEDSHOW_ShowCN(1, 6, 26, 1); OLEDSHOW_ShowCN(1, 7, 27, 1); } 
        else if(s_PumpMode == 2) { OLEDSHOW_ShowCN(1, 4, 12, 1); OLEDSHOW_ShowCN(1, 5, 25, 1); OLEDSHOW_ShowCN(1, 6, 26, 1); OLEDSHOW_ShowCN(1, 7, 27, 1); } 
        else if(s_PumpMode == 3) { OLEDSHOW_ShowCN(1, 4, 28, 1); OLEDSHOW_ShowCN(1, 5, 29, 1); OLEDSHOW_ShowCN(1, 6, 30, 1); OLEDSHOW_ShowString(1, 13, "  "); } 
        
        // 【新增】：根据工作状态显示“Working” (工作中)
        if (s_AutoFillActive == 1) {
            OLEDSHOW_ShowString(1, 9, "Working"); 
        } else {
            OLEDSHOW_ShowString(1, 9, "       "); 
        }

        OLEDSHOW_ShowCN(2, 1, 73, 1); OLEDSHOW_ShowCN(2, 2, 103, 1); OLEDSHOW_ShowString(2, 5, ":"); 
        OLEDSHOW_ShowNum(2, 6, s_WaterLevel, 3); 
        OLEDSHOW_ShowString(2, 9, "% d="); 
        OLEDSHOW_ShowNum(2, 13, s_SonarDist, 3); 
        
        OLEDSHOW_ShowCN(3, 1, 104, 1); OLEDSHOW_ShowCN(3, 2, 79, 1); OLEDSHOW_ShowString(3, 5, ":"); OLEDSHOW_ShowNum(3, 6, s_Flow, 4); OLEDSHOW_ShowString(3, 10, "m/m  ");
        OLEDSHOW_ShowCN(4, 1, 105, 1); OLEDSHOW_ShowCN(4, 2, 104, 1); OLEDSHOW_ShowString(4, 5, ":"); OLEDSHOW_ShowNum(4, 6, s_Total, 4); OLEDSHOW_ShowString(4, 10, "mL   ");
    }
    else if (s_ShowPage == 2)
    {
        OLEDSHOW_ShowCN(1, 1, 77, 1); OLEDSHOW_ShowCN(1, 2, 78, 1); OLEDSHOW_ShowString(1, 5, ":"); OLEDSHOW_ShowNum(1, 6, s_FanSpeed, 3); OLEDSHOW_ShowString(1, 9, "% ");
        if (s_FanSpeed == 0) { OLEDSHOW_ShowCN(1, 6, 16, 1); OLEDSHOW_ShowString(1, 13, "  "); } 
        else                 { OLEDSHOW_ShowCN(1, 6, 26, 1); OLEDSHOW_ShowString(1, 13, "  "); } 
        
        OLEDSHOW_ShowCN(2, 1, 6, 1); OLEDSHOW_ShowCN(2, 2, 7, 1); OLEDSHOW_ShowString(2, 5, ":"); OLEDSHOW_ShowNum(2, 6, s_DoorAngle, 3);
        if (s_DoorAngle == 0)      { OLEDSHOW_ShowCN(2, 6, 10, 1); OLEDSHOW_ShowString(2, 13, "  "); } 
        else if (s_DoorAngle > 70) { OLEDSHOW_ShowCN(2, 6, 13, 1); OLEDSHOW_ShowString(2, 13, "  "); } 
        else                       { OLEDSHOW_ShowCN(2, 6, 37, 1); OLEDSHOW_ShowString(2, 13, "  "); } 
        
        OLEDSHOW_ShowCN(3, 1, 8, 1); OLEDSHOW_ShowCN(3, 2, 9, 1); OLEDSHOW_ShowString(3, 5, ":");
        if (strcmp(s_CurtainState, "OPEN") == 0)       { OLEDSHOW_ShowCN(3, 4, 10, 1); OLEDSHOW_ShowCN(3, 5, 11, 1); OLEDSHOW_ShowCN(3, 6, 12, 1); OLEDSHOW_ShowString(3, 13, "  "); } 
        else if (strcmp(s_CurtainState, "CLOSE") == 0) { OLEDSHOW_ShowCN(3, 4, 13, 1); OLEDSHOW_ShowCN(3, 5, 14, 1); OLEDSHOW_ShowCN(3, 6, 12, 1); OLEDSHOW_ShowString(3, 13, "  "); } 
        else                                           { OLEDSHOW_ShowCN(3, 4, 15, 1); OLEDSHOW_ShowCN(3, 5, 16, 1); OLEDSHOW_ShowCN(3, 6, 17, 1); OLEDSHOW_ShowString(3, 13, "  "); } 
        OLEDSHOW_ShowString(4, 1, "                ");
    }
    else if (s_ShowPage == 3)
    {
        OLEDSHOW_ShowString(1, 1, "-- "); OLEDSHOW_ShowCN(1, 3, 18, 1); OLEDSHOW_ShowCN(1, 4, 19, 1); OLEDSHOW_ShowCN(1, 5, 20, 1); OLEDSHOW_ShowCN(1, 6, 21, 1); OLEDSHOW_ShowString(1, 13, " --"); 
        OLEDSHOW_ShowCN(2, 1, 22, 1); OLEDSHOW_ShowString(2, 3, ": "); OLEDSHOW_ShowString(2, 5, "            "); OLEDSHOW_ShowString(2, 5, s_LastRx);
        OLEDSHOW_ShowCN(3, 1, 23, 1); OLEDSHOW_ShowString(3, 3, ": "); OLEDSHOW_ShowString(3, 5, "            "); OLEDSHOW_ShowString(3, 5, s_LastTx);
        OLEDSHOW_ShowString(4, 1, "                ");
    }

    Serial_SendScreen_Number(USART1, "n0.val", s_Temp);          
    Serial_SendScreen_Number(USART1, "n1.val", s_Humi);          
    Serial_SendScreen_Number(USART1, "n2.val", s_WaterLevel); 
    Serial_SendScreen_Number(USART1, "n3.val", s_LightPercent);  
    Serial_SendScreen_Number(USART1, "n4.val", s_PumpMode);      
    Serial_SendScreen_Number(USART1, "n5.val", s_LEDBright);     
    Serial_SendScreen_Number(USART1, "n6.val", (int)s_Flow);     
    Serial_SendScreen_Number(USART1, "n7.val", (int)s_Total);    
    Serial_SendScreen_Number(USART1, "n8.val", s_FanSpeed);      
    Serial_SendScreen_Number(USART1, "n9.val", s_DoorAngle);     
    
    char* pump_zh[] = {"已关闭", "低档运行", "中档运行", "高满载"};
    char* led_zh    = (s_LedMode == 0) ? "自动光控" : "手动控制";
    char* fan_zh    = (s_FanSpeed == 0) ? "已停止" : "运行中";
    char* door_zh   = (s_DoorAngle == 0) ? "已开门" : ((s_DoorAngle > 70) ? "已关门" : "半开状态");
    
    char* curt_zh   = "已停止";
    if (strcmp(s_CurtainState, "OPEN") == 0) curt_zh = "正在开启";
    else if (strcmp(s_CurtainState, "CLOSE") == 0) curt_zh = "正在关闭";

    Serial_SendScreen_String(USART1, "t4.txt", pump_zh[s_PumpMode]);  
    Serial_SendScreen_String(USART1, "t5.txt", led_zh);               
    Serial_SendScreen_String(USART1, "t8.txt", fan_zh);               
    Serial_SendScreen_String(USART1, "t9.txt", door_zh);              
    Serial_SendScreen_String(USART1, "t10.txt", curt_zh);             
}

// ==================== 核心统一解析器 ====================
static void Show_ParseCommand(char* raw_cmd)
{
    char* ptr; 
    char cmd[32]; 
    
    if(raw_cmd == NULL) return;
    
    strncpy(cmd, raw_cmd, 30);
    cmd[30] = '\0';
    cmd[31] = '\0';
    
    strncpy(s_LastRx, cmd, 15);
    s_LastRx[15] = '\0';

    if (strstr(cmd, "start") != NULL || strstr(cmd, "START") != NULL) 
    {
        s_SystemStarted = 1;
        OLEDSHOW_Clear();              
        strcpy(s_LastTx, "Unlock OK"); 
        Serial_Printf(USART3, "\r\n[完成] 授权验证通过，系统已启动！\r\n"); 
        return; 
    }
    else if (strstr(cmd, "stop") != NULL || strstr(cmd, "STOP") != NULL) 
    {
        s_SystemStarted = 0;
        OLEDSHOW_Clear();     
        
        Pump_SetPower(0); s_PumpMode = 0;
        Fan_SetSpeed(0);  s_FanSpeed = 0;
        Blind_SetAction(0); strcpy(s_CurtainState, "STOP");
        LED_SetBrightness(0);
        
        s_AutoFillActive = 0; // 手动停止时，打断自动补水
        
        strcpy(s_LastTx, "Lock OK");
        Serial_Printf(USART3, "\r\n[完成] 紧急锁定，所有设备已停机！\r\n");
        return;
    }

    if (s_SystemStarted == 0)
    {
        strcpy(s_LastTx, "Need Start");
        Serial_Printf(USART3, "\r\n[错误] 未经授权不得操作！请先发送 @start 解锁\r\n");
        return; 
    }

    if ((ptr = strstr(cmd, "PAGE=")) != NULL) 
    {
        int page = atoi(ptr + 5); 
        if (page >= 0 && page <= 3) {  
            s_ShowPage = (uint8_t)page;
            OLEDSHOW_Clear();              
            sprintf(s_LastTx, "Page To:%d", s_ShowPage);
            Serial_Printf(USART3, "\r\n[完成] 页面已切换至: %d\r\n", s_ShowPage);
        }
    }
    // 【新增】：储水控制指令 @horizon=x (0-100)
    else if ((ptr = strstr(cmd, "horizon=")) != NULL || (ptr = strstr(cmd, "HORIZON=")) != NULL)
    {
        if (s_ShowPage != 1) { s_ShowPage = 1; OLEDSHOW_Clear(); }
        int target = atoi(ptr + 8); 
        
        if (target >= 0 && target <= 100) 
        {
            s_TargetWaterLevel = (uint8_t)target;
            s_AutoFillActive = 1;      // 开启自动补水监视
            s_FillConfirmTicks = 0;    // 防抖计时器清零
            s_PumpMode = 2;            // 设定为中档开始加水
            Pump_SetPower(PUMP_POWER_MID);
            
            strcpy(s_LastTx, "Pump: AUTO");
            Serial_Printf(USART3, "\r\n[启动] 自动储水控制启动，目标水位: %d%%...\r\n", s_TargetWaterLevel);
        }
    }
    else if (strstr(cmd, "warning") != NULL || strstr(cmd, "WARNING") != NULL)
    {
        s_AlarmState = 1;     
        s_AlarmTicks = 0;     
        s_BreathVal = 0;      
        s_BreathDir = 1;      
        strcpy(s_LastTx, "ALARM!");
        Serial_Printf(USART3, "\r\n[警告] 触发警报！LED进入紧急模式...\r\n");
    }
    else if (strstr(cmd, "safe") != NULL || strstr(cmd, "SAFE") != NULL)
    {
        s_AlarmState = 0;     
        strcpy(s_LastTx, "SAFE");
        Serial_Printf(USART3, "\r\n[解除] 警报解除！LED恢复正常。\r\n");
        LED_SetBrightness(s_LEDBright); 
    }
    else if ((ptr = strstr(cmd, "PUMP=")) != NULL)
    {
        if (s_ShowPage != 1) { s_ShowPage = 1; OLEDSHOW_Clear(); }
        int mode = atoi(ptr + 5);
        if (mode >= 0 && mode <= 3) {
            s_AutoFillActive = 0;  // 手动操作水泵时，打断自动补水
            s_PumpMode = (uint8_t)mode;
            if      (s_PumpMode == 0) Pump_SetPower(0);
            else if (s_PumpMode == 1) Pump_SetPower(PUMP_POWER_LOW);
            else if (s_PumpMode == 2) Pump_SetPower(PUMP_POWER_MID);
            else if (s_PumpMode == 3) Pump_SetPower(PUMP_POWER_HIGH);
            
            sprintf(s_LastTx, "Pump Lvl:%d", mode);
            Serial_Printf(USART3, "\r\n[完成] 水泵档位切换为: %d\r\n", s_PumpMode);
        }
    }
    else if ((ptr = strstr(cmd, "FENG=")) != NULL)
    {
        if (s_ShowPage != 2) { s_ShowPage = 2; OLEDSHOW_Clear(); }
        int speed = atoi(ptr + 5); 
        if (speed >= 0 && speed <= 100) {
            s_FanSpeed = (uint8_t)speed;
            Fan_SetSpeed(s_FanSpeed);
            sprintf(s_LastTx, "Fan Spd:%d", s_FanSpeed);
            Serial_Printf(USART3, "\r\n[完成] 风扇速度设定为: %d%%\r\n", s_FanSpeed);
        }
    }
    else if ((ptr = strstr(cmd, "Angle=")) != NULL)
    {
        if (s_ShowPage != 2) { s_ShowPage = 2; OLEDSHOW_Clear(); }
        int angle = atoi(ptr + 6); 
        if (angle >= 0 && angle <= 180) {
            s_DoorAngle = (uint8_t)angle;
            Servo_SetAngle(s_DoorAngle); 
            
            char* d_status = "半开";
            if (s_DoorAngle == 0) d_status = "开门";
            else if (s_DoorAngle > 70) d_status = "关门";

            sprintf(s_LastTx, "Door Deg:%d", s_DoorAngle);
            Serial_Printf(USART3, "\r\n[完成] 舱门角度设定: %d度 [%s]\r\n", s_DoorAngle, d_status);
        }
    }
    else if (strstr(cmd, "KAI") != NULL) 
    {
        if (s_ShowPage != 2) { s_ShowPage = 2; OLEDSHOW_Clear(); }
        Blind_SetAction(2); 
        strcpy(s_CurtainState, "OPEN");
        strcpy(s_LastTx, "Curt: OPEN");
        
        s_CurtainTargetTicks = 270; 
        s_CurtainRunTicks = 0;
        Serial_Printf(USART3, "\r\n[完成] 窗帘开启中 (预计运行2.7秒)...\r\n");
    }
    else if (strstr(cmd, "GUAN") != NULL) 
    {
        if (s_ShowPage != 2) { s_ShowPage = 2; OLEDSHOW_Clear(); }
        Blind_SetAction(1); 
        strcpy(s_CurtainState, "CLOSE");
        strcpy(s_LastTx, "Curt: CLOSE");
        
        s_CurtainTargetTicks = 270; 
        s_CurtainRunTicks = 0;
        Serial_Printf(USART3, "\r\n[完成] 窗帘关闭中 (预计运行2.7秒)...\r\n");
    }
    else if (strstr(cmd, "TING") != NULL) 
    {
        if (s_ShowPage != 2) { s_ShowPage = 2; OLEDSHOW_Clear(); }
        Blind_SetAction(0); 
        strcpy(s_CurtainState, "STOP");
        strcpy(s_LastTx, "Curt: STOP");
        
        s_CurtainTargetTicks = 0; 
        s_CurtainRunTicks = 0;
        Serial_Printf(USART3, "\r\n[完成] 窗帘已立刻停止!\r\n");
    }
    else if (strstr(cmd, "Auto") != NULL)
    {
        if (s_ShowPage != 0) { s_ShowPage = 0; OLEDSHOW_Clear(); }
        s_LedMode = 0; 
        strcpy(s_LastTx, "LED: Auto");
        Serial_Printf(USART3, "\r\n[完成] LED已切换至: 自动光控模式\r\n");
    }
    else if (strstr(cmd, "Manu") != NULL)
    {
        if (s_ShowPage != 0) { s_ShowPage = 0; OLEDSHOW_Clear(); }
        s_LedMode = 1; 
        strcpy(s_LastTx, "LED: Manu");
        Serial_Printf(USART3, "\r\n[完成] LED已切换至: 手动控制模式\r\n");
    }
    else if ((ptr = strstr(cmd, "Light=")) != NULL)
    {
        if (s_ShowPage != 0) { s_ShowPage = 0; OLEDSHOW_Clear(); }
        int level = atoi(ptr + 6); 
        if (level >= 0 && level <= 10) {
            s_LedMode = 1; 
            s_LEDBright = (uint8_t)(level * 10); 
            LED_SetBrightness(s_LEDBright);
            sprintf(s_LastTx, "Light Lvl:%d", level);
            Serial_Printf(USART3, "\r\n[完成] (已切手动) LED 亮度设定为: 档位%d (%d%%)\r\n", level, s_LEDBright);
        }
    }
    else if (strstr(cmd, "REFRESH") != NULL)
    {
        strcpy(s_LastTx, "Data Refresh");
        
        char* d_status = s_DoorAngle > 70 ? "关" : (s_DoorAngle == 0 ? "开" : "半开");
        char* led_st = s_LedMode == 0 ? "自动" : "手动";
        char* curt_st = "已停止";
        if(strcmp(s_CurtainState, "OPEN") == 0) curt_st = "开启中";
        else if(strcmp(s_CurtainState, "CLOSE") == 0) curt_st = "关闭中";
        
        Serial_Printf(USART3, "\r\n--- 状态报告 ---\r\n");
        Serial_Printf(USART3, "温度:%dC 湿度:%d%%\r\n", s_Temp, s_Humi);
        Serial_Printf(USART3, "水位:%d%% 光照:%d%%\r\n", s_WaterLevel, s_LightPercent);
        Serial_Printf(USART3, "流速:%d mL/m 总:%dmL\r\n", s_Flow, s_Total);
        Serial_Printf(USART3, "水泵:%d档 风扇:%d%%\r\n", s_PumpMode, s_FanSpeed);
        Serial_Printf(USART3, "照明:%d%% (%s)\r\n", s_LEDBright, led_st);
        Serial_Printf(USART3, "舱门:%d度(%s) 窗帘:%s\r\n", s_DoorAngle, d_status, curt_st);
    }
}

static void Show_HandleCommands(void)
{
    if (Serial_RxFlag_3 == 1) 
    {
        Show_ParseCommand((char*)Serial_RxPacket_3);
        Serial_RxFlag_3 = 0; 
    }
    
    if (Serial_RxFlag_1 == 1) 
    {
        Show_ParseCommand((char*)Serial_RxPacket_1);
        Serial_RxFlag_1 = 0; 
    }
}

void Show_MainLoop(void)
{
    static uint16_t time_counter = 0;
    Delay_ms(10);
    time_counter++;
    
    // ==================== 【新增】水位闭环防抖控制逻辑 ====================
    if (s_AutoFillActive == 1)
    {
        // 判定条件：当前水位百分比 >= 设定的目标水位
        if (s_WaterLevel >= s_TargetWaterLevel)
        {
            s_FillConfirmTicks++; // 坚持一下 (防抖累加)
            
            // 稳定坚持 2 秒钟 (200个10ms) 不掉下来，才算真正到达！
            if (s_FillConfirmTicks >= 200) 
            {
                s_AutoFillActive = 0;  // 关闭自动控制标记
                s_PumpMode = 0;        // 设定档位为停止
                Pump_SetPower(0);      // 物理断电
                strcpy(s_LastTx, "Pump: DONE");
                Serial_Printf(USART3, "\r\n[成功] 目标水位 %d%% 已稳定到达，水泵已停止!\r\n", s_TargetWaterLevel);
                
                s_FillConfirmTicks = 0; // 重置计时器
            }
        }
        else
        {
            // 如果水波荡漾导致数据掉下去了，立刻清零计时器，继续工作！
            s_FillConfirmTicks = 0;
        }
    }

    // ==================== 窗帘非阻塞定时逻辑 ====================
    if (s_CurtainTargetTicks > 0)
    {
        s_CurtainRunTicks++;
        if (s_CurtainRunTicks >= s_CurtainTargetTicks)
        {
            Blind_SetAction(0); 
            strcpy(s_CurtainState, "STOP"); 
            strcpy(s_LastTx, "Curt: AutoSTOP"); 
            Serial_Printf(USART3, "\r\n[系统提示] 窗帘行程到达，已自动停止!\r\n");
            
            s_CurtainTargetTicks = 0; 
            s_CurtainRunTicks = 0;
        }
    }

    // ==================== 警报灯光非阻塞逻辑 ====================
    if (s_AlarmState == 1)
    {
        s_AlarmTicks++;
        
        // 第一阶段：前 3 秒爆闪
        if (s_AlarmTicks <= 300) 
        {
            if ((s_AlarmTicks % 20) < 10) {
                LED_SetBrightness(100); 
            } else {
                LED_SetBrightness(0);   
            }
        }
        // 第二阶段：平缓呼吸灯
        else 
        {
            if (s_AlarmTicks % 2 == 0) 
            {
                s_BreathVal += (s_BreathDir * 2); 

                if (s_BreathVal >= 100) {
                    s_BreathVal = 100;
                    s_BreathDir = -1; 
                } else if (s_BreathVal <= 0) {
                    s_BreathVal = 0;
                    s_BreathDir = 1;  
                }
                LED_SetBrightness((uint8_t)s_BreathVal);
            }
        }
    }

    // ==================== 常规刷新逻辑 ====================
    if (time_counter >= 50)  
    {
        time_counter = 0;
        Show_ReadSensors();      
        _Show_ControlAutoLogic(); 
        Show_RefreshDisplay();       
    }
    
    Show_HandleCommands(); 
}
