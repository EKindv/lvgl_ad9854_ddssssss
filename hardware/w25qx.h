#ifndef __W25QX_H
#define __W25QX_H	
#include <stm32f10x.h> 
#include "sys.h"

#define SPI_FLASH_TYPE 0XEF15

#define W25Q80 	0XEF13 	
#define W25Q16 	0XEF14
#define W25Q32 	0XEF15
#define W25Q64 	0XEF16

//指令表
#define W25X_WriteEnable		0x06 
#define W25X_WriteDisable		0x04 
#define W25X_ReadStatusReg		0x05 
#define W25X_WriteStatusReg		0x01 
#define W25X_ReadData			0x03 
#define W25X_FastReadData		0x0B 
#define W25X_FastReadDual		0x3B 
#define W25X_PageProgram		0x02 
#define W25X_BlockErase			0xD8 
#define W25X_SectorErase		0x20 
#define W25X_ChipErase			0xC7 
#define W25X_PowerDown			0xB9 
#define W25X_ReleasePowerDown	0xAB 
#define W25X_DeviceID			0xAB 
#define W25X_ManufactDeviceID	0x90 
#define W25X_JedecDeviceID		0x9F 

#define F_CS     PAout(9)
#define F_SCLK   PAout(8)       
#define F_DIO    PAout(0) 
#define DO_Read  PAin(1)

void SPI_Write(u8 data);
u8 SPI_Read(void);
void Write_Enable(void);//写使能
void Write_Disable(void);//写保护
u8 W25Q16_ReadSR(void); //读取状态寄存器
void W25QX_Erase_Sector(u32 Dst_Addr);//擦除一个扇区
void W25Q16_Write(uint8_t *data,uint32_t address,u16 NumByteToWrite);//写入flash
void W25Q16_Read(uint8_t *data,uint32_t address,u16 NumByteToRead);//读取flash
void W25QX_Flash_Write_Page(u8* pBuffer,u32 WriteAddr,u16 NumByteToWrite);//
void W25QX_Write_NoCheck(u8* data,u32 WriteAddr,u16 NumByteToWrite);//无检验写SPI FLASH 
void W25q16_Erasure(void);//整片擦除
void W25QX_init(void);//
u16 W25QX_Flash_ReadID(void);//


#endif
