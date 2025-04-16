#include "delay.h"
////////////////////////////////////////////////////////////////////////////////// 	 
//如果需要使用OS,则包括下面的头文件即可.
#if SYSTEM_SUPPORT_OS
#include "includes.h"					//ucos 使用	  
#endif


static u8  fac_us=0;							//us延时倍乘数			   
static u16 fac_ms=0;							//ms延时倍乘数,在ucos下,代表每个节拍的ms数
	
	
#if SYSTEM_SUPPORT_OS							//如果SYSTEM_SUPPORT_OS定义了,说明要支持OS了(不限于UCOS).
//当delay_us/delay_ms需要支持OS的时候需要三个与OS相关的宏定义和函数来支持
//首先是3个宏定义:
//    delay_osrunning:用于表示OS当前是否正在运行,以决定是否可以使用相关函数
//delay_ostickspersec:用于表示OS设定的时钟节拍,delay_init将根据这个参数来初始哈systick
// delay_osintnesting:用于表示OS中断嵌套级别,因为中断里面不可以调度,delay_ms使用该参数来决定如何运行
//然后是3个函数:
//  delay_osschedlock:用于锁定OS任务调度,禁止调度
//delay_osschedunlock:用于解锁OS任务调度,重新开启调度
//    delay_ostimedly:用于OS延时,可以引起任务调度.

//本例程仅作UCOSII和UCOSIII的支持,其他OS,请自行参考着移植
//支持UCOSII
#ifdef 	OS_CRITICAL_METHOD						//OS_CRITICAL_METHOD定义了,说明要支持UCOSII				
#define delay_osrunning		OSRunning			//OS是否运行标记,0,不运行;1,在运行
#define delay_ostickspersec	OS_TICKS_PER_SEC	//OS时钟节拍,即每秒调度次数
#define delay_osintnesting 	OSIntNesting		//中断嵌套级别,即中断嵌套次数
#endif

//支持UCOSIII
#ifdef 	CPU_CFG_CRITICAL_METHOD					//CPU_CFG_CRITICAL_METHOD定义了,说明要支持UCOSIII	
#define delay_osrunning		OSRunning			//OS是否运行标记,0,不运行;1,在运行
#define delay_ostickspersec	OSCfg_TickRate_Hz	//OS时钟节拍,即每秒调度次数
#define delay_osintnesting 	OSIntNestingCtr		//中断嵌套级别,即中断嵌套次数
#endif


//us级延时时,关闭任务调度(防止打断us级延迟)
void delay_osschedlock(void)
{
#ifdef CPU_CFG_CRITICAL_METHOD   			//使用UCOSIII
	OS_ERR err; 
	OSSchedLock(&err);						//UCOSIII的方式,禁止调度，防止打断us延时
#else										//否则UCOSII
	OSSchedLock();							//UCOSII的方式,禁止调度，防止打断us延时
#endif
}

//us级延时时,恢复任务调度
void delay_osschedunlock(void)
{	
#ifdef CPU_CFG_CRITICAL_METHOD   			//使用UCOSIII
	OS_ERR err; 
	OSSchedUnlock(&err);					//UCOSIII的方式,恢复调度
#else										//否则UCOSII
	OSSchedUnlock();						//UCOSII的方式,恢复调度
#endif
}

//调用OS自带的延时函数延时
//ticks:延时的节拍数
void delay_ostimedly(u32 ticks)
{
#ifdef CPU_CFG_CRITICAL_METHOD
	OS_ERR err; 
	OSTimeDly(ticks,OS_OPT_TIME_PERIODIC,&err);//UCOSIII延时采用周期模式
#else
	OSTimeDly(ticks);						//UCOSII延时
#endif 
}
 
//systick中断服务函数,使用OS时用到
void SysTick_Handler(void)
{	
	if(delay_osrunning==1)					//OS开始跑了,才执行正常的调度处理
	{
		OSIntEnter();						//进入中断
		OSTimeTick();       				//调用ucos的时钟服务程序               
		OSIntExit();       	 				//触发任务切换软中断
	}
}
#endif
			   
