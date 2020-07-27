/*******************************************************************************
*      			 main.c TCA8418A example code for development		           *
*                                                                              *
*      			 Author:             Chris Kraft                               *
*                                                                              *
*      			 Revision Date:      Mar 2013                                  *
*                                                                              *
*      			 Revision Level:     00.02                                     *
*                                                                              *
*                                                                              *
********************************************************************************
* Copyright © 20011-2012 Texas Instruments Incorporated - http://www.ti.com/   *
********************************************************************************
*  Redistribution and use in source and binary forms, with or without          *
*  modification, are permitted provided that the following conditions are met: *
*                                                                              *
*    Redistributions of source code must retain the above copyright notice,    *
*    this list of conditions and the following disclaimer.                     *
*                                                                              *
*    Redistributions in binary form must reproduce the above copyright notice, *
*    this list of conditions and the following disclaimer in the               *
*    documentation and/or other materials provided with the distribution.      *
*                                                                              *
*    Neither the name of Texas Instruments Incorporated nor the names of its   *
*    contributors may be used to endorse or promote products derived           *
*    from this software without specific prior written permission.             *
*                                                                              *
*  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" *
*  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE   *
*  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE  *
*  ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE    *
*  LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR         *
*  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF        *
*  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS    *
*  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN     *
*  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)     *
*  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE  *
*  POSSIBILITY OF SUCH DAMAGE.                                                 *
********************************************************************************
*                                 MODULE CHANGE LOG                            *
*                                                                              *
*       Date Changed:             10/25/12                                     *
*       Developer:                David Fischer	                               *
*       Change Description:       Optimization for TCA8418E					   *
*                                                                              *
*       Date Changed:             3/13		                                   *
*       Developer:                Chris Kraft	                               *
*       Change Description:       Created struct memory map					   *
*                                 Re-coded to fit new memory map               *
*                                 Development functions created				   *
*                                                                              *
*******************************************************************************/
/*******************************************************************************
*                                 Included Headers                             *
*******************************************************************************/
#include "msp430g2553.h"
#include "USCI_I2C.h"
#include "TCA5013.h"
#include "IO_Core.h"

#define WAIT_KEY while(!(IFG2&UCA0RXIFG))

#define USERCARD 1
#define SAM1 0
#define SAM2 0
#define SAM3 0
#define OLD_RTL 0

/*******************************************************************************
*                                 Prototypes                                   *
*******************************************************************************/
void write_UART_byte(unsigned char);
void print_UART(char*);
void CLOCK_INIT(void);
void PORT_INIT(void);
void UART_I2C_INIT(void);
void byte_to_ASCII(unsigned char);
void byte_to_Hex_ASCII(unsigned char data);
void TIMER_INIT(void);
void update_TimeBase(char, char, char);
void print_Menu(void);
unsigned char UartScanfAsciiByte (unsigned char* res);
void set_voltage(void);
void set_clk_div(void);
void set_card_type(void);


//#define FLASH_PTR 0xFC00;
//unsigned char* flash_ptr;

//unsigned char UartScanfAsciiU32 (unsigned short* res);


#define CYCLE_REGS { P1OUT &= ~BIT5; __delay_cycles(320000); P1OUT |= BIT5; __delay_cycles(320000);}
#define NACK_CHECK  if(NACK){print_UART("I2C Fail\r\n"); NACK =0;}
#define TRIGGER { P1OUT |= BIT3; __delay_cycles(8000); P1OUT &= ~ BIT3; __delay_cycles(8000);}
#define immediate 0
#define IOUC BIT4

#define toggle_IOUC P1DIR ^= IOUC;

#if 1
unsigned char Tzero[] = {0x00, 0xa4, 0x04, 0x00, 0x0e, 0x31, 0x50, 0x41, 0x59, 0x2e, 0x53, 0x59, 0x53, 0x2e, 0x44, 0x44, 0x46, 0x30, 0x31, 0x00}; //comment

unsigned char get_Response[] = {0x00, 0xC0, 0x00, 0x00, 0x17};
#endif
/*******************************************************************************
*                                 External Variables                           *
*******************************************************************************/
//NACK detected if equal to 1; I2C failed
extern unsigned char NACK;

extern volatile IO_buffer recieve_Buffer[];
extern unsigned char TXFlag;
extern unsigned short Time_next_bit;
extern unsigned short Time_next_bit_5;

extern unsigned char buffer_index;
extern unsigned char count;

extern unsigned char buffer[];
#if OLD_RTL
unsigned char TCA5013_ADDRESS	=	0x3A;
#else
unsigned char TCA5013_ADDRESS   =   0x39;
#endif


unsigned char IO_index=0;
unsigned char delay_UCact = 0;
unsigned char IO_clkdiv= 1;
unsigned char IO_VCC = 4;
unsigned char card_type = 0;
unsigned char run_clocktest = 0;
/*******************************************************************************
*                                 Global Variables                             *
*******************************************************************************/
//Byte tells CPU what to handle after wake
unsigned char wake_Flag=0;
unsigned char data1 = 0;
unsigned char data = 0;
unsigned char INS_REC = 0;
unsigned char get_response = 0;
unsigned char send_Remain = 0;
unsigned char blah = 0;
unsigned char selection = 0;
unsigned char RX_buf = 0;
unsigned char input=0;
unsigned char menu = 1;
unsigned char Continue=1;
//unsigned char pass_fail[173];
unsigned char first=1;
unsigned char complete =0;
unsigned char testNumber = 0;
unsigned char shutdown =0;

static unsigned char test_sequence[4] = {0x11,0x22,0x33,0x44};

volatile TCA5013_Register_Map TCA5013_Regs;

