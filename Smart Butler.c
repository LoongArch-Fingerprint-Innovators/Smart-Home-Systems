#include <stdio.h>
#include <stdbool.h>
#include "bsp.h"
#include "console.h"
#include "ls1c102.h"
#include "font.h"                //调用同目录下的字库、图片
#include <U8g2lib.h>             //U8g2库 用于0.96 OLED IIC显示器
#ifdef U8X8_HAVE_HW_I2C
#include <Adafruit_Fingerprint.h>//AS608指纹
#include<DHT.h>//温湿度传感  
DHT dht(7,DHT11);//温湿度data接脚
#include <Wire.h>//调用I2C库
#include <LiquidCrystal_I2C.h>//调用LCD的I2C库

#endif
int q=300;                        //记录累计开门次数
U8G2_SSD1306_128X64_NONAME_1_SW_I2C u8g2(U8G2_R0,SCL,SDA, U8X8_PIN_NONE);
#define x_coordinate 40
#define KEY1 2
#define KEY2 3
#define KEY3 4
#define KEY4 5
#define KEY5 15
#define buzzer 6    // 蜂鸣器引脚
#define LED  10
#define fanPin  8
#define led 13      // 红外线，灯引脚
const int YanginSensor = A0; // 火焰传感器引脚
const int HareketSensor = A1; // PIR 传感器引脚
int fanSpeed;

SoftwareSerial mySerial(11,12);//新建一个名为mySerial的软串口 并将11号引脚作为RX端 12号引脚作为TX端
Adafruit_Fingerprint finger = Adafruit_Fingerprint(&mySerial);
u8 key_num=0;
int t;
int h;
u8 Angle = 135; //舵机默认开合角度   主页面按下K4进入设置舵机转动角度功能
void key_init()
{
    pinMode(KEY1, INPUT_PULLUP);
    pinMode(KEY2, INPUT_PULLUP);
    pinMode(KEY3, INPUT_PULLUP);
    pinMode(KEY4, INPUT_PULLUP);
    pinMode(KEY5, INPUT_PULLUP);
    pinMode(LED, OUTPUT);
    pinMode(fanPin, OUTPUT);
    pinMode (YanginSensor, INPUT); //读取火焰传感器的信号。
    pinMode (buzzer, OUTPUT);   //控制蜂鸣器的响或关
    
    pinMode (HareketSensor, INPUT); //连接 PIR 传感器，用于读取运动检测信号。
    pinMode (led, OUTPUT);  //该引脚连接 LED 灯，用于控制灯的开关。
}

u8 key_scan(u8 mode)
{
    Serial.println("staring key_scan....");
    static u8 key_up=1;//按键按松开标志
    if(mode)key_up=1;  //支持连按
    if(key_up&&(digitalRead(KEY1)==0||digitalRead(KEY2)==0||
                digitalRead(KEY3)==0||digitalRead(KEY4)==0||digitalRead(KEY5)==0))
    {
        delay(10);
        key_up=0;
        if(digitalRead(KEY1)==0)return 1;
        else if(digitalRead(KEY2)==0)return 2;
        else if(digitalRead(KEY3)==0)return 3;
        else if(digitalRead(KEY4)==0)return 4;
        else if(digitalRead(KEY5)==0)return 5;
    }
    else if(digitalRead(KEY1)==1&&digitalRead(KEY2)==1&&
            digitalRead(KEY3)==1&&digitalRead(KEY4)==1&&digitalRead(KEY5)==1)
        key_up=1;
    return 0;// 无按键按下
}


void MG90S(int angle)//MG90S舵机PWM脉冲
{
    for (int i = 1; i < 51; i++)
    {
        servopulse(angle);
        delay(10);
    }
}

void servopulse(int angle)//MG90S舵机PWM脉冲
{
    int pulsewidth = (angle * 11) + 500;
    digitalWrite(9, HIGH);
    delayMicroseconds(pulsewidth);
    digitalWrite(9, LOW);
    delayMicroseconds(20000 - pulsewidth);
}