//初始化延迟函数
//当使用OS的时候,此函数会初始化OS的时钟节拍
//SYSTICK的时钟固定为HCLK时钟的1/8
//SYSCLK:系统时钟
void delay_init(u8 SYSCLK)
{
#if SYSTEM_SUPPORT_OS 						//如果需要支持OS.
	u32 reload;
#endif
 	SysTick->CTRL&=~(1<<2);					//SYSTICK使用外部时钟源	 
	fac_us=SYSCLK/8;						//不论是否使用OS,fac_us都需要使用
#if SYSTEM_SUPPORT_OS 						//如果需要支持OS.
	reload=SYSCLK/8;						//每秒钟的计数次数 单位为K	   
	reload*=1000000/delay_ostickspersec;	//根据delay_ostickspersec设定溢出时间
											//reload为24位寄存器,最大值:16777216,在72M下,约合1.86s左右	
	fac_ms=1000/delay_ostickspersec;		//代表OS可以延时的最少单位	   
	SysTick->CTRL|=1<<1;   					//开启SYSTICK中断
	SysTick->LOAD=reload; 					//每1/delay_ostickspersec秒中断一次	
	SysTick->CTRL|=1<<0;   					//开启SYSTICK    
#else
	fac_ms=(u16)fac_us*1000;				//非OS下,代表每个ms需要的systick时钟数   
#endif
}								    

#if SYSTEM_SUPPORT_OS 						//如果需要支持OS.
//延时nus
//nus为要延时的us数.		    								   
void delay_us(u32 nus)
{		
	u32 ticks;
	u32 told,tnow,tcnt=0;
	u32 reload=SysTick->LOAD;				//LOAD的值	    	 
	ticks=nus*fac_us; 						//需要的节拍数 
	delay_osschedlock();					//阻止OS调度，防止打断us延时
	told=SysTick->VAL;        				//刚进入时的计数器值
	while(1)
	{
		tnow=SysTick->VAL;	
		if(tnow!=told)
		{	    
			if(tnow<told)tcnt+=told-tnow;	//这里注意一下SYSTICK是一个递减的计数器就可以了.
			else tcnt+=reload-tnow+told;	    
			told=tnow;
			if(tcnt>=ticks)break;			//时间超过/等于要延迟的时间,则退出.
		}  
	};
	delay_osschedunlock();					//恢复OS调度									    
}
//延时nms
//nms:要延时的ms数
void delay_ms(u16 nms)
{	
	if(delay_osrunning&&delay_osintnesting==0)//如果OS已经在跑了,并且不是在中断里面(中断里面不能任务调度)	    
	{		 
		if(nms>=fac_ms)						//延时的时间大于OS的最少时间周期 
		{ 
   			delay_ostimedly(nms/fac_ms);	//OS延时
		}
		nms%=fac_ms;						//OS已经无法提供这么小的延时了,采用普通方式延时    
	}
	delay_us((u32)(nms*1000));				//普通方式延时  
}
#else //不用OS时
//延时nus
//nus为要延时的us数.		    								   
void delay_us(u32 nus)
{		
	u32 temp;	    	 
	SysTick->LOAD=nus*fac_us; 				//时间加载	  		 
	SysTick->VAL=0x00;        				//清空计数器
	SysTick->CTRL=0x01 ;      				//开始倒数 	 
	do
	{
		temp=SysTick->CTRL;
	}while((temp&0x01)&&!(temp&(1<<16)));	//等待时间到达   
	SysTick->CTRL=0x00;      	 			//关闭计数器
	SysTick->VAL =0X00;       				//清空计数器	 
}
//延时nms
//注意nms的范围
//SysTick->LOAD为24位寄存器,所以,最大延时为:
//nms<=0xffffff*8*1000/SYSCLK
//SYSCLK单位为Hz,nms单位为ms
//对72M条件下,nms<=1864 
void delay_ms(u16 nms)
{	 		  	  
	u32 temp;		   
	SysTick->LOAD=(u32)nms*fac_ms;			//时间加载(SysTick->LOAD为24bit)
	SysTick->VAL =0x00;           			//清空计数器
	SysTick->CTRL=0x01 ;          			//开始倒数  
	do
	{
		temp=SysTick->CTRL;
	}while((temp&0x01)&&!(temp&(1<<16)));	//等待时间到达   
	SysTick->CTRL=0x00;      	 			//关闭计数器
	SysTick->VAL =0X00;       				//清空计数器	  	    
} 