/*******************************************************************************
*                                 Main Code		                               *
*******************************************************************************/
void main(void)
{
	WDTCTL = WDTPW + WDTHOLD;                 		// Stop Watchdog Timer

	CLOCK_INIT();									// Configure clock to 8 MHz
	PORT_INIT();									// Configure ports
	UART_I2C_INIT();								// Configure I2C
	TIMER_INIT();
	//FCTL2 = FWKEY + FSSEL_2 + FN5 + FN3 + FN1 + FN0;
	__delay_cycles(10000);					 		// Allow I2C devices time to power up
	_bis_SR_register(GIE);

	/*unsigned char address = 0x01;
	do{
		NACK = 0;
		I2C_Read_Byte(address,0x02);
		address++;
		__delay_cycles(90000);
	}	while(NACK && address < 0xFF);
	P1OUT = BIT0 + BIT5;*/
	TCA5013InitDefault((TCA5013_Register_Map*)&TCA5013_Regs);

	//IE2 &= ~UCA0RXIE;
	print_UART("Loaded Correctly\r\n");


	while(1)
	{


		if(menu)
			print_Menu();






		if(wake_Flag & TCA5013_Interrupt)
		{
			menu = 0;
			wake_Flag &= ~TCA5013_Interrupt;
			TCA5013_read_Interrupt((TCA5013_Register_Map*)&TCA5013_Regs);
			print_UART("Interrupt status register: ");
			byte_to_Hex_ASCII(*(unsigned char *) &TCA5013_Regs.Int_Mask_Registers.Interrupt_status);
			print_UART("\r\n");

#if print
			if(TCA5013_Regs.Int_Mask_Registers.Interrupt_status.GPIO)
			{

				print_UART("GPIO Interrupt triggered\r\n");

			}
			if(TCA5013_Regs.Int_Mask_Registers.Interrupt_status.OTP)
			{
				print_UART("OTP Interrupt triggered\r\n");
			}
			if(TCA5013_Regs.Int_Mask_Registers.Interrupt_status.Sam1)
			{
				print_UART("SAM1 Interrupt triggered\r\n");
			}
			if(TCA5013_Regs.Int_Mask_Registers.Interrupt_status.Sam2)
			{
				print_UART("SAM2 Interrupt triggered\r\n");
			}
			if(TCA5013_Regs.Int_Mask_Registers.Interrupt_status.Sam3)
			{
				print_UART("SAM3 Interrupt triggered\r\n");
			}
			if(TCA5013_Regs.Int_Mask_Registers.Interrupt_status.Supervisor)
			{
				print_UART("Supervisor Interrupt triggered\r\n");
			}


			if(TCA5013_Regs.Int_Mask_Registers.Interrupt_status.Sync_Complete)
			{
				if(run_clocktest)
				{
					P1OUT &= ~IOUC;
					P1DIR |= IOUC;
					TCA5013_Read_UC((TCA5013_Register_Map*)&TCA5013_Regs);
					print_UART("ATR: ");
					unsigned char x = 0;
					unsigned char* ptr = (unsigned char*)&TCA5013_Regs.User_Card_Registers.ATR_Byte1;
					for(x=0; x<4;x++)
					{
						byte_to_Hex_ASCII(*ptr++);
					}
					print_UART("\r\n");
					__delay_cycles(3000);

					TCA5013_Regs.User_Card_Registers.Synchronous_Settings.C4 = 1;
					TCA5013_Regs.User_Card_Registers.Synchronous_Settings.C8 = 1;
					TCA5013_Regs.User_Card_Registers.Synchronous_Settings.CLK_Disable = 0;
					toggle_IOUC
					I2C_Write_Byte(TCA5013_ADDRESS, 0x09, *(unsigned char*)&TCA5013_Regs.User_Card_Registers.Synchronous_Settings);
					run_clocktest = 0;

					__delay_cycles(1000);
					TCA5013_Regs.User_Card_Registers.Clock_Settings.Clk1 = 0x01;
					TCA5013_Regs.User_Card_Registers.Synchronous_Settings.C4 ^= 0x01;
					toggle_IOUC
					I2C_Write_Byte(TCA5013_ADDRESS, 0x02, *(unsigned char*)&TCA5013_Regs.User_Card_Registers.Clock_Settings);
					I2C_Write_Byte(TCA5013_ADDRESS, 0x09, *(unsigned char*)&TCA5013_Regs.User_Card_Registers.Synchronous_Settings);
					__delay_cycles(1000);
					TCA5013_Regs.User_Card_Registers.Clock_Settings.Clk1 = 0x02;
					TCA5013_Regs.User_Card_Registers.Synchronous_Settings.C8 ^= 0x01;
					toggle_IOUC
					I2C_Write_Byte(TCA5013_ADDRESS, 0x02, *(unsigned char*)&TCA5013_Regs.User_Card_Registers.Clock_Settings);
					I2C_Write_Byte(TCA5013_ADDRESS, 0x09, *(unsigned char*)&TCA5013_Regs.User_Card_Registers.Synchronous_Settings);
					__delay_cycles(1000);
					TCA5013_Regs.User_Card_Registers.Clock_Settings.Clk1 = 0x01;
					TCA5013_Regs.User_Card_Registers.Synchronous_Settings.C4 ^= 0x01;
					toggle_IOUC
					I2C_Write_Byte(TCA5013_ADDRESS, 0x02, *(unsigned char*)&TCA5013_Regs.User_Card_Registers.Clock_Settings);
					I2C_Write_Byte(TCA5013_ADDRESS, 0x09, *(unsigned char*)&TCA5013_Regs.User_Card_Registers.Synchronous_Settings);
					__delay_cycles(1000);
					TCA5013_Regs.User_Card_Registers.Clock_Settings.Clk1 = 0x02;
					TCA5013_Regs.User_Card_Registers.Synchronous_Settings.C8 ^= 0x01;
					toggle_IOUC
					I2C_Write_Byte(TCA5013_ADDRESS, 0x02, *(unsigned char*)&TCA5013_Regs.User_Card_Registers.Clock_Settings);
					I2C_Write_Byte(TCA5013_ADDRESS, 0x09, *(unsigned char*)&TCA5013_Regs.User_Card_Registers.Synchronous_Settings);
					__delay_cycles(1000);
					TCA5013_Regs.User_Card_Registers.Clock_Settings.Clk1 = 0x01;
					TCA5013_Regs.User_Card_Registers.Synchronous_Settings.C4 ^= 0x01;
					TCA5013_Regs.User_Card_Registers.Synchronous_Settings.C8 ^= 0x01;
					toggle_IOUC
					I2C_Write_Byte(TCA5013_ADDRESS, 0x02, *(unsigned char*)&TCA5013_Regs.User_Card_Registers.Clock_Settings);
					I2C_Write_Byte(TCA5013_ADDRESS, 0x09, *(unsigned char*)&TCA5013_Regs.User_Card_Registers.Synchronous_Settings);
					__delay_cycles(1000);
					TCA5013_Regs.User_Card_Registers.Clock_Settings.Clk1 = 0x02;
					TCA5013_Regs.User_Card_Registers.Synchronous_Settings.C4 ^= 0x01;
					TCA5013_Regs.User_Card_Registers.Synchronous_Settings.C8 ^= 0x01;
					toggle_IOUC
					I2C_Write_Byte(TCA5013_ADDRESS, 0x02, *(unsigned char*)&TCA5013_Regs.User_Card_Registers.Clock_Settings);
					I2C_Write_Byte(TCA5013_ADDRESS, 0x09, *(unsigned char*)&TCA5013_Regs.User_Card_Registers.Synchronous_Settings);
					__delay_cycles(1000);
					TCA5013_Regs.User_Card_Registers.Clock_Settings.Clk1 = 0x01;
					TCA5013_Regs.User_Card_Registers.Synchronous_Settings.C4 ^= 0x01;
					toggle_IOUC
					I2C_Write_Byte(TCA5013_ADDRESS, 0x02, *(unsigned char*)&TCA5013_Regs.User_Card_Registers.Clock_Settings);
					I2C_Write_Byte(TCA5013_ADDRESS, 0x09, *(unsigned char*)&TCA5013_Regs.User_Card_Registers.Synchronous_Settings);
					__delay_cycles(1000);
					TCA5013_Regs.User_Card_Registers.Clock_Settings.Clk1 = 0x02;
					TCA5013_Regs.User_Card_Registers.Synchronous_Settings.C4 = 0x00;
					TCA5013_Regs.User_Card_Registers.Synchronous_Settings.C8 = 0x00;
					toggle_IOUC
					I2C_Write_Byte(TCA5013_ADDRESS, 0x02, *(unsigned char*)&TCA5013_Regs.User_Card_Registers.Clock_Settings);
					I2C_Write_Byte(TCA5013_ADDRESS, 0x09, *(unsigned char*)&TCA5013_Regs.User_Card_Registers.Synchronous_Settings);
					__delay_cycles(1000);
					TCA5013_Regs.User_Card_Registers.Clock_Settings.Clk1 = 0x01;
					TCA5013_Regs.User_Card_Registers.Synchronous_Settings.C4 = 0x01;
					TCA5013_Regs.User_Card_Registers.Synchronous_Settings.C8 = 0x01;
					toggle_IOUC

					I2C_Write_Byte(TCA5013_ADDRESS, 0x02, *(unsigned char*)&TCA5013_Regs.User_Card_Registers.Clock_Settings);
					I2C_Write_Byte(TCA5013_ADDRESS, 0x09, *(unsigned char*)&TCA5013_Regs.User_Card_Registers.Synchronous_Settings);
					__delay_cycles(1000);
					TCA5013_Regs.User_Card_Registers.Clock_Settings.Clk1 = 0x02;
					TCA5013_Regs.User_Card_Registers.Synchronous_Settings.C8 = 0x01;
					toggle_IOUC
					I2C_Write_Byte(TCA5013_ADDRESS, 0x02, *(unsigned char*)&TCA5013_Regs.User_Card_Registers.Clock_Settings);
					I2C_Write_Byte(TCA5013_ADDRESS, 0x09, *(unsigned char*)&TCA5013_Regs.User_Card_Registers.Synchronous_Settings);
					__delay_cycles(1000);

					TCA5013_Regs.User_Card_Registers.Synchronous_Settings.C8 = 0x0;
					TCA5013_Regs.User_Card_Registers.Synchronous_Settings.CLK_Disable = 1;
					I2C_Write_Byte(TCA5013_ADDRESS, 0x09, *(unsigned char*)&TCA5013_Regs.User_Card_Registers.Synchronous_Settings);

				}
			}
#endif
#if immediate
			if(TCA5013_Regs.Int_Mask_Registers.Interrupt_status.UserCard)
			{

				*(unsigned char*)&TCA5013_Regs.User_Card_Registers.Status = I2C_Read_Byte(TCA5013_ADDRESS,0x00);

				if(TCA5013_Regs.User_Card_Registers.Status.PRES & TCA5013_Regs.User_Card_Registers.Status.PRESL)
				{
					if(delay_UCact)
					{
						delay_UCact = 0;

						I2C_Write_Byte(TCA5013_ADDRESS,0x01, *(unsigned char*)&TCA5013_Regs.User_Card_Registers.Settings);
						NACK_CHECK
						print_UART("Activating Card\r\n");
					}
				}
				if(TCA5013_Regs.User_Card_Registers.Status.EARLY)
				{
					print_UART("EARLY\r\n");
				}
				if(TCA5013_Regs.User_Card_Registers.Status.MUTE)
				{
					print_UART("MUTE\r\n");
				}
				if(TCA5013_Regs.User_Card_Registers.Status.PROT)
				{
					print_UART("OVERCURRENT\r\n");
				}
				if(TCA5013_Regs.User_Card_Registers.Status.RESERVED)
				{
					print_UART("VCCRAMP\r\n");
				}


			}
#endif
			if(!(P1OUT & BIT5))
			{
				print_UART("De-asserting\r\n");

				P1OUT |= BIT5;
			}
		}

#if 1
		/*while(wake_Flag & IO_Data_Received)
		{

			menu = 0;
			unsigned char space = 0;
			wake_Flag &= ~IO_Data_Received;
			unsigned char local_count = count;
			count = 0;
			unsigned char local_index = IO_index;
			while(local_count > 0)
			{

				byte_to_Hex_ASCII(recieve_Buffer[local_index].value);


				if(recieve_Buffer[local_index].value == 0x4C)
				{
					space = 1;

					wake_Flag &= ~0x40;

					unsigned char x=0;
					P2OUT |= BIT0;
					P2DIR |= BIT0;
					TA1CCTL1 &= ~CCIE;
					for(x=0;x<5;x++)
					{
						send_Byte_IO(Tzero[x]);
					}

					INS_REC = 1;
				    P2DIR &= ~BIT0;
				    P2OUT |= BIT0;
				    //TA1CCTL1 |= CCIE;
				    TA1CCTL1 &= ~CCIFG;
				    TA1CCTL1 |= CCIE;

				}


				if(recieve_Buffer[local_index].value == 0xA4 && INS_REC)
				{
					space = 1;
					INS_REC = 0;
					unsigned char i=0;
					send_Remain=0;
					P2OUT |= BIT0;
					P2DIR |= BIT0;
					TA1CCTL1 &= ~CCIE;
					for(i=5;i<19;i++)
					{
						send_Byte_IO(Tzero[i]);
					}

					get_response = 1;
					P2DIR &= ~BIT0;
					P2OUT |= BIT0;
					TA1CCTL1 &= ~CCIFG;
					TA1CCTL1 |= CCIE;
				}
				if(recieve_Buffer[(local_index-1) & 0x1F].value == 0x61 && get_response)
				{
					space = 1;
					get_response = 0;
					unsigned char i = 0;
					P2OUT |= BIT0;
					P2DIR |= BIT0;
					TA1CCTL1 &= ~CCIE;

					for(i=0;i<4;i++)
					{
						send_Byte_IO(get_Response[i]);
					}
					send_Byte_IO(recieve_Buffer[local_index].value);

					P2DIR &= ~BIT0;
					P2OUT |= BIT0;
					TA1CCTL1 &= ~CCIFG;
					TA1CCTL1 |= CCIE;
					complete = 1;
				}

				unsigned char parityEO = parityCheck(recieve_Buffer[local_index].value);
				if(recieve_Buffer[local_index].parity ^ parityEO)
				{
					print_UART(" FAIL\r\n");
				}
				else
				{
					print_UART(" PASS\r\n");
				}
				if(space)
				{
					print_UART("\r\n");
					space = 0;
				}

				local_index++;
				local_index &= 0x1F;
				local_count--;
			}
			IO_index = local_index;

		}

		if(complete)
		{
			complete = 0;
			__delay_cycles(10000);
			// goto infinite_loop;
		}*/
#endif


		//P2IE |= BIT5;
		IFG2 &= ~UCA0RXIFG;
		IE2 |= UCA0RXIE;
		_bis_SR_register(LPM0_bits + GIE);
		//P2IE &= ~BIT5;

		if(wake_Flag & 0x40)
		{
			menu =1;
			IE2 &= ~UCA0RXIE;
			wake_Flag &= ~0x40;
			switch(RX_buf)
			{
			case 't':
			{
				// infinite_loop:P2OUT |= BIT0;
				P2DIR |= BIT0;
				TA1CCTL1 &= ~CCIE;
				unsigned char x;
				for(x=0;x<5;x++)
				{
					send_Byte_IO(Tzero[x]);
				}

				INS_REC = 1;
				P2DIR &= ~BIT0;
				P2OUT |= BIT0;
				//TA1CCTL1 |= CCIE;
				TA1CCTL1 &= ~CCIFG;
				TA1CCTL1 |= CCIE;

			}
			break;
#if 0
			case 's':
			{
				I2C_Write(4,TCA5013_ADDRESS,0x61,test_sequence,0);
				I2C_Write_Byte(TCA5013_ADDRESS,0x68,0x58);
				unsigned char rlymte[] = {0x01,0x40,0x00,0x0A};
				I2C_Write(4,TCA5013_ADDRESS,0x83,rlymte,0);

			}
				break;

#endif
#if USERCARD
			/* case 'g':
			{
				set_voltage();
				set_clk_div();
				print_UART("0: Deactivate\r\n");
				print_UART("1: Shutdown\r\n");
				unsigned char x = 0;
				while(!(IFG2&UCA0RXIFG)); //wait until keyboard input
				selection = UCA0RXBUF;

				if(selection == '1')
					shutdown = 1;
				else
					shutdown = 0;
				//CYCLE_REGS
				TCA5013_Regs.User_Card_Registers.Clock_Settings.ClockDiv = IO_clkdiv;
				I2C_Write_Byte(TCA5013_ADDRESS,0x02,*(unsigned char*)&TCA5013_Regs.User_Card_Registers.Clock_Settings);
				for(x=0;x<14;x++)
				{
					//TCA5013InitDefault((TCA5013_Register_Map*)&TCA5013_Regs);
					//CYCLE_REGS
					if(shutdown)
					{
						CYCLE_REGS
						I2C_Write_Byte(TCA5013_ADDRESS,0x02,*(unsigned char*)&TCA5013_Regs.User_Card_Registers.Clock_Settings);
						__delay_cycles(40000);
					}
					else
					{
						TCA5013_Regs.User_Card_Registers.Settings.Start_Async = 0;
						I2C_Write_Byte(TCA5013_ADDRESS,0x01,*(unsigned char*)&TCA5013_Regs.User_Card_Registers.Status);
						__delay_cycles(40000);
					}

					unsigned char var_early=0;
					unsigned char var_mute =0;
					unsigned char i2c_retval;

					TCA5013_Regs.User_Card_Registers.Settings.Set_VCC = IO_VCC;
					//TCA5013_Regs.User_Card_Registers.Settings.Card_detect = 1;
					TCA5013_Regs.User_Card_Registers.Settings.Start_Async = 1;
					if(x > 7)
					{
						if(x == 8 || x ==9)
						{
							TCA5013_Regs.User_Card_Registers.Early.MSB.all = 0;
							TCA5013_Regs.User_Card_Registers.Early.LSB.all = 0;

						}
						if(x == 10 || x == 11)
						{
							TCA5013_Regs.User_Card_Registers.Early.MSB.all = 0xFF;
							TCA5013_Regs.User_Card_Registers.Early.LSB.all = 3;
						}
						if(x == 12 || x == 13)
						{
							TCA5013_Regs.User_Card_Registers.Mute.MSB.all = 0x0;
							TCA5013_Regs.User_Card_Registers.Mute.LSB.all = 0x0;
							TCA5013_Regs.User_Card_Registers.Early.MSB.all = 0;
							TCA5013_Regs.User_Card_Registers.Early.LSB.all = 0;
						}
						I2C_Write(4,TCA5013_ADDRESS,0x83,(unsigned char*)&TCA5013_Regs.User_Card_Registers.Early.MSB,0);
					}
					I2C_Write_Byte(TCA5013_ADDRESS,0x01, *(unsigned char*)&TCA5013_Regs.User_Card_Registers.Settings);

					__delay_cycles(2000000);



					TRIGGER
					__delay_cycles(2000000);
					print_UART("Test #");
					byte_to_ASCII(x);
					print_UART("\r\n");
					TCA5013_Read_INTSTAT_noAI((TCA5013_Register_Map*)&TCA5013_Regs);
					i2c_retval = TCA5013_Read_UC_noAI((TCA5013_Register_Map*)&TCA5013_Regs);
					if(i2c_retval != I2C_OPERATION_FAIL)
					{

					//	byte_to_Hex_ASCII(*(unsigned char*)&TCA5013_Regs.User_Card_Registers.Status);

						if(TCA5013_Regs.User_Card_Registers.Status.EARLY)
						{
							var_early = 1;
							print_UART("Early ");
						}
						if(TCA5013_Regs.User_Card_Registers.Status.MUTE)
						{
							var_mute = 1;
							print_UART("Mute");
						}
						if(!(var_early | var_mute))
						{
							print_UART("None");
						}

					}

					if(var_early | var_mute)
					{
						i2c_retval = TCA5013_Read_UC_noAI((TCA5013_Register_Map*)&TCA5013_Regs);
						if(i2c_retval != I2C_OPERATION_FAIL)
						{
							if(var_early & TCA5013_Regs.User_Card_Registers.Status.EARLY | var_mute & TCA5013_Regs.User_Card_Registers.Status.MUTE)
							{
								print_UART(" I2C Clear Fail");
							}


						}
					}
					print_UART("\r\n");
					while(!(IFG2&UCA0RXIFG)); //wait until keyboard input
					selection = UCA0RXBUF;
				}



			}
			break;*/



			/* case 'w':
			{
				set_voltage();
				set_clk_div();
				print_UART("0: deactivate\r\n");
				print_UART("1: Shutdown\r\n");
				while(!(IFG2&UCA0RXIFG)); //wait until keyboard input
				selection = UCA0RXBUF;
				if(selection == '1')
					shutdown = 1;
				else
					shutdown = 0;
				unsigned char x = 0;

				//CYCLE_REGS

				TCA5013_Regs.User_Card_Registers.Clock_Settings.ClockDiv = IO_clkdiv;
				I2C_Write_Byte(TCA5013_ADDRESS,0x02,*(unsigned char*)&TCA5013_Regs.User_Card_Registers.Clock_Settings);
				for(x=0;x<4;x++)
				{
					TCA5013InitDefault((TCA5013_Register_Map*)&TCA5013_Regs);
					if(shutdown)
					{
						CYCLE_REGS
						I2C_Write_Byte(TCA5013_ADDRESS,0x02,*(unsigned char*)&TCA5013_Regs.User_Card_Registers.Clock_Settings);
					}
					else
					{
						TCA5013_Regs.User_Card_Registers.Settings.Start_Async = 0;
						I2C_Write_Byte(TCA5013_ADDRESS,0x01,*(unsigned char*)&TCA5013_Regs.User_Card_Registers.Status);
						__delay_cycles(40000);
					}
					//CYCLE_REGS

					TCA5013_Regs.User_Card_Registers.Settings.Set_VCC = IO_VCC;
					unsigned char var_early=0;
					unsigned char var_mute =0;
					unsigned char i2c_retval;

					//TCA5013_Regs.User_Card_Registers.Settings.Card_detect = 1;
					TCA5013_Regs.User_Card_Registers.Settings.Start_Async = 1;
					I2C_Write_Byte(TCA5013_ADDRESS,0x01, *(unsigned char*)&TCA5013_Regs.User_Card_Registers.Settings);

					__delay_cycles(500000);



					TRIGGER
					__delay_cycles(1000000);
					print_UART("Cold Reset Test #");
					byte_to_ASCII(x);
					print_UART("\r\n");
					TCA5013_Read_INTSTAT((TCA5013_Register_Map*)&TCA5013_Regs);
					i2c_retval = TCA5013_Read_UC((TCA5013_Register_Map*)&TCA5013_Regs);
					if(i2c_retval != I2C_OPERATION_FAIL)
					{

					//	byte_to_Hex_ASCII(*(unsigned char*)&TCA5013_Regs.User_Card_Registers.Status);

						if(TCA5013_Regs.User_Card_Registers.Status.EARLY)
						{
							var_early = 1;
							print_UART("Early ");
						}
						if(TCA5013_Regs.User_Card_Registers.Status.MUTE)
						{
							var_mute = 1;
							print_UART("Mute");
						}
						if(!(var_early | var_mute))
						{
							print_UART("None");
						}

					}

					if(var_early | var_mute)
					{
						i2c_retval = TCA5013_Read_UC_noAI((TCA5013_Register_Map*)&TCA5013_Regs);
						if(i2c_retval != I2C_OPERATION_FAIL)
						{
							if(var_early & TCA5013_Regs.User_Card_Registers.Status.EARLY | var_mute & TCA5013_Regs.User_Card_Registers.Status.MUTE)
							{
								print_UART(" I2C Clear Fail");
							}


						}
					}
					print_UART("\r\n");

					TCA5013_Regs.User_Card_Registers.Settings.Warm = 1;
					var_early=0;
					var_mute =0;
//								i2c_retval =0;

					I2C_Write_Byte(TCA5013_ADDRESS,0x01, *(unsigned char*)&TCA5013_Regs.User_Card_Registers.Settings);
					__delay_cycles(500000);



					TRIGGER
					__delay_cycles(1000000);
					print_UART("Warm Test #");
					byte_to_ASCII(x);
					print_UART("\r\n");
					TCA5013_Read_INTSTAT_noAI((TCA5013_Register_Map*)&TCA5013_Regs);
					i2c_retval = TCA5013_Read_UC_noAI((TCA5013_Register_Map*)&TCA5013_Regs);
					if(i2c_retval != I2C_OPERATION_FAIL)
					{
						if(TCA5013_Regs.User_Card_Registers.Status.EARLY)
						{
							var_early = 1;
							print_UART("Early ");
						}
						if(TCA5013_Regs.User_Card_Registers.Status.MUTE)
						{
							var_mute = 1;
							print_UART("Mute");
						}
						if(!(var_early | var_mute))
						{
							print_UART("None");
						}

					}
					else
					{
						print_UART("I2C FAIL");
					}
					if(var_early | var_mute)
					{
						i2c_retval = TCA5013_Read_UC_noAI((TCA5013_Register_Map*)&TCA5013_Regs);
						if(i2c_retval != I2C_OPERATION_FAIL)
						{
							if(var_early & TCA5013_Regs.User_Card_Registers.Status.EARLY | var_mute & TCA5013_Regs.User_Card_Registers.Status.MUTE)
							{
								print_UART(" I2C Clear Fail");
							}


						}
					}
					print_UART("\r\n");

				}


				for(;x<14;x++)
				{
					TCA5013InitDefault((TCA5013_Register_Map*)&TCA5013_Regs);
					TCA5013_Regs.User_Card_Registers.Settings.Warm = 1;
					//I2C_Write_Byte(TCA5013_ADDRESS,0x01, *(unsigned char*)&TCA5013_Regs.User_Card_Registers.Settings);
					unsigned char var_early=0;
					unsigned char var_mute =0;
					unsigned char i2c_retval;
					TCA5013_Regs.User_Card_Registers.Settings.Set_VCC = IO_VCC;
					//TCA5013_Regs.User_Card_Registers.Settings.Card_detect = 1;
					TCA5013_Regs.User_Card_Registers.Settings.Start_Async = 1;
					if(x > 7)
					{
						if(x == 8 || x ==9)
						{
							TCA5013_Regs.User_Card_Registers.Early.MSB.all = 0;
							TCA5013_Regs.User_Card_Registers.Early.LSB.all = 0;

						}
						if(x == 10 || x == 11)
						{
							TCA5013_Regs.User_Card_Registers.Early.MSB.all = 0xFF;
							TCA5013_Regs.User_Card_Registers.Early.LSB.all = 3;
						}
						if(x == 12 || x == 13)
						{
							TCA5013_Regs.User_Card_Registers.Mute.MSB.all = 0x0;
							TCA5013_Regs.User_Card_Registers.Mute.LSB.all = 0x0;
							TCA5013_Regs.User_Card_Registers.Early.MSB.all = 0;
							TCA5013_Regs.User_Card_Registers.Early.LSB.all = 0;
						}
						I2C_Write(4,TCA5013_ADDRESS,0x83,(unsigned char*)&TCA5013_Regs.User_Card_Registers.Early.MSB,0);
					}
					I2C_Write_Byte(TCA5013_ADDRESS,0x01, *(unsigned char*)&TCA5013_Regs.User_Card_Registers.Settings);

					__delay_cycles(500000);


					//if(x == 5)
					//	print_UART("FIVER");
					TRIGGER
					__delay_cycles(1000000);

					print_UART("Test #");
					byte_to_ASCII(x);
					print_UART("\r\n");
					TCA5013_Read_INTSTAT((TCA5013_Register_Map*)&TCA5013_Regs);
					i2c_retval = TCA5013_Read_UC((TCA5013_Register_Map*)&TCA5013_Regs);
					if(i2c_retval != I2C_OPERATION_FAIL)
					{
						if(TCA5013_Regs.User_Card_Registers.Status.EARLY)
						{
							var_early = 1;
							print_UART("Early ");
						}
						if(TCA5013_Regs.User_Card_Registers.Status.MUTE)
						{
							var_mute = 1;
							print_UART("Mute");
						}
						if(!(var_early | var_mute))
						{
							print_UART("None");
						}

					}

					if(var_early | var_mute)
					{
						i2c_retval = TCA5013_Read_UC_noAI((TCA5013_Register_Map*)&TCA5013_Regs);
						if(i2c_retval != I2C_OPERATION_FAIL)
						{
							if(var_early & TCA5013_Regs.User_Card_Registers.Status.EARLY | var_mute & TCA5013_Regs.User_Card_Registers.Status.MUTE)
							{
								print_UART(" I2C Clear Fail");
							}


						}
					}
					print_UART("\r\n");
				}



			}
			break;*/


			/*case 'm':
			{
				TCA5013InitDefault((TCA5013_Register_Map*)&TCA5013_Regs);
				//CYCLE_REGS;
				unsigned char var_early=0;
				unsigned char var_mute =0;
				unsigned char i2c_retval;
				unsigned char x = 0;
				set_voltage();
				set_clk_div();
				TCA5013_Regs.User_Card_Registers.Clock_Settings.ClockDiv = IO_clkdiv;
				I2C_Write_Byte(TCA5013_ADDRESS,0x02,*(unsigned char*)&TCA5013_Regs.User_Card_Registers.Clock_Settings);
				for(;x<2;x++)
				{
					var_early=0;
					var_mute=0;

					TCA5013_Regs.User_Card_Registers.Settings.Start_Async = 0;
					I2C_Write_Byte(TCA5013_ADDRESS,0x01,*(unsigned char*)&TCA5013_Regs.User_Card_Registers.Settings);
					__delay_cycles(40000);

					TCA5013_Regs.User_Card_Registers.Settings.Set_VCC = IO_VCC;
					TCA5013_Regs.User_Card_Registers.Settings.Start_Async = 1;

					TCA5013_Regs.User_Card_Registers.Mute.MSB.all = 0xFF;
					TCA5013_Regs.User_Card_Registers.Mute.LSB.all = 0xFF;

					I2C_Write(2,TCA5013_ADDRESS,0x05 | 0x80,(unsigned char*)&TCA5013_Regs.User_Card_Registers.Mute.MSB,0);
					I2C_Write_Byte(TCA5013_ADDRESS,0x01, *(unsigned char*)&TCA5013_Regs.User_Card_Registers.Settings);

					__delay_cycles(500000);

					TRIGGER
					__delay_cycles(1000000);
					print_UART("Cold Test #");
					byte_to_ASCII(x);
					print_UART("\r\n");
					TCA5013_Read_INTSTAT((TCA5013_Register_Map*)&TCA5013_Regs);
					i2c_retval = TCA5013_Read_UC((TCA5013_Register_Map*)&TCA5013_Regs);
					if(i2c_retval != I2C_OPERATION_FAIL)
					{
						if(TCA5013_Regs.User_Card_Registers.Status.EARLY)
						{
							var_early = 1;
							print_UART("Early ");
						}
						if(TCA5013_Regs.User_Card_Registers.Status.MUTE)
						{
							var_mute = 1;
							print_UART("Mute");
						}
						if(!(var_early | var_mute))
						{
							print_UART("None");
						}

					}

					if(var_early | var_mute)
					{
						i2c_retval = TCA5013_Read_UC((TCA5013_Register_Map*)&TCA5013_Regs);
						if(i2c_retval != I2C_OPERATION_FAIL)
						{
							if(var_early & TCA5013_Regs.User_Card_Registers.Status.EARLY | var_mute & TCA5013_Regs.User_Card_Registers.Status.MUTE)
							{
								print_UART(" I2C Clear Fail");
							}


						}
					}
					print_UART("\r\n");
				}

				for(x=0;x<2;x++)
				{
					var_early=0;
					var_mute=0;
					TCA5013_Regs.User_Card_Registers.Settings.Warm = 1;

					TCA5013_Regs.User_Card_Registers.Mute.MSB.all = 0xFF;
					TCA5013_Regs.User_Card_Registers.Mute.LSB.all = 0xFF;

					I2C_Write(2,TCA5013_ADDRESS,0x05 | 0x80,(unsigned char*)&TCA5013_Regs.User_Card_Registers.Mute.MSB,0);
					I2C_Write_Byte(TCA5013_ADDRESS,0x01, *(unsigned char*)&TCA5013_Regs.User_Card_Registers.Settings);

					__delay_cycles(500000);

					TRIGGER
					__delay_cycles(1000000);
					print_UART("Warm Test #");
					byte_to_ASCII(x);
					print_UART("\r\n");
					TCA5013_Read_INTSTAT((TCA5013_Register_Map*)&TCA5013_Regs);
					i2c_retval = TCA5013_Read_UC((TCA5013_Register_Map*)&TCA5013_Regs);
					if(i2c_retval != I2C_OPERATION_FAIL)
					{
						if(TCA5013_Regs.User_Card_Registers.Status.EARLY)
						{
							var_early = 1;
							print_UART("Early ");
						}
						if(TCA5013_Regs.User_Card_Registers.Status.MUTE)
						{
							var_mute = 1;
							print_UART("Mute");
						}
						if(!(var_early | var_mute))
						{
							print_UART("None");
						}

					}

					if(var_early | var_mute)
					{
						i2c_retval = TCA5013_Read_UC((TCA5013_Register_Map*)&TCA5013_Regs);
						if(i2c_retval != I2C_OPERATION_FAIL)
						{
							if(var_early & TCA5013_Regs.User_Card_Registers.Status.EARLY | var_mute & TCA5013_Regs.User_Card_Registers.Status.MUTE)
							{
								print_UART(" I2C Clear Fail");
							}


						}
					}
					print_UART("\r\n");
				}

				CYCLE_REGS;
			}
			break;*/



#endif

#if SAM1
			/*case 'g':
			{
				print_UART("\r\n");
				unsigned char x = 0;

				for(x=0;x<14;x++)
				{
					TCA5013InitDefault((TCA5013_Register_Map*)&TCA5013_Regs);
					CYCLE_REGS
					unsigned char var_early=0;
					unsigned char var_mute =0;
					unsigned char i2c_retval;
					TCA5013_Regs.Sam1_Registers.Settings.Set_VCC = V_5;

					TCA5013_Regs.Sam1_Registers.Settings.Start_async = 1;
					if(x > 7)
					{
						if(x == 8 || x ==9)
						{
							TCA5013_Regs.Sam1_Registers.Early.MSB.all = 0;
							TCA5013_Regs.Sam1_Registers.Early.LSB.all = 0;

						}
						if(x == 10 || x == 11)
						{
							TCA5013_Regs.Sam1_Registers.Early.MSB.all = 0xFF;
							TCA5013_Regs.Sam1_Registers.Early.LSB.all = 3;
						}
						if(x == 12 || x == 13)
						{
							TCA5013_Regs.Sam1_Registers.Mute.MSB.all = 0x0;
							TCA5013_Regs.Sam1_Registers.Mute.LSB.all = 0x0;
							TCA5013_Regs.Sam1_Registers.Early.MSB.all = 0;
							TCA5013_Regs.Sam1_Registers.Early.LSB.all = 0;
						}
						I2C_Write(4,TCA5013_ADDRESS,0x13 | 0x80,(unsigned char*)&TCA5013_Regs.Sam1_Registers.Early.MSB,0);
					}
					I2C_Write_Byte(TCA5013_ADDRESS,0x11, *(unsigned char*)&TCA5013_Regs.Sam1_Registers.Settings);

					__delay_cycles(2000000);



					TRIGGER
					__delay_cycles(2000000);
					print_UART("Test #");
					byte_to_ASCII(x);
					print_UART("\r\n");
					TCA5013_Read_INTSTAT_noAI((TCA5013_Register_Map*)&TCA5013_Regs);
					i2c_retval = TCA5013_Read_SAM1_noAI((TCA5013_Register_Map*)&TCA5013_Regs);
					if(i2c_retval != I2C_OPERATION_FAIL)
					{

					//	byte_to_Hex_ASCII(*(unsigned char*)&TCA5013_Regs.User_Card_Registers.Status);

						if(TCA5013_Regs.Sam1_Registers.Status.EARLY)
						{
							var_early = 1;
							print_UART("Early ");
						}
						if(TCA5013_Regs.Sam1_Registers.Status.MUTE)
						{
							var_mute = 1;
							print_UART("Mute");
						}
						if(!(var_early | var_mute))
						{
							print_UART("None");
						}

					}

					if(var_early | var_mute)
					{
						i2c_retval = TCA5013_Read_SAM1_noAI((TCA5013_Register_Map*)&TCA5013_Regs);
						if(i2c_retval != I2C_OPERATION_FAIL)
						{
							if(var_early & TCA5013_Regs.Sam1_Registers.Status.EARLY | var_mute & TCA5013_Regs.Sam1_Registers.Status.MUTE)
							{
								print_UART(" I2C Clear Fail");
							}


						}
					}
					print_UART("\r\n");
				}



			}
			break;*/



			/*case 'w':
			{
				print_UART("\r\n");
				unsigned char x = 0;
				CYCLE_REGS
				TCA5013_Regs.Sam1_Registers.Settings.Start_async = 1;
				//TCA5013_Regs.User_Card_Registers.Settings.Warm = 1;

				//TCA5013_Regs.User_Card_Registers.Clock_Settings.ClockDiv = IO_clkdiv;

				I2C_Write_Byte(TCA5013_ADDRESS,0x12, *(unsigned char*)&TCA5013_Regs.Sam1_Registers.Clock_Settings);
				NACK_CHECK
				I2C_Write_Byte(TCA5013_ADDRESS,0x11, *(unsigned char*)&TCA5013_Regs.Sam1_Registers.Settings);
				NACK_CHECK

				__delay_cycles(2000000);
				TRIGGER
				__delay_cycles(2000000);
				TCA5013_Read_INTSTAT_noAI((TCA5013_Register_Map*)&TCA5013_Regs);
				TCA5013_Read_SAM1_noAI((TCA5013_Register_Map*)&TCA5013_Regs);

				for(x=0;x<14;x++)
				{
					TCA5013InitDefault((TCA5013_Register_Map*)&TCA5013_Regs);
					TCA5013_Regs.Sam1_Registers.Settings.Warm = 1;
					//I2C_Write_Byte(TCA5013_ADDRESS,0x01, *(unsigned char*)&TCA5013_Regs.User_Card_Registers.Settings);
					unsigned char var_early=0;
					unsigned char var_mute =0;
					unsigned char i2c_retval;
					TCA5013_Regs.Sam1_Registers.Settings.Set_VCC = V_5;
					//TCA5013_Regs.Sam1_Registers.Settings.Card_detect = 1;
					TCA5013_Regs.Sam1_Registers.Settings.Start_async = 1;
					if(x > 7)
					{
						if(x == 8 || x ==9)
						{
							TCA5013_Regs.Sam1_Registers.Early.MSB.all = 0;
							TCA5013_Regs.Sam1_Registers.Early.LSB.all = 0;

						}
						if(x == 10 || x == 11)
						{
							TCA5013_Regs.Sam1_Registers.Early.MSB.all = 0xFF;
							TCA5013_Regs.Sam1_Registers.Early.LSB.all = 3;
						}
						if(x == 12 || x == 13)
						{
							TCA5013_Regs.Sam1_Registers.Mute.MSB.all = 0x0;
							TCA5013_Regs.Sam1_Registers.Mute.LSB.all = 0x0;
							TCA5013_Regs.Sam1_Registers.Early.MSB.all = 0;
							TCA5013_Regs.Sam1_Registers.Early.LSB.all = 0;
						}
						I2C_Write(4,TCA5013_ADDRESS,0x13 | 0x80,(unsigned char*)&TCA5013_Regs.Sam1_Registers.Early.MSB,0);
					}
					I2C_Write_Byte(TCA5013_ADDRESS,0x11, *(unsigned char*)&TCA5013_Regs.Sam1_Registers.Settings);

					__delay_cycles(2000000);



					TRIGGER
					__delay_cycles(2000000);
					print_UART("Test #");
					byte_to_ASCII(x);
					print_UART("\r\n");
					TCA5013_Read_INTSTAT_noAI((TCA5013_Register_Map*)&TCA5013_Regs);
					i2c_retval = TCA5013_Read_SAM1_noAI((TCA5013_Register_Map*)&TCA5013_Regs);
					if(i2c_retval != I2C_OPERATION_FAIL)
					{

					//	byte_to_Hex_ASCII(*(unsigned char*)&TCA5013_Regs.User_Card_Registers.Status);

						if(TCA5013_Regs.Sam1_Registers.Status.EARLY)
						{
							var_early = 1;
							print_UART("Early ");
						}
						if(TCA5013_Regs.Sam1_Registers.Status.MUTE)
						{
							var_mute = 1;
							print_UART("Mute");
						}
						if(!(var_early | var_mute))
						{
							print_UART("None");
						}

					}

					if(var_early | var_mute)
					{
						i2c_retval = TCA5013_Read_SAM1_noAI((TCA5013_Register_Map*)&TCA5013_Regs);
						if(i2c_retval != I2C_OPERATION_FAIL)
						{
							if(var_early & TCA5013_Regs.Sam1_Registers.Status.EARLY | var_mute & TCA5013_Regs.Sam1_Registers.Status.MUTE)
							{
								print_UART(" I2C Clear Fail");
							}


						}
					}
					print_UART("\r\n");
				}



			}
			break;*/





#endif

			case 'c':
				print_UART("p: CLK Stop\r\n");
				print_UART("s: CLK Start\r\n");
				WAIT_KEY;
				selection = UCA0RXBUF;
				switch(selection)
				{
					case 'p':

						{
							print_UART("0: Set clk to static 0\r\n");
							print_UART("1: Set clk to static 1\r\n");
							WAIT_KEY;
							selection = UCA0RXBUF;
							unsigned char clk_stop;
							if(selection & 0x01)
								clk_stop = 0x01;
							else
								clk_stop = 0x02;

							print_UART("a: Async User card\r\n");
							print_UART("s: synch User card\r\n");
							print_UART("1: sam1\r\n");
							print_UART("2: sam2\r\n");
							print_UART("3: sam3\r\n");
							WAIT_KEY;
							selection = UCA0RXBUF;
							switch(selection)
							{
								case 'a':TCA5013_Regs.User_Card_Registers.Clock_Settings.Clk1 = clk_stop;
								I2C_Write_Byte(TCA5013_ADDRESS, 0x02, *(unsigned char*)&TCA5013_Regs.User_Card_Registers.Clock_Settings);
									break;
								case 's': print_UART("NOT IMPLIMENTED\r\n");
									break;
								case '1':TCA5013_Regs.Sam1_Registers.Clock_Settings.Clk1 = clk_stop;
								I2C_Write_Byte(TCA5013_ADDRESS, 0x12, *(unsigned char*)&TCA5013_Regs.Sam1_Registers.Clock_Settings);
									break;
								case '2':TCA5013_Regs.Sam2_Registers.Clock_Settings.Clk1 = clk_stop;
								I2C_Write_Byte(TCA5013_ADDRESS, 0x22, *(unsigned char*)&TCA5013_Regs.Sam2_Registers.Clock_Settings);
									break;
								case '3':TCA5013_Regs.Sam3_Registers.Clock_Settings.Clk1 = clk_stop;
								I2C_Write_Byte(TCA5013_ADDRESS, 0x32, *(unsigned char*)&TCA5013_Regs.Sam3_Registers.Clock_Settings);
									break;
							}

						}
						break;
					case 's':
						set_clk_div();
						print_UART("a: Async User card\r\n");
						print_UART("s: synch User card\r\n");
						print_UART("1: sam1\r\n");
						print_UART("2: sam2\r\n");
						print_UART("3: sam3\r\n");
						WAIT_KEY;
						selection = UCA0RXBUF;
						switch(selection)
						{
							case 'a':TCA5013_Regs.User_Card_Registers.Clock_Settings.Clk1 = 0;
							TCA5013_Regs.User_Card_Registers.Clock_Settings.ClockDiv = IO_clkdiv;
							I2C_Write_Byte(TCA5013_ADDRESS, 0x02, *(unsigned char*)&TCA5013_Regs.User_Card_Registers.Clock_Settings);
								break;
							case 's': print_UART("NOT IMPLIMENTED\r\n");
								break;
							case '1':TCA5013_Regs.Sam1_Registers.Clock_Settings.Clk1 = 0;
							TCA5013_Regs.Sam1_Registers.Clock_Settings.Clock_Divide = IO_clkdiv;
							I2C_Write_Byte(TCA5013_ADDRESS, 0x12, *(unsigned char*)&TCA5013_Regs.Sam1_Registers.Clock_Settings);
								break;
							case '2':TCA5013_Regs.Sam2_Registers.Clock_Settings.Clk1 = 0;
							TCA5013_Regs.Sam2_Registers.Clock_Settings.Clock_Divide = IO_clkdiv;
							I2C_Write_Byte(TCA5013_ADDRESS, 0x22, *(unsigned char*)&TCA5013_Regs.Sam2_Registers.Clock_Settings);
								break;
							case '3':TCA5013_Regs.Sam3_Registers.Clock_Settings.Clk1 = 0;
							TCA5013_Regs.Sam3_Registers.Clock_Settings.Clock_Divide = IO_clkdiv;
							I2C_Write_Byte(TCA5013_ADDRESS, 0x32, *(unsigned char*)&TCA5013_Regs.Sam3_Registers.Clock_Settings);
								break;
						}
					break;
				}

break;

				case 'p':
					print_UART("l: Pres default low\r\n");
					print_UART("h: Pres default high\r\n");
					WAIT_KEY;
					selection = UCA0RXBUF;
					switch (selection)
					{
						case 'l': TCA5013_Regs.User_Card_Registers.Settings.Card_detect = 1;
						I2C_Write_Byte(TCA5013_ADDRESS, 0x01, *(unsigned char*)&TCA5013_Regs.User_Card_Registers.Settings);
						NACK_CHECK
						break;

						case 'h': TCA5013_Regs.User_Card_Registers.Settings.Card_detect = 0;
						I2C_Write_Byte(TCA5013_ADDRESS, 0x01, *(unsigned char*)&TCA5013_Regs.User_Card_Registers.Settings);
						NACK_CHECK
						break;

					}
					break;

					case 'h':
						if(P1OUT & BIT5)
						{
							TCA5013InitDefault((TCA5013_Register_Map*)&TCA5013_Regs);
							print_UART("Asserting\r\n");
							P1OUT &= ~BIT5;
						}
						else
						{
							print_UART("De-asserting\r\n");
							P1OUT |= BIT5;
						}

						break;

				case 'x':
					TCA5013InitDefault((TCA5013_Register_Map*)&TCA5013_Regs);
					print_UART("Asserting/de-asserting shutdown\r\n");
					CYCLE_REGS
					break;


				case 'd':
					print_UART("a: Deactivate asynchronous card\r\n");
					print_UART("s: Deactivate synchronous card\r\n");
					print_UART("1: Deactivate SAM1\r\n");
					print_UART("2: Deactivate SAM2\r\n");
					print_UART("3: Deactivate SAM3\r\n");
					WAIT_KEY;
					selection = UCA0RXBUF;
					switch(selection)
					{
						case '1': TCA5013_Regs.Sam1_Registers.Settings.Start_async = 0;
						I2C_Write_Byte(TCA5013_ADDRESS, 0x11, *(unsigned char*)& TCA5013_Regs.Sam1_Registers.Settings);
						NACK_CHECK
						break;


						case '2': TCA5013_Regs.Sam2_Registers.Settings.Start_async = 0;
						I2C_Write_Byte(TCA5013_ADDRESS, 0x21, *(unsigned char*)& TCA5013_Regs.Sam2_Registers.Settings);
						NACK_CHECK
						break;

						case '3': 	TCA5013_Regs.Sam3_Registers.Settings.Start_async = 0;
									I2C_Write_Byte(TCA5013_ADDRESS, 0x31, *(unsigned char*)& TCA5013_Regs.Sam3_Registers.Settings);
									NACK_CHECK
						break;

						case 'a': TCA5013_Regs.User_Card_Registers.Settings.Start_Async = 0;
						I2C_Write_Byte(TCA5013_ADDRESS, 0x01, *(unsigned char*)& TCA5013_Regs.User_Card_Registers.Settings);
						NACK_CHECK
						break;

						case 's': TCA5013_Regs.User_Card_Registers.Synchronous_Settings.Start_Sync = 0;
						I2C_Write_Byte(TCA5013_ADDRESS, 0x09, *(unsigned char*)& TCA5013_Regs.User_Card_Registers.Synchronous_Settings);
						NACK_CHECK
						break;
					}
					break;

				case 'i':
				{
					selection = 0;
					print_UART("I2C Menu\r\n");

					print_UART("r: Read multiple registers\r\n");

					//print_UART("t: Open test registers\r\n");
					//print_UART("c: close test registers\r\n");
					print_UART("b: Read/Write any register\r\n");
					WAIT_KEY;
					selection = UCA0RXBUF;


					switch(selection)
					{

						/*case 't':
						{
							I2C_Write(4,TCA5013_ADDRESS,0x61,test_sequence,0);
							print_UART("Test mode enabled\r\n");
						}
						break; */

						/* case 'c':
						{
							I2C_Write_Byte(TCA5013_ADDRESS,0x61,0x55);
							print_UART("Test mode disabled\r\n");

						} */
break;
						case 'b':
						{
							print_UART("r: Read\r\n");
							print_UART("a: Read Range\r\n");
							print_UART("w: Write\r\n");
							WAIT_KEY;
							selection = UCA0RXBUF;

							switch(selection)
							{
							/*case 'f':
							{
								unsigned char fs[8] = {0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF};
								I2C_Write(8,TCA5013_ADDRESS, 0x51 | 0x80, fs,0);
							}
							break; */
							case 'a':
							{
								unsigned char echo[40];
								unsigned char startadd = 0;
								unsigned char endadd = 0;
								print_UART("Enter start register address: ");
								UartScanfAsciiByte(&startadd);
								print_UART("\r\nEnter end register address: ");
								UartScanfAsciiByte(&endadd);
								if((endadd - startadd) < 40 && (endadd - startadd) >=0)
								{
									print_UART("\r\n");
									I2C_Read((endadd- startadd),TCA5013_ADDRESS, startadd | 0x80 , echo);
								}
								else
									print_UART("\r\nInvalid");
								unsigned char x = 0;
								for(x=0;x<(endadd-startadd);x++)
								{
									byte_to_Hex_ASCII(echo[x]);
								}
								print_UART("\r\n");

							}

							break;
								case 'w':
								{
									print_UART("Enter register address: ");
									unsigned char TCAreg = 0;
									if(!UartScanfAsciiByte(&TCAreg))
									{
										print_UART("\r\nEnter databyte: ");
										unsigned char data_byte = 0;
										if(!UartScanfAsciiByte(&data_byte))
										{
											I2C_Write_Byte(TCA5013_ADDRESS,TCAreg,data_byte);
											if(NACK)
											{
												print_UART("\r\nI2C FAIL\r\n");
												NACK = 0;
											}
											else
												print_UART("\r\n");
										}
										else
											print_UART("\r\nInvalid\r\n");

									}
									else
										print_UART("\r\nInvalid\r\n");
									break;
								}

								case 'r':
								{
									print_UART("Enter register address: ");
									unsigned char ReadReg = 0;
									unsigned char ReadretVal = 0;
									if(!UartScanfAsciiByte(&ReadReg))
									{
										ReadretVal = I2C_Read_Byte(TCA5013_ADDRESS, ReadReg);
										NACK_CHECK
										print_UART("Data at ");
										byte_to_Hex_ASCII(ReadReg);
										print_UART(" = ");
										byte_to_Hex_ASCII(ReadretVal);
										print_UART("\r\n");
									}
									else
										print_UART("Invalid\r\n");
								}
								break;

							}
						}


						break;


#if 0
					case 'i':
						{
							I2C_Read_Byte(TCA5013_ADDRESS, 0x41 | 0x80);
							I2C_Read_Byte(TCA5013_ADDRESS, 0x00 | 0x80);
							NACK_CHECK
							break;
						}
						case '2':
							I2C_Read_Byte(TCA5013_ADDRESS, 0x41);
							I2C_Read_Byte(TCA5013_ADDRESS, 0x00);
							NACK_CHECK
							break;
#endif

						case 'a':
						{
							print_UART(" 0: A0 = GND\r\n 1: A0 = VCC\r\n 2: A0 = SDA\r\n 3: A0 = SCL\r\n 4: Invalid address\r\n");
							while(!(IFG2&UCA0RXIFG)); //wait until keyboard input
							input = UCA0RXBUF;
							switch(input)
							{
								case '0':
									TCA5013_ADDRESS = 0x39;
									break;
								case '1':
									TCA5013_ADDRESS = 0x3E;
									break;
								/*case '2':
									TCA5013_ADDRESS = 0x3C;
									break;
								case '3':
									TCA5013_ADDRESS = 0x3D;
									break;*/
								case '4':
								{
									print_UART("Enter Invalid address\r\n");
									unsigned char reg;
									if(!UartScanfAsciiByte(&reg))
									{
										TCA5013_ADDRESS = reg;
									}
									print_UART("\r\n");
									break;
								}
							}
							break;
						}
#if 0
						case 'w':
						{
							unsigned char ai = 0;
							print_UART(" a: write using autoincriment\r\n n: write without AI\r\n");
							while(!(IFG2&UCA0RXIFG)); //wait until keyboard input
							ai = UCA0RXBUF;
							print_UART("  f: write all to 0xFF\r\n  0: write all to 0x00\r\n  m: Write to reg Multi\r\n  s: Write slave address\r\n");
							while(!(IFG2&UCA0RXIFG)); //wait until keyboard input
							input = UCA0RXBUF;
							switch(input)
							{
								case 'f':
									TCA5013_To_value((TCA5013_Register_Map*)&TCA5013_Regs, 0xFF);
									if(ai =='a')
									{
										TCA5013_Write_All((TCA5013_Register_Map*)&TCA5013_Regs);
									}
									else if(ai == 'n')
									{
										unsigned char x = 0;
										for(x=0;x<71;x++)
										{
											I2C_Write_Byte(TCA5013_ADDRESS,x,0xFF);
										}
									}
									break;

								case '0':
									if (ai == 'a')
									{
										TCA5013_To_value((TCA5013_Register_Map*)&TCA5013_Regs, 0x00);
										TCA5013_Write_All((TCA5013_Register_Map*)&TCA5013_Regs);
									}
									else if(ai == 'n')
									{
										unsigned char x = 0;
										for(x=0;x<71;x++)
										{
											I2C_Write_Byte(TCA5013_ADDRESS,x,0x00);
										}
									}
									break;


								case 'm':
								{
									unsigned char what[] = {0x12,0x34,0xfd,0x42};
									unsigned char TCAreg = 0;
									if(!UartScanfAsciiByte(&TCAreg))
										I2C_Write(4,TCA5013_ADDRESS,TCAreg,what,0);
								}
								break;

								case 's':
								{
									unsigned char tx_buf[] = {0, 0x13, 0x42};
									tx_buf[0] = TCA5013_ADDRESS << 1;
									I2C_Write(2,TCA5013_ADDRESS, tx_buf[0], tx_buf, 1);
									print_UART("Press Any Key\r\n");
									while(!(IFG2&UCA0RXIFG)); //wait until keyboard input
									unsigned char ai = UCA0RXBUF;
									byte_to_Hex_ASCII(I2C_Read_Byte(TCA5013_ADDRESS,0x13));
									print_UART("\r\n");
								}
								break;
							}
						}
						break;
#endif

						case 'r':
						IE2 &= ~UCA0RXIE;
						unsigned char ai = 0;
						print_UART(" a: Read using autoincriment\r\n n: Read without AI\r\n");
						while(!(IFG2&UCA0RXIFG)); //wait until keyboard input
						ai = UCA0RXBUF;
						print_UART("  r: Read all\r\n  u: Read User Card\r\n  1: Read Sam1\r\n  2: Read Sam2\r\n  3: Read Sam3\r\n  i: Read int status\r\n");
						while(!(IFG2&UCA0RXIFG)); //wait until keyboard input
						input = UCA0RXBUF;

						switch(input)
						{
							case 'r':
							{
								unsigned char i2c_retval;
								if(ai == 'a')
								{
									i2c_retval = TCA5013_Read_All((TCA5013_Register_Map*)&TCA5013_Regs);
								}
								else
								{
									i2c_retval = TCA5013_Read_All_noAI((TCA5013_Register_Map*)&TCA5013_Regs);

								}
								if(i2c_retval != I2C_OPERATION_FAIL)
								{
									unsigned char x = 0;
									unsigned char* ptr = (unsigned char*)&TCA5013_Regs;
									for(x=0; x<44;x++)
									{
										byte_to_Hex_ASCII(*ptr++);
										if(x == 13 | x==22 | x==29 | x==36 | x == 43)
										{
											print_UART("\r\n");
										}
									}
								}
								else
								{
									UCB0CTL1 |= UCSWRST;

									NACK = 0;
									print_UART("I2C FAIL\r\n");
									UCB0CTL1 &= ~UCSWRST;
								}
							break;
							}

							case 'u':
							{
								unsigned char i2c_retval;
								if(ai == 'a')
								{
									i2c_retval = TCA5013_Read_UC((TCA5013_Register_Map*)&TCA5013_Regs);
								}
								else
								{
									i2c_retval = TCA5013_Read_UC_noAI((TCA5013_Register_Map*)&TCA5013_Regs);

								}
								if(i2c_retval != I2C_OPERATION_FAIL)
								{
									print_UART("User Card Regs\r\n");
									unsigned char* ptr = (unsigned char*)&TCA5013_Regs.User_Card_Registers;
									unsigned char x = 0;
									for(x=0;x<14;x++)
									{
										byte_to_Hex_ASCII(*ptr++);

									}
									print_UART("\r\n");
								}
								else
								{
									UCB0CTL1 |= UCSWRST;

									NACK = 0;
									print_UART("I2C FAIL\r\n");
									UCB0CTL1 &= ~UCSWRST;
								}
							break;
							}
							case '1':
							{
								unsigned char i2c_retval;
								if(ai == 'a')
								{
									i2c_retval = TCA5013_Read_SAM1((TCA5013_Register_Map*)&TCA5013_Regs);
								}
								else
								{
									i2c_retval = TCA5013_Read_SAM1_noAI((TCA5013_Register_Map*)&TCA5013_Regs);

								}
								if(i2c_retval != I2C_OPERATION_FAIL)
								{
									print_UART("Sam 1 Regs\r\n");
									unsigned char* ptr = (unsigned char*)&TCA5013_Regs.Sam1_Registers;
									unsigned char x = 0;
									for(x=0;x<9;x++)
									{
										byte_to_Hex_ASCII(*ptr++);

									}
									print_UART("\r\n");
								}
								else
								{
									UCB0CTL1 |= UCSWRST;

									NACK = 0;
									print_UART("I2C FAIL\r\n");
									UCB0CTL1 &= ~UCSWRST;
								}
							break;
							}
							case '2':
							{
								unsigned char i2c_retval;
								if(ai == 'a')
								{
									i2c_retval = TCA5013_Read_SAM2((TCA5013_Register_Map*)&TCA5013_Regs);
								}
								else
								{
									i2c_retval = TCA5013_Read_SAM2_noAI((TCA5013_Register_Map*)&TCA5013_Regs);

								}
								if(i2c_retval != I2C_OPERATION_FAIL)
								{
									print_UART("Sam 2 Regs\r\n");
									unsigned char* ptr = (unsigned char*)&TCA5013_Regs.Sam2_Registers;
									unsigned char x = 0;
									for(x=0;x<7;x++)
									{
										byte_to_Hex_ASCII(*ptr++);

									}
									print_UART("\r\n");
								}
								else
								{
									UCB0CTL1 |= UCSWRST;

									NACK = 0;
									print_UART("I2C FAIL\r\n");
									UCB0CTL1 &= ~UCSWRST;
								}
							break;
							}
							case '3':
							{
								unsigned char i2c_retval;
								if(ai == 'a')
								{
									i2c_retval = TCA5013_Read_SAM3((TCA5013_Register_Map*)&TCA5013_Regs);
								}
								else
								{
									i2c_retval = TCA5013_Read_SAM3_noAI((TCA5013_Register_Map*)&TCA5013_Regs);

								}
								if(i2c_retval != I2C_OPERATION_FAIL)
								{
									print_UART("Sam 3 Regs\r\n");
									unsigned char* ptr = (unsigned char*)&TCA5013_Regs.Sam3_Registers;
									unsigned char x = 0;
									for(x=0;x<7;x++)
									{
										byte_to_Hex_ASCII(*ptr++);

									}
									print_UART("\r\n");
								}
								else
								{
									UCB0CTL1 |= UCSWRST;

									NACK = 0;
									print_UART("I2C FAIL\r\n");
									UCB0CTL1 &= ~UCSWRST;
								}
							break;
							}
							case 'i':
							{
								unsigned char i2c_retval;
								if(ai == 'a')
								{
									i2c_retval = TCA5013_Read_INTSTAT((TCA5013_Register_Map*)&TCA5013_Regs);
								}
								else
								{
									i2c_retval = TCA5013_Read_INTSTAT_noAI((TCA5013_Register_Map*)&TCA5013_Regs);

								}
								if(i2c_retval != I2C_OPERATION_FAIL)
								{

									print_UART("Interrupt and Status Regs\r\n");
									unsigned char* ptr = (unsigned char*)&TCA5013_Regs.Int_Mask_Registers;
									unsigned char x = 0;
									for(x=0;x<7;x++)
									{
										byte_to_Hex_ASCII(*ptr++);

									}
									print_UART("\r\n");
								}
								else
								{
									UCB0CTL1 |= UCSWRST;

									NACK = 0;
									print_UART("I2C FAIL\r\n");
									UCB0CTL1 &= ~UCSWRST;
								}

							break;
							}
#if 0
							/*case 'h':{
								unsigned char this[71];

									if(ai == 'a')
									{
										I2C_Read(71,TCA5013_ADDRESS, USER_CARD_STAT | 0x80, this);
										unsigned char x = 0;
										print_UART("Read with holes\r\n");
										for(x=0;x<71;x++)
										{

											byte_to_Hex_ASCII(this[x]);
											if(x==15 | x == 31 | x == 47 | x==63)
												print_UART("\r\n");

										}

									}
									else if (ai =='n')
									{
										unsigned char y =0;
										print_UART("Read with holes\r\n");
										for(y=0;y<71;y++)
										{

											byte_to_Hex_ASCII(I2C_Read_Byte(TCA5013_ADDRESS,y));
											if(y==15 | y == 31 | y == 47 | y==63)
												print_UART("\r\n");
										}

									}
									print_UART("\r\n");



							}
							break; */

							/*case 'n':
							{
								unsigned char reg;
								print_UART("Select register\r\n");
								if(!UartScanfAsciiByte(&reg))
								{
									print_UART("\r\n");
									unsigned char x = 0;

									byte_to_Hex_ASCII(I2C_Read_Byte(TCA5013_ADDRESS, reg));
									for (x=0;x<10;x++)
									{
										__delay_cycles(16000000);
										byte_to_Hex_ASCII(I2C_Read_NOREG(TCA5013_ADDRESS));

									}
									print_UART("\r\n");
								}

								break;


							}
							break;*/
#endif
						}
						break;




					}
					break;

					case 'a':
					{
						selection = 0;
						print_UART("Activation Menu\r\n");
						print_UART("a: Activate Async\r\n");
						print_UART("s: Auto Activate Sync\r\n");
						print_UART("m: Manual Sync Activate\r\n");
						print_UART("1: Activate sam1\r\n");
						print_UART("2: Activate sam2\r\n");
						print_UART("3: Activate sam3\r\n");

						WAIT_KEY;
						selection = UCA0RXBUF;
						switch (selection)
						{
							case 'm':
								print_UART("Select Voltage\r\n");
								set_voltage();


								TCA5013_Regs.User_Card_Registers.Settings.Set_VCC = IO_VCC;
								TCA5013_Regs.User_Card_Registers.Synchronous_Settings.Activation_type = 0;
								TCA5013_Regs.User_Card_Registers.Synchronous_Settings.Card_type = 0;
								TCA5013_Regs.User_Card_Registers.Synchronous_Settings.CLK_Disable = 0;
								TCA5013_Regs.User_Card_Registers.Synchronous_Settings.Start_Sync = 1;

								TCA5013_Regs.User_Card_Registers.Synchronous_Settings.C4 = 1;
								TCA5013_Regs.User_Card_Registers.Synchronous_Settings.C8 = 1;
								TCA5013_Regs.User_Card_Registers.Synchronous_Settings.RST = 1;
								TCA5013_Regs.User_Card_Registers.Clock_Settings.Clk1 = 0x02;


								I2C_Write_Byte(TCA5013_ADDRESS, 0x01, *(unsigned char *) &TCA5013_Regs.User_Card_Registers.Settings);
								I2C_Write_Byte(TCA5013_ADDRESS, 0x09, *(unsigned char *) &TCA5013_Regs.User_Card_Registers.Synchronous_Settings);
								I2C_Write_Byte(TCA5013_ADDRESS, 0x02, *(unsigned char *) &TCA5013_Regs.User_Card_Registers.Clock_Settings);
								//data = I2C_Read_Byte(TCA5013_ADDRESS,0x09);

								__delay_cycles(20000);

								TCA5013_Regs.User_Card_Registers.Clock_Settings.Clk1 = 0x01;
								TCA5013_Regs.User_Card_Registers.Synchronous_Settings.C4 ^= 1;
								I2C_Write_Byte(TCA5013_ADDRESS, 0x02, *(unsigned char *) &TCA5013_Regs.User_Card_Registers.Clock_Settings);
								I2C_Write_Byte(TCA5013_ADDRESS, 0x09, *(unsigned char *) &TCA5013_Regs.User_Card_Registers.Synchronous_Settings);
								__delay_cycles(100);

								TCA5013_Regs.User_Card_Registers.Clock_Settings.Clk1 = 0x02;
								TCA5013_Regs.User_Card_Registers.Synchronous_Settings.C4 ^= 1;
								I2C_Write_Byte(TCA5013_ADDRESS, 0x02, *(unsigned char *) &TCA5013_Regs.User_Card_Registers.Clock_Settings);
								I2C_Write_Byte(TCA5013_ADDRESS, 0x09, *(unsigned char *) &TCA5013_Regs.User_Card_Registers.Synchronous_Settings);
								__delay_cycles(100);

								TCA5013_Regs.User_Card_Registers.Clock_Settings.Clk1 = 0x01;
								TCA5013_Regs.User_Card_Registers.Synchronous_Settings.C8 ^= 1;
								I2C_Write_Byte(TCA5013_ADDRESS, 0x02, *(unsigned char *) &TCA5013_Regs.User_Card_Registers.Clock_Settings);
								I2C_Write_Byte(TCA5013_ADDRESS, 0x09, *(unsigned char *) &TCA5013_Regs.User_Card_Registers.Synchronous_Settings);
								__delay_cycles(100);

								TCA5013_Regs.User_Card_Registers.Clock_Settings.Clk1 = 0x02;
								TCA5013_Regs.User_Card_Registers.Synchronous_Settings.C8 ^= 1;
								I2C_Write_Byte(TCA5013_ADDRESS, 0x02, *(unsigned char *) &TCA5013_Regs.User_Card_Registers.Clock_Settings);
								I2C_Write_Byte(TCA5013_ADDRESS, 0x09, *(unsigned char *) &TCA5013_Regs.User_Card_Registers.Synchronous_Settings);
								__delay_cycles(100);

								TCA5013_Regs.User_Card_Registers.Clock_Settings.Clk1 = 0x01;
								TCA5013_Regs.User_Card_Registers.Synchronous_Settings.RST ^= 1;
								I2C_Write_Byte(TCA5013_ADDRESS, 0x02, *(unsigned char *) &TCA5013_Regs.User_Card_Registers.Clock_Settings);
								I2C_Write_Byte(TCA5013_ADDRESS, 0x09, *(unsigned char *) &TCA5013_Regs.User_Card_Registers.Synchronous_Settings);
								__delay_cycles(100);

								TCA5013_Regs.User_Card_Registers.Clock_Settings.Clk1 = 0x02;
								TCA5013_Regs.User_Card_Registers.Synchronous_Settings.RST ^= 1;
								I2C_Write_Byte(TCA5013_ADDRESS, 0x02, *(unsigned char *) &TCA5013_Regs.User_Card_Registers.Clock_Settings);
								I2C_Write_Byte(TCA5013_ADDRESS, 0x09, *(unsigned char *) &TCA5013_Regs.User_Card_Registers.Synchronous_Settings);
								__delay_cycles(100);

								TCA5013_Regs.User_Card_Registers.Clock_Settings.Clk1 = 0x01;
								TCA5013_Regs.User_Card_Registers.Synchronous_Settings.RST = 0;
								TCA5013_Regs.User_Card_Registers.Synchronous_Settings.C4 = 0;
								TCA5013_Regs.User_Card_Registers.Synchronous_Settings.C8 = 0;
								I2C_Write_Byte(TCA5013_ADDRESS, 0x02, *(unsigned char *) &TCA5013_Regs.User_Card_Registers.Clock_Settings);
								I2C_Write_Byte(TCA5013_ADDRESS, 0x09, *(unsigned char *) &TCA5013_Regs.User_Card_Registers.Synchronous_Settings);
								__delay_cycles(300);

								TCA5013_Regs.User_Card_Registers.Clock_Settings.Clk1 = 0x02;
								TCA5013_Regs.User_Card_Registers.Synchronous_Settings.RST = 1;
								TCA5013_Regs.User_Card_Registers.Synchronous_Settings.C4 = 1;
								TCA5013_Regs.User_Card_Registers.Synchronous_Settings.C8 = 1;
								I2C_Write_Byte(TCA5013_ADDRESS, 0x02, *(unsigned char *) &TCA5013_Regs.User_Card_Registers.Clock_Settings);
								I2C_Write_Byte(TCA5013_ADDRESS, 0x09, *(unsigned char *) &TCA5013_Regs.User_Card_Registers.Synchronous_Settings);




								/*manual: print_UART("Toggle the following\r\n");
								print_UART("c: CLK\r\n");
								print_UART("4: C4\r\n");
								print_UART("8: C8\r\n");
								print_UART("r: RST\r\n");
								print_UART("q: Exit this menu\r\n");
								WAIT_KEY;
								selection = UCA0RXBUF;


								switch(selection)
								{
									case 'c':
									TCA5013_Regs.User_Card_Registers.Clock_Settings.Clk1 ^= 0x2;
									 I2C_Write_Byte(TCA5013_ADDRESS, 0x02, *(unsigned char *) &TCA5013_Regs.User_Card_Registers.Clock_Settings);
									 NACK_CHECK
									 goto manual;
									// break;

									case '4': TCA5013_Regs.User_Card_Registers.Synchronous_Settings.C4 ^= 1;
									I2C_Write_Byte(TCA5013_ADDRESS, 0x09, *(unsigned char *) &TCA5013_Regs.User_Card_Registers.Synchronous_Settings);
									NACK_CHECK
									goto manual;
	//								break;

									case '8':TCA5013_Regs.User_Card_Registers.Synchronous_Settings.C8 ^= 1;
									I2C_Write_Byte(TCA5013_ADDRESS, 0x09, *(unsigned char *) &TCA5013_Regs.User_Card_Registers.Synchronous_Settings);
									NACK_CHECK
									goto manual;
		//							break;

									case 'r':TCA5013_Regs.User_Card_Registers.Synchronous_Settings.RST ^= 1;
									I2C_Write_Byte(TCA5013_ADDRESS, 0x09, *(unsigned char *) &TCA5013_Regs.User_Card_Registers.Synchronous_Settings);
									NACK_CHECK
									goto manual;
			//						break;

									case 'q': break;


									default: goto manual;
								}*/

								break;
								case 'a':
								{
									//TCA5013InitDefault((TCA5013_Register_Map*)&TCA5013_Regs);
									print_UART("Select reset type\r\n");
									print_UART("c: Cold reset\r\n");
									print_UART("w: Warm Reset\r\n");
									print_UART("e: Change Early&Mute\r\n");
									print_UART("x: Switch to External Clock\r\n");
									print_UART("k: Switch to Internal Clock\r\n");

									WAIT_KEY;
									selection = UCA0RXBUF;
									switch(selection)
									{

									case 'k':
									{
										TCA5013_Regs.User_Card_Registers.Clock_Settings.Internal_Clk = 1;
										I2C_Write_Byte(TCA5013_ADDRESS,0x02, *(unsigned char*)&TCA5013_Regs.User_Card_Registers.Clock_Settings);
										NACK_CHECK
									}
									break;

									case 'x':
									{
										TCA5013_Regs.User_Card_Registers.Clock_Settings.Internal_Clk = 0;
										I2C_Write_Byte(TCA5013_ADDRESS,0x02, *(unsigned char*)&TCA5013_Regs.User_Card_Registers.Clock_Settings);
										NACK_CHECK
									}
									break;
#if 0
									case 'e':
										{
											unsigned short early,mute;

											print_UART("Set Early value\r\n");
											UartScanfAsciiU32(&early);
											print_UART("Set Mute value\r\n");
											UartScanfAsciiU32(&mute);


											TCA5013_Regs.User_Card_Registers.Early.MSB.all = early >> 2;
											TCA5013_Regs.User_Card_Registers.Early.LSB.all = early & 0x3;

											TCA5013_Regs.User_Card_Registers.Mute.MSB.all = mute >> 8;
											TCA5013_Regs.User_Card_Registers.Mute.LSB.all = mute & 0xFF;

											goto repeat;
										}
#endif


									case 'c':print_UART("Select Voltage\r\n");
									set_voltage();
									print_UART("Select CLK div\r\n");
									set_clk_div();
#if immediate
									print_UART("1: Immediate activation\r\n");
									print_UART("i: Wait for int activation\r\n");
									WAIT_KEY;
									selection = UCA0RXBUF;
									if(selection == 'i')
									{
										delay_UCact = 1;
									}
#endif

									//TCA5013InitDefault((TCA5013_Register_Map*)&TCA5013_Regs);
									TCA5013_Regs.User_Card_Registers.Settings.Set_VCC = IO_VCC;
									TCA5013_Regs.User_Card_Registers.Settings.Start_Async = 1;
									TCA5013_Regs.User_Card_Registers.Clock_Settings.ClockDiv = IO_clkdiv;


									I2C_Write_Byte(TCA5013_ADDRESS,0x02, *(unsigned char*)&TCA5013_Regs.User_Card_Registers.Clock_Settings);


									//TCA5013_Regs.User_Card_Registers.Early.MSB =
									//I2C_Write(4,TCA5013_ADDRESS,0x83,(unsigned char*)&TCA5013_Regs.User_Card_Registers.Early.MSB,0);
									NACK_CHECK
#if immediate
									if(!delay_UCact)
									{
#endif
										I2C_Write_Byte(TCA5013_ADDRESS,0x01, *(unsigned char*)&TCA5013_Regs.User_Card_Registers.Settings);
										NACK_CHECK
#if immediate
										}
#endif
										break;




									case 'w':

										print_UART("Select Voltage\r\n");
										set_voltage();
										print_UART("Select CLK div\r\n");
										set_clk_div();

										//TCA5013InitDefault((TCA5013_Register_Map*)&TCA5013_Regs);
										TCA5013_Regs.User_Card_Registers.Settings.Set_VCC = IO_VCC;
										TCA5013_Regs.User_Card_Registers.Settings.Start_Async = 1;
										TCA5013_Regs.User_Card_Registers.Settings.Warm = 1;

										TCA5013_Regs.User_Card_Registers.Clock_Settings.ClockDiv = IO_clkdiv;

										//I2C_Write(4,TCA5013_ADDRESS,0x03,(unsigned char*)&TCA5013_Regs.User_Card_Registers.Early.MSB,0);
										//NACK_CHECK
										I2C_Write_Byte(TCA5013_ADDRESS,0x02, *(unsigned char*)&TCA5013_Regs.User_Card_Registers.Clock_Settings);
										NACK_CHECK
										I2C_Write_Byte(TCA5013_ADDRESS,0x01, *(unsigned char*)&TCA5013_Regs.User_Card_Registers.Settings);
										NACK_CHECK
										break;

									}





									//print_UART("Async Card waiting activation\r\n");
								}
							break;

							case '1':
							{
								//TCA5013InitDefault((TCA5013_Register_Map*)&TCA5013_Regs);
								print_UART("Select reset type\r\n");
								print_UART("c: Cold reset\r\n");
								print_UART("w: Warm Reset\r\n");
								//print_UART("e: Change Early&Mute\r\n");

								WAIT_KEY;
								selection = UCA0RXBUF;
								switch(selection)
								{

								case 'k':
								{
									TCA5013_Regs.Sam1_Registers.Clock_Settings.Intern_clk = 1;
									I2C_Write_Byte(TCA5013_ADDRESS,0x12, *(unsigned char*)&TCA5013_Regs.Sam1_Registers.Clock_Settings);
									NACK_CHECK
								}
								break;

								case 'x':
								{
									TCA5013_Regs.Sam1_Registers.Clock_Settings.Intern_clk = 0;
									I2C_Write_Byte(TCA5013_ADDRESS,0x12, *(unsigned char*)&TCA5013_Regs.Sam1_Registers.Clock_Settings);
									NACK_CHECK
								}
								break;

								/*case 'e':
									{
										unsigned short early,mute;

										print_UART("Set Early value\r\n");
										UartScanfAsciiU32(&early);
										print_UART("Set Mute value\r\n");
										UartScanfAsciiU32(&mute);


										TCA5013_Regs.User_Card_Registers.Early.MSB.all = early >> 2;
										TCA5013_Regs.User_Card_Registers.Early.LSB.all = early & 0x3;

										TCA5013_Regs.User_Card_Registers.Mute.MSB.all = mute >> 8;
										TCA5013_Regs.User_Card_Registers.Mute.LSB.all = mute & 0xFF;

										goto repeat;
									}*/



								case 'c':print_UART("Select Voltage\r\n");
								set_voltage();
								print_UART("Select CLK div\r\n");
								set_clk_div();

								//TCA5013InitDefault((TCA5013_Register_Map*)&TCA5013_Regs);
								TCA5013_Regs.Sam1_Registers.Settings.Set_VCC = IO_VCC;
								TCA5013_Regs.Sam1_Registers.Settings.IO_EN = 1;
								TCA5013_Regs.Sam1_Registers.Settings.Start_async = 1;

								TCA5013_Regs.Sam1_Registers.Clock_Settings.Clock_Divide = IO_clkdiv;
								//TCA5013_Regs.User_Card_Registers.Early.MSB =
								I2C_Write_Byte(TCA5013_ADDRESS,0x11, *(unsigned char*)&TCA5013_Regs.Sam1_Registers.Settings);
								NACK_CHECK
								I2C_Write_Byte(TCA5013_ADDRESS,0x12, *(unsigned char*)&TCA5013_Regs.Sam1_Registers.Clock_Settings);
								NACK_CHECK
								//I2C_Write(4,TCA5013_ADDRESS,0x83,(unsigned char*)&TCA5013_Regs.User_Card_Registers.Early.MSB,0);
								break;




								case 'w':

									print_UART("Select Voltage\r\n");
									set_voltage();
									print_UART("Select CLK div\r\n");
									set_clk_div();

									//TCA5013InitDefault((TCA5013_Register_Map*)&TCA5013_Regs);
									TCA5013_Regs.Sam1_Registers.Settings.Set_VCC = IO_VCC;
									TCA5013_Regs.Sam1_Registers.Settings.IO_EN = 1;
									TCA5013_Regs.Sam1_Registers.Settings.Start_async = 1;
									TCA5013_Regs.Sam1_Registers.Settings.Warm = 1;

									TCA5013_Regs.Sam1_Registers.Clock_Settings.Clock_Divide = IO_clkdiv;

									I2C_Write_Byte(TCA5013_ADDRESS,0x11, *(unsigned char*)&TCA5013_Regs.Sam1_Registers.Settings);
									NACK_CHECK
									I2C_Write_Byte(TCA5013_ADDRESS,0x12, *(unsigned char*)&TCA5013_Regs.Sam1_Registers.Clock_Settings);
									NACK_CHECK
									//I2C_Write(4,TCA5013_ADDRESS,0x03,(unsigned char*)&TCA5013_Regs.User_Card_Registers.Early.MSB,0);
									break;

								}





								//print_UART("Async Card waiting activation\r\n");
							}
							break;

							case '2':
							{
								//TCA5013InitDefault((TCA5013_Register_Map*)&TCA5013_Regs);
								print_UART("Select reset type\r\n");
								print_UART("c: Cold reset\r\n");
								print_UART("w: Warm Reset\r\n");
								//print_UART("e: Change Early&Mute\r\n");

								WAIT_KEY;
								selection = UCA0RXBUF;
								switch(selection)
								{
								/*case 'e':
									{
										unsigned short early,mute;

										print_UART("Set Early value\r\n");
										UartScanfAsciiU32(&early);
										print_UART("Set Mute value\r\n");
										UartScanfAsciiU32(&mute);


										TCA5013_Regs.User_Card_Registers.Early.MSB.all = early >> 2;
										TCA5013_Regs.User_Card_Registers.Early.LSB.all = early & 0x3;

										TCA5013_Regs.User_Card_Registers.Mute.MSB.all = mute >> 8;
										TCA5013_Regs.User_Card_Registers.Mute.LSB.all = mute & 0xFF;

										goto repeat;
									}*/

								case 'k':
								{
									TCA5013_Regs.Sam2_Registers.Clock_Settings.Intern_clk = 1;
									I2C_Write_Byte(TCA5013_ADDRESS,0x22, *(unsigned char*)&TCA5013_Regs.Sam2_Registers.Clock_Settings);
									NACK_CHECK
								}
								break;

								case 'x':
								{
									TCA5013_Regs.Sam2_Registers.Clock_Settings.Intern_clk = 0;
									I2C_Write_Byte(TCA5013_ADDRESS,0x22, *(unsigned char*)&TCA5013_Regs.Sam2_Registers.Clock_Settings);
									NACK_CHECK
								}
								break;

								case 'c':print_UART("Select Voltage\r\n");
								set_voltage();
								print_UART("Select CLK div\r\n");
								set_clk_div();

								//TCA5013InitDefault((TCA5013_Register_Map*)&TCA5013_Regs);
								TCA5013_Regs.Sam2_Registers.Settings.Set_VCC = IO_VCC;
								TCA5013_Regs.Sam2_Registers.Settings.IO_EN = 1;
								TCA5013_Regs.Sam2_Registers.Settings.Start_async = 1;

								TCA5013_Regs.Sam2_Registers.Clock_Settings.Clock_Divide = IO_clkdiv;

								//TCA5013_Regs.User_Card_Registers.Early.MSB =
								I2C_Write_Byte(TCA5013_ADDRESS,0x21, *(unsigned char*)&TCA5013_Regs.Sam2_Registers.Settings);
								NACK_CHECK
								I2C_Write_Byte(TCA5013_ADDRESS,0x22, *(unsigned char*)&TCA5013_Regs.Sam2_Registers.Clock_Settings);
								NACK_CHECK
								//I2C_Write(4,TCA5013_ADDRESS,0x83,(unsigned char*)&TCA5013_Regs.User_Card_Registers.Early.MSB,0);
								break;




								case 'w':

									print_UART("Select Voltage\r\n");
									set_voltage();
									print_UART("Select CLK div\r\n");
									set_clk_div();

									//TCA5013InitDefault((TCA5013_Register_Map*)&TCA5013_Regs);
									TCA5013_Regs.Sam2_Registers.Settings.Set_VCC = IO_VCC;
									TCA5013_Regs.Sam2_Registers.Settings.IO_EN = 1;
									TCA5013_Regs.Sam2_Registers.Settings.Start_async = 1;
									TCA5013_Regs.Sam2_Registers.Settings.Warm = 1;
									TCA5013_Regs.Sam2_Registers.Clock_Settings.Clock_Divide = IO_clkdiv;

									I2C_Write_Byte(TCA5013_ADDRESS,0x21, *(unsigned char*)&TCA5013_Regs.Sam2_Registers.Settings);
									NACK_CHECK
									I2C_Write_Byte(TCA5013_ADDRESS,0x22, *(unsigned char*)&TCA5013_Regs.Sam2_Registers.Clock_Settings);
									NACK_CHECK
									//I2C_Write(4,TCA5013_ADDRESS,0x03,(unsigned char*)&TCA5013_Regs.User_Card_Registers.Early.MSB,0);
									break;

								}





								//print_UART("Async Card waiting activation\r\n");
							}
							break;

							case '3':
							{
								//TCA5013InitDefault((TCA5013_Register_Map*)&TCA5013_Regs);
								print_UART("Select reset type\r\n");
								print_UART("c: Cold reset\r\n");
								print_UART("w: Warm Reset\r\n");
								//print_UART("e: Change Early&Mute\r\n");

								WAIT_KEY;
								selection = UCA0RXBUF;
								switch(selection)
								{
								/*case 'e':
									{
										unsigned short early,mute;

										print_UART("Set Early value\r\n");
										UartScanfAsciiU32(&early);
										print_UART("Set Mute value\r\n");
										UartScanfAsciiU32(&mute);


										TCA5013_Regs.User_Card_Registers.Early.MSB.all = early >> 2;
										TCA5013_Regs.User_Card_Registers.Early.LSB.all = early & 0x3;

										TCA5013_Regs.User_Card_Registers.Mute.MSB.all = mute >> 8;
										TCA5013_Regs.User_Card_Registers.Mute.LSB.all = mute & 0xFF;

										goto repeat;
									}*/

								case 'k':
								{
									TCA5013_Regs.Sam3_Registers.Clock_Settings.Intern_clk = 1;
									I2C_Write_Byte(TCA5013_ADDRESS,0x32, *(unsigned char*)&TCA5013_Regs.Sam3_Registers.Clock_Settings);
									NACK_CHECK
								}
								break;

								case 'x':
								{
									TCA5013_Regs.Sam3_Registers.Clock_Settings.Intern_clk = 0;
									I2C_Write_Byte(TCA5013_ADDRESS,0x32, *(unsigned char*)&TCA5013_Regs.Sam3_Registers.Clock_Settings);
									NACK_CHECK
								}
								break;

								case 'c':print_UART("Select Voltage\r\n");
								set_voltage();
								print_UART("Select CLK div\r\n");
								set_clk_div();

								//TCA5013InitDefault((TCA5013_Register_Map*)&TCA5013_Regs);
								TCA5013_Regs.Sam3_Registers.Settings.Set_VCC = IO_VCC;
								TCA5013_Regs.Sam3_Registers.Settings.IO_EN = 1;
								TCA5013_Regs.Sam3_Registers.Settings.Start_async = 1;

								TCA5013_Regs.Sam3_Registers.Clock_Settings.Clock_Divide = IO_clkdiv;

								//TCA5013_Regs.User_Card_Registers.Early.MSB =
								I2C_Write_Byte(TCA5013_ADDRESS,0x31, *(unsigned char*)&TCA5013_Regs.Sam3_Registers.Settings);
								NACK_CHECK
								I2C_Write_Byte(TCA5013_ADDRESS,0x32, *(unsigned char*)&TCA5013_Regs.Sam3_Registers.Clock_Settings);
								NACK_CHECK
								//I2C_Write(4,TCA5013_ADDRESS,0x83,(unsigned char*)&TCA5013_Regs.User_Card_Registers.Early.MSB,0);
								break;




								case 'w':

									print_UART("Select Voltage\r\n");
									set_voltage();
									print_UART("Select CLK div\r\n");
									set_clk_div();

									//TCA5013InitDefault((TCA5013_Register_Map*)&TCA5013_Regs);
									TCA5013_Regs.Sam3_Registers.Settings.Set_VCC = IO_VCC;
									TCA5013_Regs.Sam3_Registers.Settings.IO_EN = 1;
									TCA5013_Regs.Sam3_Registers.Settings.Start_async = 1;
									TCA5013_Regs.Sam3_Registers.Settings.Warm = 1;
									TCA5013_Regs.Sam3_Registers.Clock_Settings.Clock_Divide = IO_clkdiv;

									I2C_Write_Byte(TCA5013_ADDRESS,0x31, *(unsigned char*)&TCA5013_Regs.Sam3_Registers.Settings);
									NACK_CHECK
									I2C_Write_Byte(TCA5013_ADDRESS,0x32, *(unsigned char*)&TCA5013_Regs.Sam3_Registers.Clock_Settings);
									NACK_CHECK
									//I2C_Write(4,TCA5013_ADDRESS,0x03,(unsigned char*)&TCA5013_Regs.User_Card_Registers.Early.MSB,0);
									break;

								}





								//print_UART("Async Card waiting activation\r\n");
							}
							break;





							case 's':
							{
								//TCA5013InitDefault((TCA5013_Register_Map*)&TCA5013_Regs);
								print_UART("Select Voltage\r\n");
								set_voltage();
								print_UART("Select Card type\r\n");
								set_card_type();
								//print_UART("Select CLK div\r\n");
								//set_clk_div();






								TCA5013_Regs.User_Card_Registers.Settings.Set_VCC = IO_VCC;
								TCA5013_Regs.User_Card_Registers.Synchronous_Settings.Card_type = card_type;
								TCA5013_Regs.User_Card_Registers.Synchronous_Settings.Start_Sync = 1;
								TCA5013_Regs.User_Card_Registers.Synchronous_Settings.CLK_Disable = 1;

								I2C_Write_Byte(TCA5013_ADDRESS, 0x01, *(unsigned char*)&TCA5013_Regs.User_Card_Registers.Settings);
								I2C_Write_Byte(TCA5013_ADDRESS, 0x09, *(unsigned char*)&TCA5013_Regs.User_Card_Registers.Synchronous_Settings);


								NACK_CHECK
								NACK_CHECK
																//I2C_Write_Byte(TCA5013_ADDRESS, 0x02, *(unsigned char*)&TCA5013_Regs.User_Card_Registers.Clock_Settings);
																//NACK_CHECK

								run_clocktest = 1;

							}
							break;
							/*case 'd':
								TCA5013_Regs.User_Card_Registers.Synchronous_Settings.Start_Sync = 0;
								I2C_Write_Byte(TCA5013_ADDRESS, 0x09, *(unsigned char*)&TCA5013_Regs.User_Card_Registers.Synchronous_Settings);
								break;*/
						}

					}
					break;




				}




			}

		}




	}
}



