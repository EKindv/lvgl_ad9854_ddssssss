#ifndef __DELAY_H
#define __DELAY_H 			   
#include "sys.h"  

void delay_init(u8 SYSCLK);
void delay_ms(u16 nms);
void delay_us(u32 nus);
extern void TimingDelay_Decrement(void);
extern void Delay_ns (u8 t);			// ns��ʱ
extern void Delay_1us (u8 t);							// ��ʱʱ��:1us
extern void Delay_2us (u16 t);							// ��ʱʱ��:2us
extern void Delay_10us (u8 t);   					// ��ʱʱ��:10us
extern void Delay_882us (void);							// ��ʱ822us
extern void Delay_250us (u8 t);  // ��ʱʱ��:250us
extern void Delay_5ms (u8 t);							// ��ʱʱ��:5ms
extern void Delay_1ms (u8 t);		// ��ʱʱ��:1ms
extern void Delay_50ms (u8 t);		// ��ʱʱ��:50ms
extern void Delay(__IO uint32_t nCount);

#endif





