//-----------------------------------------------------------------
// 功能程序区
//-----------------------------------------------------------------
//-----------------------------------------------------------------
// void TimingDelay_Decrement(void)
//-----------------------------------------------------------------
// 
// 函数功能: 延时计数器递减函数
// 入口参数: 无 
// 返回参数: 无
// 全局变量: 无
// 调用模块: 无
// 注意事项: 此函数被中断函数调用
//					 时钟嘀嗒使用系统时钟源72MHz,
//
//-----------------------------------------------------------------
void TimingDelay_Decrement(void)
{
}
//-----------------------------------------------------------------
// void Delay_ns (unsigned char t)
//-----------------------------------------------------------------
//
// 函数功能: 时基为ns的延时
// 入口参数: 无符号8bit整数
// 返回参数: 无
// 全局变量: 无
// 调用模块: 无
// 注意事项:  
//
//测得延时如下：(SYSCLK=72MHz) 
//													延时个数		延时				误差
//				Delay_ns(1):       17       236ns				500ns
//				Delay_ns(2):	     26       361ns				550ns
//				Delay_ns(10)														1.625us
//
//-----------------------------------------------------------------
void Delay_ns (u8 t)
{
	do {
			;
	} while (--t); 
}

//-----------------------------------------------------------------
// void Delay_1us (unsigned char t)
//-----------------------------------------------------------------
// 函数功能: 时基为1us的延时
// 入口参数: 无符号8bit整数
// 返回参数: 无
// 全局变量: 无
// 调用模块: 无
// 注意事项:  时钟周期：1/72mhz
//						目标1us:-->要72个时钟周期
//						-->要观察执行了Delay_1us函数前后的state差是否为72个周期
//						-->通过多次更改，使得误差越来越小
/*
//i=7,测得延时如下：(SYSCLK=72MHz) 
												延时个数		   延时			  	误差
		Delay_1us(1):	  	     70				 972ns				28ns		
		Delay_1us(2):	  	 	  131				1.819us				181ns
		Delay_1us(10):		    619			 8.5972us				1.4us
		Delay_1us(20):			 1229			 17.069us				2.9us
		Delay_1us(100): 	   6109				84.84us				15.16us
		
		i=4!!!					i=9				i=8
		1us：1us				1.98us!		1.88us
		5us:3,22us			5.6us			5.16us	
		10us:6us				10.86us		9.89us
		50us:28.22us		52.5us		47.6us
		100us:					104.6us   95.4us
		200us:					209us			189.5
		500us:					522us 		478us
		结论:i=9,误差+4.6%，1us延时误差大
				 i=8,误差-4.6%，1us延时误差大
*/
//-----------------------------------------------------------------
void Delay_1us (u8 t)
{
	u8 i=0;
	
	do
	{
		i=8;											// i=7,t=0.986us,误差14ns
		do
		{
		} while (--i);	
	}while(--t);
}

//-----------------------------------------------------------------
// void Delay_2us (u16 t)
//-----------------------------------------------------------------
//
// 函数功能: 时基为2us的延时
// 入口参数: 无符号8bit整数
// 返回参数: 无
// 全局变量: 无
// 调用模块: 无
// 注意事项:  
/*
		i=14，实际比理论小3%
		i=15，实际比理论大3%
		i=14,	理论：实际 		 i=15, 实际			 i=13
					2us   2.36us					2.5us		 2.2
					4us	  4.24us				  4.5us		 4us
					6us   6.2us						6.6us		 5.8us
					8us   8.1us						8.6us		 7.6us
					10us	10us						10.6us	 9.4us
					20us  19.6us				  20.8us	 18.4us
					50us	49us					  52us		 45.2us
					100us	97us						103us		 90us
					200us 192us						204us		 180us
					400us	384us	 					408us		 360us
					500us	480us						510us    448us
					1ms	  950us         1.020ms		 890us
*/
//-----------------------------------------------------------------
void Delay_2us (u16 t)
{
	u8 i=0;
	
	do
	{
		i=15;																		// i=7,t=0.986us,误差14ns
		do
		{
		} while (--i);	
	}while(--t);
}