/*******************************************************************************
*                                 Functions                                    *
*******************************************************************************/
#if 0
unsigned char UartScanfAsciiU32 (unsigned short* res){
	unsigned char len=0,i;
	unsigned char ringbuffer[7];
	unsigned char errorflag = 0;
	unsigned short result=0;

	while(len<7){ //check for RETURN hit or buffer overflow 11 digits max
		while(!(IFG2&UCA0RXIFG)); // Waituntil keyboard input
		ringbuffer[len] = UCA0RXBUF;
		while (!(IFG2&UCA0TXIFG));// USCI_A0 TX buffer ready?
		if(ringbuffer[len]==0x0D) break;
		UCA0TXBUF = ringbuffer[len]; // send back the received character
		len++;
	}

	if(ringbuffer[0]!='0') {errorflag = 1;}
	else
	{
		for(i=0;i<len-2;i++){ //must subtract the first 2 prefix characters
			result = result <<4; //one digit is processed at a time e.g. 4bits
			if((ringbuffer[i+2] <= '9')&&(ringbuffer[i+2] >= '0')){
				result += (ringbuffer[i+2]-'0');
			}
			else if((ringbuffer[i+2] >= 'A')&&(ringbuffer[i+2] <= 'F')){
				result += (ringbuffer[i+2]-'7');
			}
			else if((ringbuffer[i+2] >= 'a')&&(ringbuffer[i+2] <= 'f')){
				result += (ringbuffer[i+2] -'W');
			}
			else {errorflag = 1; break;}
		}
	}

	//binary processing

	*res=result;
	return errorflag;
}

