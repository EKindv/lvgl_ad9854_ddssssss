#include "stm32f10x.h"  
#include "sys.h"
#include "delay.h"
#include "lcd.h"
#include "timer.h"
#include "lvgl.h"
#include "lv_port_disp_template.h"
#include "lv_port_indev_template.h"
#include "AD9854.h"
#include "usart.h"
#include "stm32f10x_usart.h"  
#include "touch.h"  
#include "w25qx.h"
#include "test code.h"
#include "function.h"
#include "lvgl/lvgl.h"
#include "math.h"
#include "ui.h"

int main(void)
{

	Stm32_Clock_Init(9);	//系统时钟设置
	delay_init(72);	   	 	//延时初始化 
	Delay_1ms(100);
	timer_init(999, 71);
	GPIO_AD9854_Configuration();
	AD9854_Init();
	USART1_Init();

	USART_SendData(USART1,0);	
	LCD_Init();
	
	//初始化LCD 

	tp_dev.touchtype|=USE_TP_TYPE;			//电容触摸&电阻触摸
	LCD_Display_Dir(3);		 		//屏幕方向
	LCD_Clear(WHITE);		//清屏

	delay_ms(500);		
	timer_init(72-1,1000-1);

	lv_init();                                          /* lvgl系统初始化 */
  lv_port_disp_init();                                /* lvgl显示接口初始化,放在lv_init()的后面 */
	lv_port_indev_init(); 
	tp_dev.init();
	
//	lv_ex_label();	/* lvgl输入接口初始化,放在lv_init()的后面 */
//	create_moving_boxes();
	GPIOD->ODR |= (1 << 15);//点亮背光	
	USART_SendData(USART1,2);
	ui_init();
//	lv_task_handler();

	delay_ms(10);
	while(1)
	{		
		lv_timer_handler(); /* LVGL计时器 */
	}

}