//-----------------------------------------------------------------
// void Delay_10us (u8 t)
//-----------------------------------------------------------------
//
// 函数功能: 时基为10us的延时
// 入口参数: 无符号8bit整数
// 返回参数: 无
// 全局变量: 无
// 调用模块: 无
// 注意事项:  
/* 
//j=7,u=11,测得延时如下：(SYSCLK=72MHz) 
												延时个数						延时				误差  实际
			Delay_10us(1):	      731  				10.153us		153ns 	10.45
			Delay_10us(2):	   	 1454 				20.194us		194ns		20.5
			Delay_10us(10):		   7238 			 100.527us		527ns		101.8
			Delay_10us(20):			14468			 	 200.944us 		944ns  	201.4
			Delay_10us(100): 	  72338				1004.694us		  4us		1.004ms
*/
//-----------------------------------------------------------------
void Delay_10us (u8 t)
{
	u16 i,j;
   
	do {
		j = 7;						// j=6,i=13,737,10.236us,误差236ns
		do {							// j=7,i=11,731,10.152us,误差152ns
			i = 11; 
			do {									 
       } while (--i);	
		} while (--j);		
	} while (--t); 
}	

//-----------------------------------------------------------------
// void Delay_250us (u8 t)
//-----------------------------------------------------------------
//
// 函数功能: 时基为250us的延时
// 入口参数: 无符号8bit整数
// 返回参数: 无
// 全局变量: 无
// 调用模块: 无
// 注意事项:  
/* 
//j=66,u=30,测得延时如下：(SYSCLK=72MHz) 
														延时个数	    	延时				误差	实际
			Delay_250us(1):	        18035		250.486us			486ns		251us
			Delay_250us(2):	   		  36062		500.861us			861ns		501us
			Delay_250us(10):	  	 180338		  2.504ms				4us		2500us
			Delay_250us(20):	   	 360698 	  5.009ms				9us	 	5.01ms
			Delay_250us(100):   	1803428		  25.047ms	 	 47us		25.05ms
*/
//-----------------------------------------------------------------
void Delay_250us (u8 t)
{
	unsigned char i,j;
   
	do {
		j = 66;						// j=66,i=30, 18035,250.486us
		do {
			i = 30;
			do {									
         } while (--i);	
		} while (--j);		
	} while (--t); 
}

//-----------------------------------------------------------------
// void Delay_882us (void)
//-----------------------------------------------------------------
//
// 函数功能: 延时882us
// 入口参数: 无
// 返回参数: 无
// 全局变量: 无
// 调用模块: 无     
// 注意事项: 延时时间为880us,误差为2us,实际测试：882us，误差0
//-----------------------------------------------------------------
void Delay_882us (void)			  			
{	
	u16 i,j;   
	j = 101;						// j=101,i=88,63431,880.986us
	do {
		i = 88;
		do {									 
		} while (--i);	
	} while (--j);	
}

//-----------------------------------------------------------------
// void Delay_1ms (unsigned char t)
//-----------------------------------------------------------------
//
// 函数功能: 时基为1ms的延时
// 入口参数: 无符号8bit整数
// 返回参数: 无
// 全局变量: 无
// 调用模块: 无
// 注意事项: 
/* 			 
   手工延时：
   j=119,u=67,测得延时如下：(SYSCLK=72MHz) 
														延时个数					延时			误差
				Delay_1ms(1):	       72158				1.002ms		 	 2us	
				Delay_1ms(2):	   		 144304				2.004ms			 4us
				Delay_1ms(5):	     	 360752				5.010ms			10us
				Delay_1ms(10):	  	 721498			 10.020ms			20us		
				Delay_1ms(20):	   	1442989			 20.041ms			41us				
				Delay_1ms(40):	  	2885874			 40.082ms 		82us	
				Delay_1ms(100):   	7214936			100.207ms			207us	
*/
//-----------------------------------------------------------------
void Delay_1ms (u8 t)
{
	u16 i,j;
   
	do {
		j = 119;	
		do {
			i = 67;																// 1.002278ms ,误差2us
			do {} while (--i);										// 
		} while (--j);		
	} while (--t); 
}