#endif

void set_voltage(void)
{
	selection =0;
	//print_UART("Select voltage\r\n");
	print_UART("1: 1.8V\r\n");
	print_UART("2: 3.3V\r\n");
	print_UART("3: 5V\r\n");
	WAIT_KEY;
	selection = UCA0RXBUF;
	switch(selection)
	{
		case '1': //TCA5013_Regs.User_Card_Registers.Settings.Set_VCC = V_1pt8;

			IO_VCC = V_1pt8;
		break;
		case '2': //TCA5013_Regs.User_Card_Registers.Settings.Set_VCC = V_3pt3;

			IO_VCC = V_3pt3;
		break;
		case '3': //TCA5013_Regs.User_Card_Registers.Settings.Set_VCC = V_5;

			IO_VCC = V_5;
		break;
	}
}

void set_clk_div(void)
{
	selection =0;
	//print_UART("Select CLK div\r\n");
	print_UART("1: 1\r\n");
	print_UART("2: 2\r\n");
	print_UART("4: 4\r\n");
	print_UART("5: 5\r\n");
	print_UART("8: 8\r\n");
	WAIT_KEY;
	selection = UCA0RXBUF;
	switch(selection)
	{
		case '1': IO_clkdiv = div_1;
		break;
		case '2': IO_clkdiv = div_2;
		break;
		case '4': IO_clkdiv = div_4;
		break;
		case '5': IO_clkdiv = div_5;
		break;
		case '8': IO_clkdiv = div_8;
		break;
	}
}


