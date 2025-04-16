#ifndef _USART_H_
#define _USART_H_

#include <stm32f10x.h>

// ????????
#define RX_BUFFER_SIZE 128

// ????????????
extern volatile char rx_buffer[RX_BUFFER_SIZE];
extern volatile uint16_t rx_index;
extern volatile uint8_t rx_complete;

void USART1_Init(void);
void process_command(char *cmd);

#endif
