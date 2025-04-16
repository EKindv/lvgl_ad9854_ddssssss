#include "stm32f10x.h"                
#include "usart.h"
#include "lvgl.h"

void timer_init(uint16_t arr,uint16_t psc)
{
    TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStruct;
    NVIC_InitTypeDef    NVIC_InitStructure;  

    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2,ENABLE);
    
    TIM_InternalClockConfig(TIM2);                                /*�����ڲ�ʱ��*/
    
    TIM_TimeBaseInitStruct.TIM_ClockDivision=TIM_CKD_DIV1;        /* 1��Ƶ          */
    TIM_TimeBaseInitStruct.TIM_CounterMode=TIM_CounterMode_Up;    /* ���ϼ���       */
    TIM_TimeBaseInitStruct.TIM_Period=arr;                        /* ARR �Զ���װֵ   */
    TIM_TimeBaseInitStruct.TIM_Prescaler=psc;                     /* psc Ԥ��Ƶϵ��   */
    TIM_TimeBaseInitStruct.TIM_RepetitionCounter=0;               /* �߼���ʱ������  */
    TIM_TimeBaseInit(TIM2,&TIM_TimeBaseInitStruct);            
    
    TIM_ClearFlag(TIM2,TIM_FLAG_Update);
    TIM_ITConfig(TIM2,TIM_IT_Update,ENABLE);                      /*�����ж�ʹ��*/
    

    NVIC_InitStructure.NVIC_IRQChannel=TIM2_IRQn;                 /* TIM2�ж� */
    NVIC_InitStructure.NVIC_IRQChannelCmd=ENABLE;                 /* ʹ���ж� */
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=3;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority=0;
    NVIC_Init(&NVIC_InitStructure);
    
    TIM_Cmd(TIM2,ENABLE);                   
}



void TIM2_IRQHandler(void)
{
    if(TIM_GetITStatus(TIM2,TIM_IT_Update)==1)        /*�����ж���1*/
    {

        lv_tick_inc(1);
        TIM_ClearITPendingBit(TIM2,TIM_IT_Update);
    }
}