void set_card_type(void)
{
	selection =0;
	//print_UART("Select Card type\r\n");
	print_UART("1: Type 1\r\n");
	print_UART("2: Type 2\r\n");
	WAIT_KEY;
	selection = UCA0RXBUF;
	switch(selection)
	{
		case '1': card_type = 0;
		break;
		case '2': card_type = 1;
		break;
	}
}
/**************************************************************************************************************************************************
*  write_UART_byte
**************************************************************************************************************************************************/
/*
* The Function will write a single byte to the UART
*
*
*
**************************************************************************************************************************************************/
void write_UART_byte(unsigned char byte)
{
	while(!(IFG2&UCA0TXIFG));
	UCA0TXBUF = byte;
}

/**************************************************************************************************************************************************
*  print_UART
**************************************************************************************************************************************************/
/*!
*
* The Function will a string over UART
*
**************************************************************************************************************************************************/
void print_UART(char* string)
{
	while(*string)
	{
		write_UART_byte(*string++);
	}
}


void print_Menu(void)
{
	print_UART("Select an option below\r\n");
	print_UART("i: I2C Menu\r\n");
	print_UART("a: Card Activation menu\r\n");
	print_UART("d: Deactivate a card\r\n");
	print_UART("p: Change Pres default\r\n");
	print_UART("c: Clock menu\r\n");
	print_UART("h: Assert/De-assert Shutdown\r\n");
	print_UART("x: Reset TCA5013\r\n");
}

