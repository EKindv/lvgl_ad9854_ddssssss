#ifndef __CTP_H
#define __CTP_H	
#include "sys.h"	



//与电容触摸屏连接的芯片引脚(未包含IIC引脚) 
//IO操作函数	 
#define FT_RST    				PEout(10)	//FT6336复位引脚
#define FT_INT    				PEin(8)		//FT6336中断引脚	


//I2C读写命令	
#define FT_CMD_WR 				0X70    	//写命令
#define FT_CMD_RD 				0X71		//读命令
  
//FT6336 部分寄存器定义 
#define FT_REG_NUM_FINGER       0x02		//触摸状态寄存器

#define FT_TP1_REG 				0X03	  //第一个触摸点数据地址
#define FT_TP2_REG 				0X09		//第二个触摸点数据地址
#define FT_TP3_REG 				0X0F		//第三个触摸点数据地址
#define FT_TP4_REG 				0X15		//第四个触摸点数据地址
#define FT_TP5_REG 				0X1B		//第五个触摸点数据地址  
 


u8 FT6336_WR_Reg(u16 reg,u8 *buf,u8 len);
void FT6336_RD_Reg(u16 reg,u8 *buf,u8 len);
void FT6336_Init(void);
u8 FT6336_Scan(u8 mode);
u8 Read_XY(u16 *x, u16 *y);
u8 Check_Touch_Event(void);

#endif
