#include "IO_Core.h"
#include "msp430.h"

/************ Software UART Global Variables *******************************/
unsigned int  TXData;
unsigned char BitCnt;
unsigned char UART_Busy = 0;
unsigned char Data_Trigger = 0;
unsigned int rxBuffer=0;
unsigned char parityBit=0;
volatile IO_buffer recieve_Buffer[32];

unsigned char TXFlag = 0;
unsigned short Time_next_bit_5 = 0x2E8;
unsigned short Time_next_bit = 0x5D0;

unsigned char buffer_index=0;
unsigned char count=0;

unsigned char buffer[16];

void send_Byte_IO(unsigned char byte)
{
	//P2OUT &= ~BIT0;
	//P2OUT |= BIT0;
	//P2DIR |= BIT0;
	//TA1CCTL1 &= ~CCIE;
	BitCnt = 0xE;						        // 10 Bits to send

    TXData = byte;                         		// Load global variable
    //TXData &= ~(0x100); 					// Add start bit
    
    //TXData <<= 1;                                 // Add space for stop bit
    TXData |= parityCheck(byte)<<8;                           	// Add parity bit
    TXData |= 0x1E00;
    TXData <<=1;
    //TXData |= 0x01;
    
    TA1CCR0 = TA1R;                           			// Current state of TA counter
    TA1CCR0 += Time_next_bit;                   		// One bit time till first bit
    //TA1CCTL0 = CCIE;
    TA1CCTL0 = CCIS0 + OUTMOD0 + CCIE;                  // Set TXD on EQU0, Int

    while (TA1CCTL0 & CCIE);							// Wait for communication to finish



}
void send_Byte_IO_nowait(unsigned char byte)
{
	TXFlag= 1;
	P2DIR |= BIT0;
	BitCnt = 0xA;						        // 10 Bits to send

    TXData = byte;                         		// Load global variable
    //TXData &= ~(0x100); 					// Add start bit

    //TXData <<= 1;                                 // Add space for stop bit
    TXData |= parityCheck(byte)<<8;                           	// Add parity bit
    TXData |= 0x1E00;
    TXData <<=1;
    //TXData |= 0x01;

    TA1CCR0 = TA1R;                           			// Current state of TA counter
    TA1CCR0 += Time_next_bit;                   		// One bit time till first bit

    TA1CCTL0 = CCIS0 + OUTMOD0 + CCIE;                  // Set TXD on EQU0, Int
    TXFlag = 0;
}

void send_ALL_H_NO_START(void)
{
	while (TA1CCTL0 & CCIE);							// Wait for communication to finish
	BitCnt = 0xE;						        // 10 Bits to send
	TXData = 0x3FFF;

	TA1CCR0 = TA1R;                           			// Current state of TA counter
	TA1CCR0 += Time_next_bit;                   		// One bit time till first bit

	TA1CCTL0 = CCIS0 + OUTMOD0 + CCIE;                  // Set TXD on EQU0, Int
	while (TA1CCTL0 & CCIE);							// Wait for communication to finish

}

void send_Byte_MSB(unsigned char byte)
{
	char i = 0;

	unsigned char byte_flip=0;
	for(i=0;i<7;i++)
	{
		if(byte & 0x80)
		{
			byte_flip |= 0x80;
		}
		byte <<= 1;
		byte_flip >>= 1;
	}

	while (TA1CCTL0 & CCIE);							// Wait for communication to finish
	BitCnt = 0xB;						        // 10 Bits to send

    TXData = byte;                         		// Load global variable
    //TXData &= ~(0x100); 					// Add start bit

    //TXData <<= 1;                                 // Add space for stop bit
    TXData |= parityCheck(byte)<<8;                           	// Add parity bit
    TXData |= 0x200;
    TXData <<=1;
    //TXData |= 0x01;

    TA1CCR0 = TA1R;                           			// Current state of TA counter
    TA1CCR0 += Time_next_bit;                   		// One bit time till first bit

    TA1CCTL0 = CCIS0 + OUTMOD0 + CCIE;                  // Set TXD on EQU0, Int

}




unsigned char parityCheck(unsigned char byte)
{
	unsigned char parity=0;
	while(byte)
	{
		parity^=BIT0;
		byte&=(byte-1);
	}
	return parity;


}

void Timer_TX_ISR (void)
{
	TA1CCR0 += Time_next_bit;                       // Add Offset to CCR0
	if (TA1CCTL0 & CCIS0)                           // TX on CCI0B?
	{
		if ( BitCnt == 0)
		{
			P2OUT |= BIT0;
			//P2DIR &= ~BIT0;
			//TA1CCTL1 |= CCIE;
			TA1CCTL0 &= ~ CCIE;                     // All bits TXed, disable interrupt
		}
		else
		{
			TA1CCTL0 |=  OUTMOD2;                   // TX Space
			if (TXData & 0x01)
			{
				//P2DIR &= ~BIT0;
				TA1CCTL0 &= ~ OUTMOD2;              // TX Mark
			}
			//else
				//P2DIR |= BIT0;
			TXData = TXData >> 1;
			BitCnt --;
		}
	}
}


unsigned short RX_ISR(void)
{
	static unsigned char rxBitCnt = 9;
	static unsigned char rxData = 0;
	//recieve_Buffer.parity = 0;

	switch (__even_in_range(TA1IV, TA1IV_TAIFG))  // Use calculated branching
	{
		case TA1IV_TACCR1:                        // TACCR1 CCIFG - UART RX

			TA1CCR1 += Time_next_bit;                 // Add Offset to CCRx
			if (TA1CCTL1 & CAP) 				 // Capture mode = start bit edge
			{
				TA1CCTL1 &= ~CAP;                 // Switch capture to compare mode
				TA1CCR1 += Time_next_bit_5;       // Point CCRx to middle of D0
			}
			else
			{
				if(rxBitCnt > 1)
				{
					rxData >>= 1;
					if (TA1CCTL1 & SCCI) 			  // Get bit waiting in receive latch
					{
						rxData |= 0x80;
					}
					rxBitCnt--;

				}
				else if (rxBitCnt == 1) 						// All bits RXed?
				{
					if (TA1CCTL1 & SCCI) 			  // Get bit waiting in receive latch
					{
						recieve_Buffer[buffer_index].parity = 0x01;
					}
					else
						recieve_Buffer[buffer_index].parity = 0x00;
					//recieve_Buffer.value = rxData;           		// Store in global variable
					recieve_Buffer[buffer_index].value = rxData;
					//if(TXFlag)
					//{
					//	recieve_Buffer[buffer_index].TX = 0x01;
					//}
					//else
					//	recieve_Buffer[buffer_index].TX = 0x00;
					buffer_index++;
					count++;
					rxBitCnt = 9;               		 // Re-load bit counter
					TA1CCTL1 |= CAP;             		 // Switch compare to capture mode
					buffer_index &= 0x1F;
					return (LPM0_bits);
				}
			}
			break;
	}
	return 0;

}