/**************************************************************************************************************************************************
*  byte_to_ASCII
**************************************************************************************************************************************************/
/*!
* Converts decimal number to 3 byte ASCII string and outputs the result over UART
*
**************************************************************************************************************************************************/
void byte_to_ASCII(unsigned char key_number)
{
	char key_number_array[3];
	signed char i = 0;
	for(i=0; i<3; i++)
	{
	  key_number_array[i] = '0' + (key_number % 10);
	  key_number = key_number-(key_number%10);
	  key_number = key_number/10;
	}
	print_UART(" ");
	write_UART_byte(key_number_array[2]);
	write_UART_byte(key_number_array[1]);
	write_UART_byte(key_number_array[0]);
}

unsigned char UartScanfAsciiByte (unsigned char* res){
	unsigned char len=0,i;
	unsigned char result=0;
	unsigned char ringbuffer[10];
	unsigned char errorflag = 0;

	while(len<11){ //check for RETURN hit or buffer overflow
		while(!(IFG2&UCA0RXIFG)); // Waituntil keyboard input
		ringbuffer[len] = UCA0RXBUF;
		while (!(IFG2&UCA0TXIFG));// USCI_A0 TX buffer ready?
		if(ringbuffer[len]==0x0D) break;
		UCA0TXBUF = ringbuffer[len]; // send back the received character
		len++;
	}
	switch(ringbuffer[1]){
		//hex processing fromat = 0xaa or 0Xaa or 0XAA or 0xAA
		case 'x':
		case 'X':
			if(ringbuffer[0]!='0') {errorflag = 1; break;}
			for(i=0;i<len-2;i++){ //must subtract the first 2 prefix characters
				result = result <<4; //one digit is processed at a time e.g. 4bits
				if((ringbuffer[i+2] <= '9')&&(ringbuffer[i+2] >= '0')){
					result += (ringbuffer[i+2]-'0');
				}
				else if((ringbuffer[i+2] >= 'A')&&(ringbuffer[i+2] <= 'F')){
					result += (ringbuffer[i+2]-'7');
				}
				else if((ringbuffer[i+2] >= 'a')&&(ringbuffer[i+2] <= 'f')){
					result += (ringbuffer[i+2] -'W');
				}
				else {errorflag = 1; break;}
			}
			break;
			//binary processing
		case 'b':
		case 'B':
			if(ringbuffer[0]!='0') {errorflag = 2; break;}
			for(i=2;i<len;i++){
				result = result << 1;
				if	(ringbuffer[i] == '1') {
					result+=1;
				}
				else if (ringbuffer[i] == '0') {
;
				}

				else {errorflag = 3;break;}
			}
			break;
			//decimal processing or error
		default:
			for(i=0;i<len;i++){
				if((ringbuffer[i]>='0')&&(ringbuffer[i]<='9')){
					result *=10;
					result += ringbuffer[i] - '0';
				}
				else errorflag = 4; //non numerical character
			}
			break;
		}

		if(errorflag==0) {
			*res=result;
			return 0;
		}
		else
			return errorflag;
}