void Add_FR()      
{                                ///////////////////
                                 // 添加指纹 函数
    u8 i,ensure,processnum=0;    // By ZYB  2021 02.02
    u8 ID_NUM=0;                 ///////////////////
    char str2[10];
    while(1)
    {
        switch (processnum)
        {
        case 0:
            i++;
            u8g2.firstPage();
            do
            {
                u8g2.drawXBMP(32,24,64,16,State5);  /* 字串 请按手指   64x16  */
            }
            while(u8g2.nextPage());
            ensure=finger.getImage();
            if(ensure==FINGERPRINT_OK)
            {
                ensure=finger.image2Tz(1);//生成特征
                if(ensure==FINGERPRINT_OK)
                {
                    u8g2.firstPage();
                    do
                    {
                        u8g2.drawXBMP(32,24,64,16,State6);  /* 字串 指纹正常  64x16  */
                    }
                    while(u8g2.nextPage());
                    // Serial.println(" 000 is true");
                    i=0;
                    processnum=1;//跳到第二步
                }
                else {};
            }
            else {};
            break;

        case 1:
            i++;
            u8g2.firstPage();
            do
            {
                u8g2.drawXBMP(32,24,64,16,State7);  /* 字串 再按一次   64x16  */
            }
            while(u8g2.nextPage());
            ensure=finger.getImage();
            if(ensure==FINGERPRINT_OK)
            {
                ensure=finger.image2Tz(2);//生成特征
                if(ensure==FINGERPRINT_OK)
                {
                    u8g2.firstPage();
                    do
                    {
                        u8g2.drawXBMP(32,24,64,16,State6);  /* 字串 指纹正常  64x16  */
                    }
                    while(u8g2.nextPage());
                    i=0;
                    processnum=2;//跳到第三步
                }
                else {};
            }
            else {};
            break;

        case 2:
            u8g2.firstPage();
            do
            {
                u8g2.drawXBMP(32,24,64,16,State8);/* 字串 创建模板   64x16  */
            }
            while(u8g2.nextPage());
            ensure=finger.createModel();
            if(ensure==FINGERPRINT_OK)
            {
                u8g2.firstPage();
                do
                {
                    u8g2.drawXBMP(16,24,96,16,State9);  /* 字串 模板创建成功   96x16  */
                }
                while(u8g2.nextPage());
                processnum=3;//跳到第四步
            }
            else
            {
                u8g2.firstPage();
                do
                {
                    u8g2.drawXBMP(16,24,96,16,State10);  /* 字串 模板创建失败   96x16  */
                }
                while(u8g2.nextPage());
                i=0;
                processnum=0;//跳回第一步
            }
            delay(500);
            break;
        case 3:

            u8g2.firstPage();
            do
            {
                u8g2.drawXBMP(1,0,128,48,State11);
                /* 字串  按K4加，按K2减 按K3保存 0=< ID <=99 128x48*/
                u8g2.setFont(u8g2_font_gb16st_t_2); // 选择字体
                u8g2.drawStr(40,62,"ID=00");
            }
            while(u8g2.nextPage());

            while(key_num!=3)
            {
                key_num=key_scan(0);
                if(key_num==2)
                {
                    key_num=0;
                    if(ID_NUM>0)
                        ID_NUM--;
                    if(ID_NUM<10)
                        sprintf(str2,"ID=0%d",ID_NUM);
                    else
                        sprintf(str2,"ID=%d",ID_NUM);
                    u8g2.firstPage();
                    do
                    {
                        u8g2.setFont(u8g2_font_gb16st_t_2); // 选择字体
                        u8g2.drawXBMP(1,0,128,48,State11);
                        u8g2.drawStr(40,62,str2);
                    }
                    while(u8g2.nextPage());
                }
                if(key_num==4)
                {
                    key_num=0;
                    if(ID_NUM<99)
                        ID_NUM++;
                    if(ID_NUM<10)
                        sprintf(str2,"ID=0%d",ID_NUM);
                    else
                        sprintf(str2,"ID=%d",ID_NUM);
                    u8g2.firstPage();
                    do
                    {
                        u8g2.setFont(u8g2_font_gb16st_t_2);
                        u8g2.drawStr(40,62,str2);
                        u8g2.drawXBMP(1,0,128,48,State11);
                    }
                    while(u8g2.nextPage());
                }
            }
            key_num=0;
            ensure=finger.storeModel(ID_NUM);//储存模板
            if(ensure==0x00)
            {
                u8g2.firstPage();
                do
                {
                    u8g2.drawXBMP(16,24,96,16,State12);  /* 字串 录入指纹成功   96x16  */
                }
                while(u8g2.nextPage());
                Serial.println("FR receive OK");
                delay(1500);
                return ;
            }
            else
            {
                processnum=0;
            }
            break;
        }
        delay(400);
        if(i==10)//超过5次没有按手指则退出
        {
            break;
        }
    }
}



