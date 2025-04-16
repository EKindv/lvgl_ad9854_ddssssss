#ifndef __MYCT_IIC_H
#define __MYCT_IIC_H
#include "sys.h"	    

 	 
// I�C ????
#define CT_IIC_SCL    PBout(6)   // SCL ??
#define CT_IIC_SDA    PBout(7)    // SDA ??
#define CT_READ_SDA   PBin(7)     // SDA ??

// SDA ????(?????)
#define CT_SDA_IN()  { GPIOB->CRL &= ~(0xF << 28); GPIOB->CRL |= (0x4 << 28); }
#define CT_SDA_OUT() { GPIOB->CRL &= ~(0xF << 28); GPIOB->CRL |= (0x5 << 28); }
 

//IIC���в�������
void CT_IIC_Init(void);                	//��ʼ��IIC��IO��				 
void CT_IIC_Start(void);				//����IIC��ʼ�ź�
void CT_IIC_Stop(void);	  				//����IICֹͣ�ź�
void CT_IIC_Send_Byte(u8 txd);			//IIC����һ���ֽ�
u8 CT_IIC_Read_Byte(unsigned char ack);	//IIC��ȡһ���ֽ�
u8 CT_IIC_Wait_Ack(void); 				//IIC�ȴ�ACK�ź�
void CT_IIC_Ack(void);					//IIC����ACK�ź�
void CT_IIC_NAck(void);					//IIC������ACK�ź�

#endif