void byte_to_Hex_ASCII(unsigned char data)
{
	char hexArray[2];
	hexArray[1] = data & 0xF;
	hexArray[0] = data >> 4;
	char i = 0;
	for(i=0; i<2; i++)
	{
		if(hexArray[i] > 0x9)
		{
			hexArray[i] += 0x37;
		}
		else
		{
			hexArray[i] += '0';
		}
	}
	print_UART("0x");
	write_UART_byte(hexArray[0]);
	write_UART_byte(hexArray[1]);
	print_UART(", ");
}

/**************************************************************************************************************************************************
*  CLOCK_INIT
**************************************************************************************************************************************************/
/*!
* Setup clock on MSP430
*
**************************************************************************************************************************************************/
void CLOCK_INIT(void)
{
	BCSCTL2 = SELM_0 + DIVM_0 + DIVS_0;

	if (CALBC1_16MHZ != 0xFF)
	{
		/* Adjust this accordingly to your VCC rise time */
		__delay_cycles(100000);
		// Follow recommended flow. First, clear all DCOx and MODx bits. Then
		// apply new RSELx values. Finally, apply new DCOx and MODx bit values.
		DCOCTL = 0x00;
		BCSCTL1 = CALBC1_16MHZ;      							/* Set DCO to 8MHz */
		DCOCTL = CALDCO_16MHZ;
	}

	BCSCTL1 |= XT2OFF + DIVA_0;
	BCSCTL3 = XT2S_0 + LFXT1S_2 + XCAP_1;
}

