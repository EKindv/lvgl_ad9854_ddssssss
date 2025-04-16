#include "delay.h"

/*
    SysTick��ʱ�����δ�ʱ����
    
    CTRL��������״̬�Ĵ�����
    LOAD���Զ���װ��ֵ�Ĵ�����           24λ��������ʱ��   �Ƶ�0ʱ���Զ���װ��ֵ��ֹͣ
    VAL ����ǰֵ�Ĵ�����     			 ��ȡʱ���ص�ǰֵ��д�뽫����
*/
/*SysTick���Ƽ�״̬�Ĵ���CTRL����ַ��0xE000_E010��
λ��      ����            ����                 ˵��
 16      COUNTFLAG       ֻ��       ������0��1����ȡ�Զ�����
  2      CLKSOURCE      �ɶ�д      ʱ��Դ   0=HCLK/8    1=HCLK
  1       TICKINT       �ɶ�д      1=SysTick ������ 0 ʱ���� SysTick �쳣����   0=����0ʱ�޶���
  0       ENABLE        �ɶ�д      SysTick ��ʱ����ʹ��λ  1ʹ��   0ʧ��
*/

void delay_us(uint16_t us)
{
    SysTick ->LOAD =(SystemCoreClock /1000000)*us;  //��װ��������ֵ ��72mhz/1mhz =72  ����72��Ϊ1us
    SysTick->VAL   = 0;                             //��ռ�������ֵ
    SysTick->CTRL  = 0x00000005;                    //����ʱ��ԴΪHCLK��������ʱ��
    while(!(SysTick->CTRL&0x00010000));  		    //�ȴ�������0 (0001 0000 0000 0000 0000 0000 λ16��1) 
    SysTick->CTRL  = 0x00000004;       				//�رն�ʱ��
    SysTick->VAL   = 0;                				//��ռ�������ֵ
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







































