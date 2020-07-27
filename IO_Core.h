#ifndef TIMER_UART_H_
#define TIMER_UART_H_

/************************** Software UART *************************************/
                                                                                // Conditions for 9600 Baud SW UART, SMCLK = 8MHz /8 = 1,000,000
                                                                                // Calculation is Bitime = ((SMCK/Desired BaudRate))
                                                                                // Bitime_5 = (Bitime/2)+1


/************ Software UART Prototype Headers *******************************/
void TX_Byte (void);
void RX_Ready (void);
void send_Byte_IO(unsigned char byte);
unsigned char parityCheck(unsigned char);
unsigned short RX_ISR(void);
void Timer_TX_ISR (void);
void send_ALL_H_NO_START(void);
void send_Byte_IO_nowait(unsigned char byte);
void send_Byte_MSB(unsigned char byte);

typedef struct {
	unsigned char value:8;
	unsigned char parity:1;
}IO_buffer;







#endif /*TIMER_UART_H*/