/**************************************************************************************************************************************************
*  PORT_INIT
**************************************************************************************************************************************************/
/*!
*  UART = 1.1 & 1.2; I2C = 1.6 & 1.7; LED_OUT = 1.0; /RESET = 1.4; /INT = 1.5; /CAD_INT = 2.0
*
**************************************************************************************************************************************************/
void PORT_INIT(void)
{
	// P1.1 = RXD, P1.2=TXD P1.4=SMCLK P1.7=SDA P1.6=SCL
	P1SEL 	= BIT1 + BIT2 + BIT6 + BIT7;
	P1SEL2 	= BIT1 + BIT2 + BIT6 + BIT7;

	// P2.0 = TIMER1_TX P2.1 = TIMER1_RX
	P2SEL = BIT0 + BIT1 + BIT6 + BIT7;
	//P2SEL = BIT1 + BIT6 + BIT7;
	// Red LED on launchpad set to output P1.4 EN set to output
	P1DIR = BIT0 + BIT5 + BIT3;

	// TIMER1_TX out
	//P2OUT = BIT0;
	//P2DIR = BIT0;
	P2IES = BIT5;   // Trigger interrupt on falling edge of P2.5
	P2IFG = 0;
	P2IE = BIT5;	// Enable Interrupt on P2.5

}

/**************************************************************************************************************************************************
*  UART_I2C_INIT
**************************************************************************************************************************************************/
/*!
*
**************************************************************************************************************************************************/
void UART_I2C_INIT(void)
{
	// SMCLK
	UCA0CTL1 |= UCSSEL_2 + UCSWRST;
	UCA0BR0 = 0x82;
	UCA0BR1 = 0x06;
	UCA0MCTL = UCBRS2 + UCBRS0;
	UCA0CTL1 &= ~UCSWRST;


	//Set UCSWRST to configure I2C bus
	UCB0CTL1 |= UCSWRST;

	//I2C, synchronous master mode
	UCB0CTL0 = UCMST + UCMODE_3 + UCSYNC;

	//Use 8MHz SMCLK
	UCB0CTL1 = UCSSEL_2 + UCSWRST;

	//Set Bitrate register
	UCB0BR0 = 255;

	//Reset UCSWRST to enable I2C communication
	UCB0CTL1 &= ~UCSWRST;

	// Enable USCI_A0 RX interrupt
	IE2 |= UCA0RXIE;
}

void TIMER_INIT(void)
{

	TA1CCTL0 = OUT;				//Timer A1 compare mode, outmode 0
	TA1CCTL1 = SCS + CM1 + CAP + CCIE;
	TA1CTL = TASSEL_2 + ID_0 + MC_2;   //SMCLK continuous mode
}


//void print_Menu(void)
//{
	//print_UART("r: Read all I2C Registers\r\n");
	//print_UART("c: Read all I2C Registers\r\n")


//}
/*******************************************************************************
*                                 Interrupts                            *
*******************************************************************************/

/*******************************************************
 * 		   Interrupt service routine for USCI          *
 *-----------------------------------------------------*
 * 	I2C interrupt routines for Transmit and Receive	   *
 *******************************************************/
#pragma vector=USCIAB0TX_VECTOR
__interrupt void USCI0TX_ISR_HOOK(void)
{
	unsigned short lowPowerBits = 0;

    if (IFG2 & UCB0TXIFG) {
		/* USCI_B0 Transmit Interrupt Handler */
		lowPowerBits = USCIAB0TX_ISR();

		/* Exit low power mode based on return bits */
		_bic_SR_register_on_exit(lowPowerBits);
    }
    else {
		/* USCI_B0 Receive Interrupt Handler */
		USCIAB0RX_ISR();

		/* Enter active mode on exit */
		_bic_SR_register_on_exit(LPM4_bits);
    }
}


/*******************************************************
 * 		   Interrupt service routine for USCI          *
 *-----------------------------------------------------*
 * 	Detects if I2C not acknowledge occurs during	   *
 * 	I2C communication								   *
 *******************************************************/
#pragma vector=USCIAB0RX_VECTOR
__interrupt void USCI0RX_ISR(void)
{
	if(UCB0STAT & UCNACKIFG)
	{
		// Set external variable for NACK
		NACK=1;

		// Clear NACK interrupt flag
		UCB0STAT &= ~UCNACKIFG;

		// wake CPU
		_bic_SR_register_on_exit(LPM0_bits);
	}
	else
	{
		RX_buf = UCA0RXBUF;
		wake_Flag |= 0x40;
		_bic_SR_register_on_exit(LPM0_bits);
	}
}

#pragma vector=PORT2_VECTOR
__interrupt void PORT2_ISR(void)
{
	P2IFG &= ~BIT5;
	wake_Flag |= TCA5013_Interrupt;
	_bic_SR_register_on_exit(LPM0_bits);
}

#pragma vector=TIMER1_A0_VECTOR
__interrupt void TIMERA0_ISR(void)
{
	Timer_TX_ISR();

}


#pragma vector=TIMER1_A1_VECTOR
__interrupt void TIMERA1_ISR(void)
{
	unsigned short lowPowerBits = 0;

	lowPowerBits = RX_ISR();

	if(lowPowerBits & LPM0_bits)
		wake_Flag |= IO_Data_Received;

	_bic_SR_register_on_exit(lowPowerBits);
}
