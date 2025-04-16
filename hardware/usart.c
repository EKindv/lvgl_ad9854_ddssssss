#include "stm32f10x.h"
#include <string.h>
#include <stdlib.h>
#include "stm32f10x_usart.h"
#include "usart.h"
#include "AD9854.h"
#include "lcd.h" 


#define RX_BUFFER_SIZE 128
volatile char rx_buffer[RX_BUFFER_SIZE];
volatile uint16_t rx_index = 0;
volatile uint8_t rx_complete = 0;

#define PARSE_INT(var, str) do {           \
    char *endptr;                          \
    long tmp = strtol((str), &endptr, 10); \
    if (*endptr != '\0') return;           \
    (var) = (u32)tmp;              \
} while(0)

// USART1?????
void USART1_Init(void) {
    GPIO_InitTypeDef GPIO_InitStruct;
    USART_InitTypeDef USART_InitStruct;
    NVIC_InitTypeDef NVIC_InitStruct;

    // ????
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1 | RCC_APB2Periph_GPIOA, ENABLE);

    // ??USART1??
    // TX: PA9 ????
    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_9;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStruct);
    
    // RX: PA10 ????
    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_10;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOA, &GPIO_InitStruct);

    // USART????
    USART_InitStruct.USART_BaudRate = 115200;
    USART_InitStruct.USART_WordLength = USART_WordLength_8b;
    USART_InitStruct.USART_StopBits = USART_StopBits_1;
    USART_InitStruct.USART_Parity = USART_Parity_No;
    USART_InitStruct.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStruct.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
    USART_Init(USART1, &USART_InitStruct);

    // ??????
    USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);

    // ??NVIC
    NVIC_InitStruct.NVIC_IRQChannel = USART1_IRQn;
    NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStruct.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStruct);

    USART_Cmd(USART1, ENABLE);
}

void USART1_IRQHandler(void) {
    if (USART_GetITStatus(USART1, USART_IT_RXNE) != RESET) {
        char received_char = USART_ReceiveData(USART1);
        	
        if (rx_index < RX_BUFFER_SIZE - 1) {
            if (received_char == '\r' || received_char == '\n') {
                rx_buffer[rx_index] = '\0';
							process_command((char*)rx_buffer);
							rx_complete = 1;
                rx_index = 0;
            } else {
                rx_buffer[rx_index++] = received_char;
            }
        } else {
            rx_index = 0;
        }
    }
}

void process_command(char *cmd) {
	
	    /*---- ???? ----*/
    // SIN????:sin,<??>,<??>
   if (strncmp(cmd, "sin,", 4) == 0) {
        char *frq_str = cmd + 4;
        char *val_str = strchr(frq_str, ',');
        if (!val_str) return;
        *val_str++ = '\0';

        int frq, val;
        PARSE_INT(frq, frq_str); // ???????
        PARSE_INT(val, val_str);
        AD9854_SetSine(frq, val);
    }
    
     else if (strncmp(cmd, "bpsk,", 5) == 0) {
        cmd += 5;
        char *phase1_str = cmd;
        char *phase2_str = strchr(phase1_str, ',');
        if (!phase2_str) return;
        *phase2_str++ = '\0';

        u16 phase1, phase2;
        PARSE_INT(phase1, phase1_str);
        PARSE_INT(phase2, phase2_str);
				AD9854_InitBPSK();
        AD9854_SetBPSK(phase1, phase2);
    }

    // FSK??:fsk,<??1>,<??2>
    else if (strncmp(cmd, "fsk,", 4) == 0) {
    cmd += 4;
    char *freq1_str = cmd;
    char *freq2_str = strchr(freq1_str, ',');
    if (!freq2_str) return;
    *freq2_str++ = '\0';

    u32 freq1, freq2;
    PARSE_INT(freq1, freq1_str);
    PARSE_INT(freq2, freq2_str);
		AD9854_InitFSK();		
    AD9854_SetFSK(freq1, freq2);
}

// OSK??:osk,<RateShape>
else if (strncmp(cmd, "osk,", 4) == 0) {
    cmd += 4;
    char *rate_shape_str = cmd;

    u8 rate_shape;
    PARSE_INT(rate_shape, rate_shape_str);
		AD9854_InitOSK();
    AD9854_SetOSK(rate_shape);
}

// AM??:am,<Shape>
else if (strncmp(cmd, "am,", 3) == 0) {
    cmd += 3;
    char *shape_str = cmd;

    u16 shape;
    PARSE_INT(shape, shape_str);
		AD9854_InitAM();
    AD9854_SetAM(shape);
}

// RFSK??:rfsk,<Freq_Low>,<Freq_High>,<Freq_Up_Down>,<FreRate>
else if (strncmp(cmd, "rfsk,", 5) == 0) {
    cmd += 5;
    char *freq_low_str = cmd;
    char *freq_high_str = strchr(freq_low_str, ',');
    if (!freq_high_str) return;
    *freq_high_str++ = '\0';

    char *freq_up_down_str = strchr(freq_high_str, ',');
    if (!freq_up_down_str) return;
    *freq_up_down_str++ = '\0';

    char *fre_rate_str = strchr(freq_up_down_str, ',');
    if (!fre_rate_str) return;
    *fre_rate_str++ = '\0';

    u32 freq_low, freq_high, freq_up_down, fre_rate;
    PARSE_INT(freq_low, freq_low_str);
    PARSE_INT(freq_high, freq_high_str);
    PARSE_INT(freq_up_down, freq_up_down_str);
    PARSE_INT(fre_rate, fre_rate_str);
	AD9854_InitRFSK();
    AD9854_SetRFSK(freq_low, freq_high, freq_up_down, fre_rate);
}

    // ??????...

    #undef PARSE_INT

/*    if (strncmp(cmd, "sin,", 4) != 0) return;
    char *frq_str = cmd + 4;
    char *val_str = strchr(frq_str, ',');
    if (!val_str) return; 
    
    *val_str = '\0'; 
    val_str++;       
    int frq = atoi(frq_str);
    int val = atoi(val_str);
    AD9854_SetSine(frq, val);*/
}