void Press_FR()
{
                           //////////////////////
    u8 ensure,i;           // 验证指纹并开锁 函数
    char str[20];          // By ZYB  2021 02.02
    char cishu[5];        //////////////////////
    char wendu[15];
    char shidu[15];
    u8g2.firstPage();
    do
    {
        u8g2.drawXBMP(32,24,64,16,State5);  /* 字串 请按手指   64x16  */
    }
    while(u8g2.nextPage());
    while(1)
    {
        ensure=finger.getImage();
        if(ensure==0x00)//获取图像成功
        {
            ensure=finger.image2Tz();
            if(ensure==0x00) //生成特征成功
            {
                ensure=finger.fingerFastSearch();
                if(ensure==0x00)//搜索成功
                {
                    MG90S(Angle);
                    digitalWrite(LED, 1);
                    fanSpeed = 20;
                    digitalWrite(fanPin, fanSpeed);
                    
                    q++;
                    sprintf(str,"ID:%d Score:%d",finger.fingerID,finger.confidence);
                    sprintf(cishu,"%d",q);
                    u8g2.firstPage();
                    do
                    {
                        u8g2.setFont(u8g2_font_gb16st_t_2); // 选择字体
                        u8g2.drawXBMP(16,16,96,16,State13);  //显示指纹搜索成功
                        u8g2.drawStr(100,16,cishu);
                        u8g2.drawStr(1,46,str);
                    }
                    while(u8g2.nextPage());
//                    Serial.println("OPEN");
                    delay(1401);
                    MG90S(1);

                    
                    key_num=key_scan(0);
                    if(key_num==1)
                    {
                        break;
                    }
                    

                    t = dht.readTemperature();
                    h = dht.readHumidity();
                    delay(100);
                    sprintf(wendu,"%dC",t);
                    sprintf(shidu,"%d",h);

                    u8g2.firstPage();
                    do
                    {
                        u8g2.setFont(u8g2_font_gb16st_t_2);
                        u8g2.drawXBMP(0,0,128,64,HOME);  /* 图片 主页  128x64  */
                        u8g2.drawStr(100,32,wendu);
                        u8g2.drawStr(100,46,shidu);
                        u8g2.drawStr(120,46,"%");
                    }
                    while(u8g2.nextPage());
                }
                else
                {
                    u8g2.firstPage();
                    do
                    {
                        u8g2.drawXBMP(16,16,96,16,State14); //State14_字串 未搜索到指纹 96x16
                    }
                    while(u8g2.nextPage());

                    delay(600);
                    u8g2.firstPage();
                    do
                    {
                        u8g2.setFont(u8g2_font_gb16st_t_2);
                        u8g2.drawXBMP(0,0,128,64,HOME);  /* 图片 主页  128x64  */
                        u8g2.drawStr(100,32,wendu);
                        u8g2.drawStr(100,46,shidu);
                        u8g2.drawStr(120,46,"%");
                    }
                    while(u8g2.nextPage());
                    //break;
                }
            }
            else
            {
                //ShowErrMessage(ensure);
            }
        }
        delay(200);
        key_num=key_scan(0);
        if(key_num==1)
        {
             digitalWrite(LED, 0);
        }
        if(key_num==2)
        {
             digitalWrite(fanPin, 0);
        }
         ////////// 检查火焰传感器状态 /////////
  
        int YanginSensorState = digitalRead (YanginSensor);
        if (YanginSensorState == LOW) 
        {
          // 如果检测到火焰，蜂鸣器响
          digitalWrite(buzzer, HIGH);
        }
        else
        {
          // 否则，蜂鸣器关闭
          digitalWrite(buzzer, LOW);
        }
      
        ///////////// 检查 PIR 传感器状态 /////////////
        
        int HareketSensorState = digitalRead (HareketSensor);
        if (HareketSensorState == LOW)
        {
          // 如果检测到运动，灯亮
          digitalWrite(led, HIGH);
        }
        else
        {
          // 否则，灯灭
          digitalWrite(led, LOW);
        }
    }
    
}


