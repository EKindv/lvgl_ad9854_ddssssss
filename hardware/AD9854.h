/*--------------------------------------------------------------
头文件名：AD9854.h				  
文件描述：数据处理头文件
---------------------------------------------------------------*/
#ifndef _AD9854_H_
#define _AD9854_H_

#include <stm32f10x.h>
//----------------------------------------------------------------
// AD9854_I/O控制线
//					 AD9854模块标号     |    STM32核心板         
//					 A[5:0]   				 -->   PA[5:0](GPIOA[5:0])
//					 D[7:0]    			   -->   PB[15:8](GPIOB[15:8])
//					 RD                -->   PC8(GPIOC_PIN8)
//					 WR                -->   PC9(GPIOC_PIN9)
//					 UD(UDCLK)         -->   PC10(GPIOC_PIN10)
//					 RT(RESET)         -->   PB14(GPIOC_PIN11)
//					 SP                -->   PB15(GPIOC_PIN12)
//					 AD9854模块标号     |    外部信号比如信号源连接控制         
//					 OSK      					外部控制信号接在OSK引脚来控制D12
//					 FD(FDSTA)       		外部信号接AD9854中的FSK(S2)射频头来控制也可以直接输入到AD9854中的FD引脚D13
//-----------------------------------------------------------------
#define AD9854_RD_Set (GPIO_SetBits(GPIOC,GPIO_Pin_8))         
#define AD9854_RD_Clr (GPIO_ResetBits(GPIOC,GPIO_Pin_8))

#define AD9854_WR_Set (GPIO_SetBits(GPIOC,GPIO_Pin_9))         
#define AD9854_WR_Clr (GPIO_ResetBits(GPIOC,GPIO_Pin_9))

#define AD9854_UDCLK_Set (GPIO_SetBits(GPIOC,GPIO_Pin_10))           
#define AD9854_UDCLK_Clr (GPIO_ResetBits(GPIOC,GPIO_Pin_10))

#define AD9854_RST_Set (GPIO_SetBits(GPIOC,GPIO_Pin_11))				   //开关电源板
#define AD9854_RST_Clr (GPIO_ResetBits(GPIOC,GPIO_Pin_11))

#define AD9854_SP_Set (GPIO_SetBits(GPIOC,GPIO_Pin_12))         
#define AD9854_SP_Clr (GPIO_ResetBits(GPIOC,GPIO_Pin_12))

#define AD9854_OSK_Set (GPIO_SetBits(GPIOD,GPIO_Pin_12))         
#define AD9854_OSK_Clr (GPIO_ResetBits(GPIOD,GPIO_Pin_12))

#define AD9854_FDATA_Set (GPIO_SetBits(GPIOD,GPIO_Pin_13))        //fsk/bpsk/hold  
#define AD9854_FDATA_Clr (GPIO_ResetBits(GPIOD,GPIO_Pin_13))


//**************************以下部分为函数定义********************************
extern void GPIO_AD9854_Configuration(void);						// AD9854_IO口初始化
static void AD9854_WR_Byte(u8 addr,u8 dat);	  
extern void AD9854_Init(void);						  
static void Freq_convert(long Freq);	         	  
extern void AD9854_SetSine(u32 Freq,u16 Shape);	  
static void Freq_double_convert(double Freq);		  
extern void AD9854_SetSine_double(double Freq,u16 Shape);
extern void AD9854_InitFSK(void);				
extern void AD9854_SetFSK(u32 Freq1,u32 Freq2);					  
extern void AD9854_InitBPSK(void);					  
extern void AD9854_SetBPSK(u16 Phase1,u16 Phase2);					
extern void AD9854_InitOSK(void);					 
extern void AD9854_SetOSK(u8 RateShape);					  
extern void AD9854_InitAM(void);					 
extern void AD9854_SetAM(u16 Shape);					
extern void AD9854_InitRFSK(void);					 
extern void AD9854_SetRFSK(u32 Freq_Low,u32 Freq_High,u32 Freq_Up_Down,u32 FreRate);				

#endif
