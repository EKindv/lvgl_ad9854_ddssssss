#include "w25qx.h"
#include "sys.h"
#include "lcd.h"
#include "touch.h"


//初始化SPI_FLASH
void W25QX_init(void)
{
		RCC->APB2ENR|=1<<2;    //PA时钟使能
		JTAG_Set(SWD_ENABLE);  //开启SWD
	
	  GPIOA->CRL&=0XFFFFFF00;
		GPIOA->CRL|=0X00000083;
		GPIOA->CRH&=0X0FFFFF00;
		GPIOA->CRH|=0X30000033;
		GPIOA->ODR|=0X0303;     
}
//SPI_FLASH 写
void SPI_Write(u8 data) 
{
    uint8_t i;
    for(i=0;i<8;i++)
    {
        F_SCLK=0;
        if(data&0x80)
            F_DIO=1;
        else
            F_DIO=0;
        F_SCLK=1;
        data<<=1;
     }
} 
//SPI_FLASH 读
u8 SPI_Read(void) 
{
    u8 buff=0x00,i;
    for(i=0;i<8;i++)
    {
        F_SCLK=0;
        buff<<=1;
        if(DO_Read)
            buff|=0x01;
        F_SCLK=1;
    }
    return buff;
}
//SPI_FLASH写使能	
//将WEL置位 
void Write_Enable(void) 
{
    F_CS=0;
    SPI_Write(W25X_WriteEnable);
    F_CS=1;
}
//SPI_FLASH写禁止	
//将WEL清零  
void Write_Disable(void) 
{
    F_CS=0;
     SPI_Write(W25X_WriteDisable);
        F_CS=1;
}
//读取SPI_FLASH的状态寄存器
//BIT7  6   5   4   3   2   1   0
//SPR   RV  TB BP2 BP1 BP0 WEL BUSY
//SPR:默认0,状态寄存器保护位,配合WP使用
//TB,BP2,BP1,BP0:FLASH区域写保护设置
//WEL:写使能锁定
//BUSY:忙标记位(1,忙;0,空闲)
//默认:0x00
u8 W25Q16_ReadSR(void) 
{
    uint8_t flag;
    F_CS=0;
    SPI_Write(W25X_ReadStatusReg);
    flag=SPI_Read();
    F_CS=1;
    flag&=0x01;
    return flag;
}