void Del_FR()          /////////////////////
{                      ////////////////////
                       //选择删除指纹函数
    u8  ensure;        //By ZYB  2021 02.02
    u8 ID_NUM=0;       /////////////////////
    char str2[10];
    sprintf(str2,"ID=0%d",ID_NUM);
    u8g2.firstPage();
    do
    {
        u8g2.setFont(u8g2_font_gb16st_t_2);
        u8g2.drawStr(44,62,str2);
        u8g2.drawXBMP(1,0,128,48,State15);  //显示字模汉字
    }
    while(u8g2.nextPage());



    while(key_num!=3)
    {
        key_num=key_scan(0);
        if(key_num==2)
        {
            key_num=0;
            if(ID_NUM>0)
                ID_NUM--;
            if(ID_NUM<10)
                sprintf(str2,"ID=0%d",ID_NUM);
            else
                sprintf(str2,"ID=%d",ID_NUM);
            u8g2.firstPage();
            do
            {
                u8g2.drawStr(44,62,str2);
            }
            while(u8g2.nextPage());

        }
        if(key_num==4)
        {
            key_num=0;
            if(ID_NUM<99)
                ID_NUM++;
            if(ID_NUM<10)
                sprintf(str2,"ID=0%d",ID_NUM);
            else
                sprintf(str2,"ID=%d",ID_NUM);
            u8g2.firstPage();
            do
            {
                u8g2.drawStr(44,62,str2);

            }
            while(u8g2.nextPage());
        
        }
        
        if(key_num==1)
        {
            key_num=0;
            ensure=finger.emptyDatabase();//清空指纹库
            if(ensure==0)
            {
                u8g2.firstPage();
                do
                {
                    u8g2.drawXBMP(8,16,80,16,State17);  //显示字模汉字
                    u8g2.drawXBMP(88,16,32,16,State19);  //显示字模汉字
                }
                while(u8g2.nextPage());

            }
            else
            {
                u8g2.firstPage();
                do
                {
                    u8g2.drawXBMP(8,16,80,16,State17);  //显示字模汉字
                    u8g2.drawXBMP(88,16,32,16,State20);  //显示字模汉字

                }
                while(u8g2.nextPage());

            }
            delay(1500);
            goto MENU ; //返回主页面
        }
    }
    ensure=finger.deleteModel(ID_NUM);//删除单个指纹
    if(ensure==0)
    {
        u8g2.firstPage();
        do
        {
            u8g2.drawXBMP(16,16,64,16,State18);  //显示字模汉字
            u8g2.drawXBMP(80,16,32,16,State19);  //显示字模汉字
        }
        while(u8g2.nextPage());


    }
    else
    {
        u8g2.firstPage();
        do
        {
            u8g2.drawXBMP(16,16,64,16,State18);  //显示字模汉字
            u8g2.drawXBMP(80,16,32,16,State20);  //显示字模汉字

        }
        while(u8g2.nextPage());

    }
    delay(1500);
MENU:
    key_num=0;
}

void setAngle()
{
    char str2[10];
    sprintf(str2, "setAngle=%d", Angle);
    char str3[15];
    sprintf(str3, "K2:- K3:OK  K4:+");
    u8g2.firstPage();
    do
    {
        u8g2.setFont(u8g2_font_gb16st_t_2);
        u8g2.drawStr(0, 13, str2);
        u8g2.drawStr(0, 26, str3);
    } while (u8g2.nextPage());

    while (key_num != 3)
    {
        key_num = key_scan(0);
        if (key_num == 2)
        {
            key_num = 0;

            Angle -= 10;

            sprintf(str2, "setAngle=%d", Angle);
            u8g2.firstPage();
            do
            {
                u8g2.drawStr(0, 13, str2);
                u8g2.drawStr(0, 26, str3);

            } while (u8g2.nextPage());
        }
        if (key_num == 4)
        {
            key_num = 0;

            Angle += 10;

            sprintf(str2, "setAngle=%d", Angle);
            u8g2.firstPage();
            do
            {
                u8g2.drawStr(0, 13, str2);
                u8g2.drawStr(0, 26, str3);

            } while (u8g2.nextPage());
        }
        if (key_num == 3)
            break;
    }
}

void MENU()
{
    u8g2.firstPage();
    do
    {
        u8g2.setFont(u8g2_font_gb16st_t_2); // 选择字体
        u8g2.drawXBMP(4,0,112,16,State1);  //显示字模汉字   /* 竹401指纹门禁  128x16  */
        u8g2.drawXBMP(40,16,64,16,State2);/* 字串 添加指纹   64x16  */
        u8g2.drawXBMP(40,32,64,16,State3);/* 字串 删除指纹   64x16  */
        u8g2.drawXBMP(40,48,64,16,State4);/* 字串 验证指纹   64x16  */
        u8g2.drawStr(22,30,"K1");
        u8g2.drawStr(22,46,"K2");
        u8g2.drawStr(22,62,"K3");
    }
    while(u8g2.nextPage());
    Serial.println("一切准备就绪....");
    Serial.println("By YBBandHZM");
}

void setup()
{
    // put your setup code here, to run once:
    key_init();
    dht.begin();//开启温湿度
    u8g2.begin();       //开启OLED通信
    Serial.begin(9600); //开启串口通信 波特率9600
    finger.begin(57600); //设置AS608波特率 57600
    MENU();
}

void loop()
{

    key_num = key_scan(0);
    if(key_num == 1)            //按下K1键 调用Add_FR()
    {
      Serial.println("add_fr....");
        
        key_num = 0;
        Add_FR();
        MENU();
    }

    if(key_num == 2)            //按下K2键 调用Del_FR();
    {
        digitalWrite(LED, 0);
        key_num = 0;
        Del_FR();
        MENU();
    }

    if(key_num == 3)            //按下K3键 调用Press_FR()
    {
      Serial.println("press_fr....");
        key_num = 0;
        Press_FR();
        MENU();
    }
    if (key_num == 4) //按下K4键 调节舵机最大转动度数（默认最大135°）
    {
        key_num = 0;
        setAngle();
        MENU();
    }
   
}