//-----------------------------------------------------------------
// void Delay_5ms (unsigned char t)
//-----------------------------------------------------------------
//
// 函数功能: 时基为5ms的延时
// 入口参数: 无符号8bit整数
// 返回参数: 无
// 全局变量: 无
// 调用模块: 无
// 注意事项:  
/* 
   j=625,u=63,测得延时如下：(SYSCLK=72MHz) 
													  	延时个数		  	延时			误差
				Delay_5ms(1):	        360166			5.0023ms		 2us	
				Delay_5ms(2):	   			720324			10.004ms 		 4us
				Delay_5ms(5):	     		1800795			25.011ms		11us			
				Delay_5ms(10):	  		3601588			50.022ms		22us	
				Delay_5ms(20):	   		7203168		 100.044ms		44us	
				Delay_5ms(40):	  	 14406118		 200.084ms		84us
				Delay_5ms(100):   	 36015742		 500.218ms	 218us		
*/
//-----------------------------------------------------------------
void Delay_5ms (u8 t)
{
	u16 i,j;
   
	do {
		j = 625;	//j=625,u=63,误差2.30us
		do {
			i =63; 
			do {			
		   } while (--i);
		} while (--j);		
	} while (--t); 
}

//-----------------------------------------------------------------
// void Delay_50ms (u8 t)
//-----------------------------------------------------------------
//
// 函数功能: 时基为50ms的延时
// 例子提示: 调用Delay_50ms(20),得到1s延时
// 入口参数: 无符号8bit整数
// 返回参数: 无
// 全局变量: 无
// 调用模块: 无
// 注意事项:  
/* 
  j=1000,i=513,测得以下延时: (SYSCLK=72MHz)
													时钟个数					延时			 误差
			Delay_50ms(1):	    3599596	  		49.994ms			 6us
			Delay_50ms(2):	    7199150				99.988ms			12us
			Delay_50ms(5):	    17997911			249.970ms			30us
			Delay_50ms(10):	  	35995845			499.942ms			58us
			Delay_50ms(20):	   	71991715			999.885ms			115us
			Delay_50ms(40):	  	143983456		 1999.770ms			230us
			Delay_50ms(100):  	359958683		 4999.426ms		  574us
*/
//-----------------------------------------------------------------
void Delay_50ms (u8 t)
{
	u16 i,j;
   
	do {
		j = 1000;					// j=1000,i=513-->6us
		do {
			i = 513;
			do 
			{										
      } while (--i);
		} while (--j);		
	} while (--t); 
}

//-----------------------------------------------------------------
// void Delay(__IO uint32_t nCount)
//-----------------------------------------------------------------
// 
// 函数功能: 粗略延时
// 入口参数: 延时长度 
// 返回参数: 无
// 注意事项: 
//   -__IO 就是volatile, 加上这个后可以避免延迟函数被编译器优化掉
//				一个系统时钟:1/72MHZ=13.89ns,经过23个周期:319ns
/*
  	j=1000,i=513,测得以下延时: (SYSCLK=72MHz)
										时钟个数					延时				 误差		实际
			Delay(1):		    22						305ns								639ns
			Delay(2):		    29					  402ns								736ns
			Delay(5):		    50						694ns								1.028us
			Delay(10):	  	85				 1.1805us								1.514us
			Delay(20):	   155	 			 2.1527us								2.486us
			Delay(40):	   295				 4.0972us								4.4305us
			Delay(100):	   715				 9.9305us								10.26us
*/
//-----------------------------------------------------------------
void Delay(__IO uint32_t nCount)
{
  for(; nCount != 0; nCount--);// 23个
}

//-----------------------------------------------------------------
// End Of File
//-----------------------------------------------------------------



#endif 




