#ifndef MEM_ALLOC_TABLE_SIZE
u8 SPI_FLASH_BUFFER[4096];
#endif 
void W25Q16_Write(uint8_t *data,uint32_t address,u16 NumByteToWrite)
{

	u32 secpos;
	u16 secoff;
	u16 secremain;	   
 	u16 i;    
	u8 * SPI_FLASH_BUF;	  
#ifdef MEM_ALLOC_TABLE_SIZE			
	SPI_FLASH_BUF=mymalloc(4096);	//使用内存管理 
	if(SPI_FLASH_BUF==NULL)return;	//申请失败
#else
	SPI_FLASH_BUF=SPI_FLASH_BUFFER;	//不使用内存管理
#endif	
 	secpos=address/4096;//扇区地址  
	secoff=address%4096;//在扇区内的偏移
	secremain=4096-secoff;//扇区剩余空间大小   
 	//printf("ad:%X,nb:%X\r\n",WriteAddr,NumByteToWrite);//测试用
 	if(NumByteToWrite<=secremain)secremain=NumByteToWrite;//不大于4096个字节
	while(1) 
	{	
		W25Q16_Read(SPI_FLASH_BUF,secpos*4096,4096);//读出整个扇区的内容
		for(i=0;i<secremain;i++)//校验数据
		{
			if(SPI_FLASH_BUF[secoff+i]!=0XFF)break;//需要擦除  	  
		}
		if(i<secremain)//需要擦除
		{
			W25QX_Erase_Sector(secpos);//擦除这个扇区
			for(i=0;i<secremain;i++)	   //复制
			{
				SPI_FLASH_BUF[i+secoff]=data[i];	  
			}
			W25QX_Write_NoCheck(SPI_FLASH_BUF,secpos*4096,4096);//写入整个扇区  

		}else W25QX_Write_NoCheck(data,address,secremain);//写已经擦除了的,直接写入扇区剩余区间. 
		if(NumByteToWrite==secremain)
		{
			//myfree(SPI_FLASH_BUF);
			break;//写入结束了
		}
		else//写入未结束
		{
			secpos++;//扇区地址增1
			secoff=0;//偏移位置为0 	 

		   	data+=secremain;  //指针偏移
			address+=secremain;//写地址偏移	   
		   	NumByteToWrite-=secremain;				//字节数递减
			if(NumByteToWrite>4096)secremain=4096;	//下一个扇区还是写不完
			else secremain=NumByteToWrite;			//下一个扇区可以写完了
		}	 
	};		
	
#ifdef MEM_ALLOC_TABLE_SIZE			
	myfree(SPI_FLASH_BUF);		//释放内存	 	 
#endif	
		
}
//无检验写SPI FLASH 
//必须确保所写的地址范围内的数据全部为0XFF,否则在非0XFF处写入的数据将失败!
//具有自动换页功能 
//在指定地址开始写入指定长度的数据,但是要确保地址不越界!
//pBuffer:数据存储区
//WriteAddr:开始写入的地址(24bit)
//NumByteToWrite:要写入的字节数(最大65535)
//CHECK OK
void W25QX_Write_NoCheck(u8* data,u32 WriteAddr,u16 NumByteToWrite)   
{ 			 		 
	u16 pageremain;	   
	pageremain=256-WriteAddr%256; //单页剩余的字节数		 	    
	if(NumByteToWrite<=pageremain)pageremain=NumByteToWrite;//不大于256个字节
	while(1)
	{	   
		W25QX_Flash_Write_Page(data,WriteAddr,pageremain);
		if(NumByteToWrite==pageremain)break;//写入结束了
	 	else //NumByteToWrite>pageremain
		{
			data+=pageremain;
			WriteAddr+=pageremain;	

			NumByteToWrite-=pageremain;			  //减去已经写入了的字节数
			if(NumByteToWrite>256)pageremain=256; //一次可以写入256个字节
			else pageremain=NumByteToWrite; 	  //不够256个字节了
		}
	};	    
}
//SPI在一页(0~65535)内写入少于256个字节的数据
//在指定地址开始写入最大256字节的数据
//pBuffer:数据存储区
//WriteAddr:开始写入的地址(24bit)
//NumByteToWrite:要写入的字节数(最大256),该数不应该超过该页的剩余字节数!!!	 
void W25QX_Flash_Write_Page(u8* pBuffer,u32 WriteAddr,u16 NumByteToWrite)
{
 	u16 i;  
    Write_Enable();                  //SET WEL 
		F_CS=0;                            //使能器件   
    SPI_Write(W25X_PageProgram);      //发送写页命令   
    SPI_Write((u8)((WriteAddr)>>16)); //发送24bit地址    
    SPI_Write((u8)((WriteAddr)>>8));   
    SPI_Write((u8)WriteAddr);   
    for(i=0;i<NumByteToWrite;i++)SPI_Write(pBuffer[i]);//循环写数  
		F_CS=1;                           //取消片选 
		while(W25Q16_ReadSR()); 					   //等待写入结束
} 
//擦除一个扇区
//Dst_Addr:扇区地址 根据实际容量设置
//擦除一个山区的最少时间:150ms
void W25QX_Erase_Sector(u32 Dst_Addr)   
{    
		Dst_Addr*=4096;
    Write_Enable();                  //SET WEL 	 
    while(W25Q16_ReadSR());   
  	F_CS=0;                            //使能器件   
    SPI_Write(W25X_SectorErase);      //发送扇区擦除指令 
    SPI_Write((u8)((Dst_Addr)>>16));  //发送24bit地址    
    SPI_Write((u8)((Dst_Addr)>>8));   
    SPI_Write((u8)Dst_Addr);  
		F_CS=1;                            //取消片选     	      
    while(W25Q16_ReadSR());   				   //等待擦除完成
}
//读取SPI FLASH  
//在指定地址开始读取指定长度的数据
//data:数据存储区
//address:开始读取的地址(24bit)
//NumByteToRead:要读取的字节数(最大65535)
void W25Q16_Read(uint8_t *data,uint32_t address,u16 NumByteToRead)
{
    u16 i;
    while(W25Q16_ReadSR());
    F_CS=0;
    SPI_Write(W25X_ReadData);
    SPI_Write(address>>16);
    SPI_Write(address>>8);
    SPI_Write(address);
    for(i=0;i<NumByteToRead;i++)
    {
        data[i]=SPI_Read();
    }
    F_CS=1;
}
//擦除整个芯片		  
//等待时间超长...
void W25q16_Erasure()
{
    Write_Enable();
    SPI_Write(0x00);
    Write_Enable();
    while(W25Q16_ReadSR());
    F_CS=0;
    SPI_Write(W25X_ChipErase);
    F_CS=1;
    while(W25Q16_ReadSR());
}
//读取芯片ID
//返回值如下:				   
//0XEF13,表示芯片型号为W25Q80  
//0XEF14,表示芯片型号为W25Q16    
//0XEF15,表示芯片型号为W25Q32  
//0XEF16,表示芯片型号为W25Q64 
u16 W25QX_Flash_ReadID(void)
{
	u16 Temp=0;
	F_CS=0;
	SPI_Write(0X90);
	SPI_Write(0X00);
	SPI_Write(0X00);
	SPI_Write(0X00);
	Temp|=(SPI_Read()<<8);
	Temp|=SPI_Read();
	F_CS=1;

	return Temp;
}

