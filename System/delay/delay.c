#include "delay.h"

/*
    SysTick定时器（滴答定时器）
    
    CTRL（控制与状态寄存器）
    LOAD（自动重装载值寄存器）           24位倒计数定时器   计到0时，自动重装初值不停止
    VAL （当前值寄存器）     			 读取时返回当前值，写入将清零
*/
/*SysTick控制及状态寄存器CTRL（地址：0xE000_E010）
位段      名称            类型                 说明
 16      COUNTFLAG       只读       计数到0置1，读取自动清零
  2      CLKSOURCE      可读写      时钟源   0=HCLK/8    1=HCLK
  1       TICKINT       可读写      1=SysTick 倒数到 0 时产生 SysTick 异常请求   0=数到0时无动作
  0       ENABLE        可读写      SysTick 定时器的使能位  1使能   0失能
*/

void delay_us(uint16_t us)
{
    SysTick ->LOAD =(SystemCoreClock /1000000)*us;  //重装计数器的值 用72mhz/1mhz =72  计数72次为1us
    SysTick->VAL   = 0;                             //清空计数器的值
    SysTick->CTRL  = 0x00000005;                    //设置时钟源为HCLK，启动定时器
    while(!(SysTick->CTRL&0x00010000));  		    //等待计数到0 (0001 0000 0000 0000 0000 0000 位16置1) 
    SysTick->CTRL  = 0x00000004;       				//关闭定时器
    SysTick->VAL   = 0;                				//清空计数器的值
}



void delay_ms(uint16_t ms)
{
    while(ms--)
    {
        delay_us(1000);
    }
}

void delay_s(uint16_t s)
{
    while(s--)
    {
        delay_ms(1000);
    }
}







































