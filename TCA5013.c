#include "TCA5013.h"
#include "cubic_i2C.h"

#define CLR_NACK NACK = 0
#define REF_MACRO *(unsigned char*)&

extern unsigned char TCA5013_ADDRESS;

extern unsigned char NACK;

// ****************************************************************************
//! @fn
//! @brief
//!
//!
//!
//!
//!
//! ****************************************************************************
void TCA5013InitDefault(TCA5013_Register_Map* Regs)
{
	*(unsigned char *)&Regs->User_Card_Registers.Settings = 0x60;

	/*Regs->User_Card_Registers.Settings.Set_VCC = 1;
	Regs->User_Card_Registers.Settings.IOEN = 0;
	Regs->User_Card_Registers.Settings.Warm = 0;
	Regs->User_Card_Registers.Settings.Card_detect = 0;
	Regs->User_Card_Registers.Settings.Start_Async = 0;*/

	REF_MACRO Regs->User_Card_Registers.Clock_Settings = 0x0C;

	/*Regs->User_Card_Registers.Clock_Settings.Internal_Clk = 0;
	Regs->User_Card_Registers.Clock_Settings.Clk0 = 0;
	Regs->User_Card_Registers.Clock_Settings.Clk1 = 0;
	Regs->User_Card_Registers.Clock_Settings.ClockDiv = 1;*/



	Regs->User_Card_Registers.Early.MSB.all = 0xAA;
	Regs->User_Card_Registers.Early.LSB.all = 0;

	Regs->User_Card_Registers.Mute.MSB.all = 0xA4;
	Regs->User_Card_Registers.Mute.LSB.all = 0x74;

	Regs->User_Card_Registers.IO_Slew.Rise_Time = 4;
	Regs->User_Card_Registers.IO_Slew.Fall_Time = 0;

	Regs->User_Card_Registers.Clk_Slew.Rise_Fall_Time = 0xA;

	REF_MACRO Regs->User_Card_Registers.Synchronous_Settings = 0x76;
	/*Regs->User_Card_Registers.Synchronous_Settings.Card_type = 0;
	Regs->User_Card_Registers.Synchronous_Settings.Activation_type = 1;
	Regs->User_Card_Registers.Synchronous_Settings.C4 = 1;
	Regs->User_Card_Registers.Synchronous_Settings.C8 = 1;
	Regs->User_Card_Registers.Synchronous_Settings.RST = 0;
	Regs->User_Card_Registers.Synchronous_Settings.CLK_Disable = 1;
	Regs->User_Card_Registers.Synchronous_Settings.Edge = 1;
	Regs->User_Card_Registers.Synchronous_Settings.Start_Sync = 0;*/

	Regs->User_Card_Registers.ATR_Byte1.all = 0;
	Regs->User_Card_Registers.ATR_Byte2.all = 0;
	Regs->User_Card_Registers.ATR_Byte3.all = 0;
	Regs->User_Card_Registers.ATR_Byte4.all = 0;

	REF_MACRO Regs->Sam1_Registers.Settings = 0x40;
	/*Regs->Sam1_Registers.Settings.Set_VCC = 1;
	Regs->Sam1_Registers.Settings.IO_EN = 1;
	Regs->Sam1_Registers.Settings.Warm = 0;
	Regs->Sam1_Registers.Settings.Start_async = 1;*/


	REF_MACRO Regs->Sam1_Registers.Clock_Settings = 0x0C;
	/*Regs->Sam1_Registers.Clock_Settings.Intern_clk = 0;
	Regs->Sam1_Registers.Clock_Settings.Clk0 = 0;
	Regs->Sam1_Registers.Clock_Settings.Clk1 = 0;
	Regs->Sam1_Registers.Clock_Settings.Clock_Divide = 1;*/

	Regs->Sam1_Registers.Early.MSB.all = 0xAA;
	Regs->Sam1_Registers.Early.LSB.all = 0;

	Regs->Sam1_Registers.Mute.MSB.all = 0xA4;
	Regs->Sam1_Registers.Mute.LSB.all = 0x74;

	Regs->Sam1_Registers.IO_Slew.Rise_Time = 0x04;
	Regs->Sam1_Registers.IO_Slew.Fall_Time = 0;

	Regs->Sam1_Registers.Clk_Slew.Rise_Fall_Time = 0xA;

	Regs->Sam2_Registers.Settings.Set_VCC = 1;
	Regs->Sam2_Registers.Settings.IO_EN = 0;
	Regs->Sam2_Registers.Settings.Warm = 0;
	Regs->Sam2_Registers.Settings.Start_async = 0;

	REF_MACRO Regs->Sam2_Registers.Clock_Settings = 0x0C;
	/*Regs->Sam2_Registers.Clock_Settings.Intern_clk = 0;
	Regs->Sam2_Registers.Clock_Settings.Clk0 = 0;
	Regs->Sam2_Registers.Clock_Settings.Clk1 = 0;
	Regs->Sam2_Registers.Clock_Settings.Clock_Divide = 1;*/

	Regs->Sam2_Registers.Early.MSB.all = 0xAA;
	Regs->Sam2_Registers.Early.LSB.all = 0;

	Regs->Sam2_Registers.Mute.MSB.all = 0xA4;
	Regs->Sam2_Registers.Mute.LSB.all = 0x74;

	Regs->Sam3_Registers.Settings.Set_VCC = 1;
	Regs->Sam3_Registers.Settings.IO_EN = 0;
	Regs->Sam3_Registers.Settings.Warm = 0;
	Regs->Sam3_Registers.Settings.Start_async = 0;

	REF_MACRO Regs->Sam3_Registers.Clock_Settings = 0x0C;
	/*Regs->Sam3_Registers.Clock_Settings.Intern_clk = 0;
	Regs->Sam3_Registers.Clock_Settings.Clk0 = 0;
	Regs->Sam3_Registers.Clock_Settings.Clk1 = 0;
	Regs->Sam3_Registers.Clock_Settings.Clock_Divide = 1;*/

	Regs->Sam3_Registers.Early.MSB.all = 0xAA;
	Regs->Sam3_Registers.Early.LSB.all = 0;

	Regs->Sam3_Registers.Mute.MSB.all = 0xA4;
	Regs->Sam3_Registers.Mute.LSB.all = 0x74;

	REF_MACRO Regs->Int_Mask_Registers.Device_Settings = 0x80;
	/*Regs->Int_Mask_Registers.Device_Settings.GPIO1 = 0;
	Regs->Int_Mask_Registers.Device_Settings.GPIO2 = 0;
	Regs->Int_Mask_Registers.Device_Settings.GPIO3 = 0;
	Regs->Int_Mask_Registers.Device_Settings.GPIO4 = 0;*/


	Regs->Int_Mask_Registers.GPIO_Settings.GPIO1_IN = 0;
	Regs->Int_Mask_Registers.GPIO_Settings.GPIO2_IN = 0;
	Regs->Int_Mask_Registers.GPIO_Settings.GPIO3_IN = 0;
	Regs->Int_Mask_Registers.GPIO_Settings.GPIO4_IN = 0;
	Regs->Int_Mask_Registers.GPIO_Settings.GPIO1_OUT = 1;
	Regs->Int_Mask_Registers.GPIO_Settings.GPIO2_OUT = 1;
	Regs->Int_Mask_Registers.GPIO_Settings.GPIO3_OUT = 1;
	Regs->Int_Mask_Registers.GPIO_Settings.GPIO4_OUT = 1;

	Regs->Int_Mask_Registers.UC_Int_Mask.Early_UserCard = 0;
	Regs->Int_Mask_Registers.UC_Int_Mask.Mute_UserCard = 0;
	Regs->Int_Mask_Registers.UC_Int_Mask.Prot_UserCard = 0;
	Regs->Int_Mask_Registers.UC_Int_Mask.Sync_Complete = 0;
	Regs->Int_Mask_Registers.UC_Int_Mask.OTP = 0;
	Regs->Int_Mask_Registers.UC_Int_Mask.Supervisor = 0;
	Regs->Int_Mask_Registers.UC_Int_Mask.GPIO = 0;

	Regs->Int_Mask_Registers.Sam1_2_Int_Mask.Early_Sam1 = 0;
	Regs->Int_Mask_Registers.Sam1_2_Int_Mask.Mute_Sam1 = 0;
	Regs->Int_Mask_Registers.Sam1_2_Int_Mask.Prot_Sam1 = 0;
	Regs->Int_Mask_Registers.Sam1_2_Int_Mask.Early_Sam2 = 0;
	Regs->Int_Mask_Registers.Sam1_2_Int_Mask.Mute_Sam2 = 0;
	Regs->Int_Mask_Registers.Sam1_2_Int_Mask.Prot_Sam2 = 0;

	Regs->Int_Mask_Registers.Sam3_GPIO_Int_Mask.Early_Sam3 = 0;
	Regs->Int_Mask_Registers.Sam3_GPIO_Int_Mask.Mute_Sam3 = 0;
	Regs->Int_Mask_Registers.Sam3_GPIO_Int_Mask.Prot_Sam3 = 0;
	Regs->Int_Mask_Registers.Sam3_GPIO_Int_Mask.GPIO1 = 0;
	Regs->Int_Mask_Registers.Sam3_GPIO_Int_Mask.GPIO2 = 0;
	Regs->Int_Mask_Registers.Sam3_GPIO_Int_Mask.GPIO3 = 0;
	Regs->Int_Mask_Registers.Sam3_GPIO_Int_Mask.GPIO4 = 0;
}

void TCA5013_To_value(TCA5013_Register_Map* Regs, unsigned char value){
	unsigned char* ptr = (unsigned char*) Regs;
	unsigned char x =0;
	for(x=0;x<44;x++)
	{
		*ptr++ = value;
	}
}


// ****************************************************************************
//! @fn          void TCA6408AInitI2CReg(TCA6408ARegs* Regs)
//! @brief
//! @return		 Returns I2C success or failure
//!
// ****************************************************************************
unsigned char TCA5013_Write_All(TCA5013_Register_Map* Regs)
{
	unsigned char return_Value = I2C_OPERATION_SUCCESSFUL;
	CLR_NACK;
	I2C_Write(14,TCA5013_ADDRESS, USER_CARD_STAT | 0x80, (unsigned char *) &Regs->User_Card_Registers,0);
	I2C_Write(9,TCA5013_ADDRESS, SAM1_STAT | 0x80, (unsigned char *) &Regs->Sam1_Registers,0);
	I2C_Write(7,TCA5013_ADDRESS, SAM2_STAT | 0x80, (unsigned char *) &Regs->Sam2_Registers,0);
	I2C_Write(7,TCA5013_ADDRESS, SAM3_STAT | 0x80, (unsigned char *) &Regs->Sam3_Registers,0);
	I2C_Write(7,TCA5013_ADDRESS, DEVICE_SETTINGS | 0x80, (unsigned char *) &Regs->Int_Mask_Registers,0);
	if(NACK)
	{
		NACK = 0;

		return_Value = I2C_OPERATION_FAIL;
	}
	return return_Value;
}

// ****************************************************************************
//! @fn          void TCA6408AInitI2CReg(TCA6408ARegs* Regs)
//! @brief
//! @return		 Returns I2C success or failure
//!
// ****************************************************************************
unsigned char TCA5013_Read_All(TCA5013_Register_Map* Regs)
{
	unsigned char return_Value = I2C_OPERATION_SUCCESSFUL;
	CLR_NACK;
	I2C_Read(14,TCA5013_ADDRESS, USER_CARD_STAT | 0x80, (unsigned char *) &Regs->User_Card_Registers);
	I2C_Read(9,TCA5013_ADDRESS, SAM1_STAT | 0x80, (unsigned char *) &Regs->Sam1_Registers);
	I2C_Read(7,TCA5013_ADDRESS, SAM2_STAT | 0x80, (unsigned char *) &Regs->Sam2_Registers);
	I2C_Read(7,TCA5013_ADDRESS, SAM3_STAT | 0x80, (unsigned char *) &Regs->Sam3_Registers);
	I2C_Read(7,TCA5013_ADDRESS, VERSION | 0x80, (unsigned char *) &Regs->Int_Mask_Registers);
	if(NACK)
	{
		NACK = 0;

		return_Value = I2C_OPERATION_FAIL;
	}
	return return_Value;
}

unsigned char TCA5013_Read_All_noAI(TCA5013_Register_Map* Regs)
{
	unsigned char return_Value = I2C_OPERATION_SUCCESSFUL;
	unsigned char* ptr = (unsigned char*) Regs;
	unsigned char x = 0;
	unsigned char reg_Address = USER_CARD_STAT;
	for(x=0;x<44;x++)
	{
		*ptr++ = I2C_Read_Byte(TCA5013_ADDRESS, reg_Address++);
		switch(x)
		{
			case 13: reg_Address = SAM1_STAT;
			break;
			case 22: reg_Address = SAM2_STAT;
			break;
			case 29: reg_Address = SAM3_STAT;
			break;
			case 36: reg_Address = VERSION;
			break;
		}
	}
	if(NACK)
	{
		NACK = 0;

		return_Value = I2C_OPERATION_FAIL;
	}
	return return_Value;
}

unsigned char TCA5013_Read_UC(TCA5013_Register_Map* Regs)
{
	unsigned char return_Value = I2C_OPERATION_SUCCESSFUL;
	CLR_NACK;
	I2C_Read(14,TCA5013_ADDRESS, USER_CARD_STAT | 0x80, (unsigned char *) &Regs->User_Card_Registers);
	if(NACK)
	{
		NACK = 0;

		return_Value = I2C_OPERATION_FAIL;
	}
	return return_Value;
}


unsigned char TCA5013_Read_UC_noAI(TCA5013_Register_Map* Regs)
{
	unsigned char return_Value = I2C_OPERATION_SUCCESSFUL;
	unsigned char* ptr = (unsigned char*) Regs;
	unsigned char x = 0;
	unsigned char reg_Address = USER_CARD_STAT;
	for(x=0;x<14;x++)
	{
		*ptr++ = I2C_Read_Byte(TCA5013_ADDRESS, reg_Address++);
	}
	if(NACK)
	{
		NACK = 0;

		return_Value = I2C_OPERATION_FAIL;
	}
	return return_Value;
}

unsigned char TCA5013_Read_SAM1(TCA5013_Register_Map* Regs)
{
	unsigned char return_Value = I2C_OPERATION_SUCCESSFUL;
	CLR_NACK;
	I2C_Read(9,TCA5013_ADDRESS, SAM1_STAT | 0x80, (unsigned char *) &Regs->Sam1_Registers);
	if(NACK)
	{
		NACK = 0;

		return_Value = I2C_OPERATION_FAIL;
	}
	return return_Value;
}


unsigned char TCA5013_Read_SAM1_noAI(TCA5013_Register_Map* Regs)
{
	unsigned char return_Value = I2C_OPERATION_SUCCESSFUL;
	unsigned char* ptr = (unsigned char*) &Regs->Sam1_Registers;
	unsigned char x = 0;
	unsigned char reg_Address = SAM1_STAT;
	for(x=0;x<9;x++)
	{
		*ptr++ = I2C_Read_Byte(TCA5013_ADDRESS, reg_Address++);
	}
	if(NACK)
	{
		NACK = 0;

		return_Value = I2C_OPERATION_FAIL;
	}
	return return_Value;
}

unsigned char TCA5013_Read_SAM2(TCA5013_Register_Map* Regs)
{
	unsigned char return_Value = I2C_OPERATION_SUCCESSFUL;
	CLR_NACK;
	I2C_Read(7,TCA5013_ADDRESS, SAM2_STAT | 0x80, (unsigned char *) &Regs->Sam2_Registers);
	if(NACK)
	{
		NACK = 0;

		return_Value = I2C_OPERATION_FAIL;
	}
	return return_Value;
}


unsigned char TCA5013_Read_SAM2_noAI(TCA5013_Register_Map* Regs)
{
	unsigned char return_Value = I2C_OPERATION_SUCCESSFUL;
	unsigned char* ptr = (unsigned char*) &Regs->Sam2_Registers;
	unsigned char x = 0;
	unsigned char reg_Address = SAM2_STAT;
	for(x=0;x<7;x++)
	{
		*ptr++ = I2C_Read_Byte(TCA5013_ADDRESS, reg_Address++);
	}
	if(NACK)
	{
		NACK = 0;

		return_Value = I2C_OPERATION_FAIL;
	}
	return return_Value;
}

unsigned char TCA5013_Read_SAM3(TCA5013_Register_Map* Regs)
{
	unsigned char return_Value = I2C_OPERATION_SUCCESSFUL;
	CLR_NACK;
	I2C_Read(7,TCA5013_ADDRESS, SAM3_STAT | 0x80, (unsigned char *) &Regs->Sam3_Registers);
	if(NACK)
	{
		NACK = 0;

		return_Value = I2C_OPERATION_FAIL;
	}
	return return_Value;
}


unsigned char TCA5013_Read_SAM3_noAI(TCA5013_Register_Map* Regs)
{
	unsigned char return_Value = I2C_OPERATION_SUCCESSFUL;
	unsigned char* ptr = (unsigned char*) &Regs->Sam3_Registers;
	unsigned char x = 0;
	unsigned char reg_Address = SAM3_STAT;
	for(x=0;x<7;x++)
	{
		*ptr++ = I2C_Read_Byte(TCA5013_ADDRESS, reg_Address++);
	}
	if(NACK)
	{
		NACK = 0;

		return_Value = I2C_OPERATION_FAIL;
	}
	return return_Value;
}

unsigned char TCA5013_Read_INTSTAT(TCA5013_Register_Map* Regs)
{
	unsigned char return_Value = I2C_OPERATION_SUCCESSFUL;
	CLR_NACK;
	I2C_Read(7,TCA5013_ADDRESS, VERSION | 0x80, (unsigned char *) &Regs->Int_Mask_Registers);
	if(NACK)
	{
		NACK = 0;

		return_Value = I2C_OPERATION_FAIL;
	}
	return return_Value;
}


unsigned char TCA5013_Read_INTSTAT_noAI(TCA5013_Register_Map* Regs)
{
	unsigned char return_Value = I2C_OPERATION_SUCCESSFUL;
	unsigned char* ptr = (unsigned char*) &Regs->Int_Mask_Registers;
	unsigned char x = 0;
	unsigned char reg_Address = VERSION;
	for(x=0;x<7;x++)
	{
		*ptr++ = I2C_Read_Byte(TCA5013_ADDRESS, reg_Address++);
	}
	if(NACK)
	{
		NACK = 0;

		return_Value = I2C_OPERATION_FAIL;
	}
	return return_Value;
}

// ****************************************************************************
//! @fn          unsigned char TCA5013_read_Interrupt(TCA5013_Register_Map* Regs)
//! @brief		 Stores interrupt register value to MSP430 Memory map
//! @return		 Returns I2C success or failure
//!
// ****************************************************************************
unsigned char TCA5013_read_Interrupt(TCA5013_Register_Map* Regs)
{
	unsigned char return_Value = I2C_OPERATION_SUCCESSFUL;
	CLR_NACK;
	I2C_Read(1,TCA5013_ADDRESS, INT_STAT, (unsigned char*)&Regs->Int_Mask_Registers.Interrupt_status);
	if(NACK)
	{
		NACK = 0;
		return_Value = I2C_OPERATION_FAIL;
	}
	return return_Value;
}

/*
// ****************************************************************************
//! @fn          void TCA6408AWriteConfig(TCA6408ARegs* Regs)
//! @brief		 Writes config value from MSP430 memory map to TCA6408A
//! @return		 Returns I2C success or failure
//!
// ****************************************************************************
unsigned char TCA6408AWriteConfig(TCA6408ARegs * Regs)
{
	unsigned char return_Value = I2C_OPERATION_SUCCESSFUL;
	CLR_NACK;
	I2C_Write(1, TCA6408A_ADDRESS, TCA6408A_CONFIG_REG, (unsigned char*)&Regs->Config, 0);
	if(NACK)
	{
		NACK = 0;
		return_Value = I2C_OPERATION_FAIL;
	}
	return return_Value;
}

// ****************************************************************************
//! @fn          void TCA6408AWriteOutput(TCA6408ARegs* Regs)
//! @brief		 Writes output value from MSP430 memory map to TCA6408A
//! @return		 Returns I2C success or failure
//!
// ****************************************************************************
unsigned char TCA6408AWriteOutput(TCA6408ARegs * Regs)
{
	unsigned char return_Value = I2C_OPERATION_SUCCESSFUL;
	CLR_NACK;
	I2C_Write(1, TCA6408A_ADDRESS, TCA6408A_OUTPUT_REG, (unsigned char*)&Regs->Output, 0);
	if(NACK)
	{
		NACK = 0;
		return_Value = I2C_OPERATION_FAIL;
	}
	return return_Value;
}

// ****************************************************************************
//! @fn          void TCA6408AWritePolarity(TCA6408ARegs* Regs)
//! @brief		 Writes polarity inversion value from MSP430 memory map to TCA6408A
//! @return		 Returns I2C success or failure
//!
// ****************************************************************************
unsigned char TCA6408AWritePolarity(TCA6408ARegs * Regs)
{
	unsigned char return_Value = I2C_OPERATION_SUCCESSFUL;
	CLR_NACK;
	I2C_Write(1, TCA6408A_ADDRESS, TCA6408A_POLARITY_REG, (unsigned char*)&Regs->PolarityInversion, 0);
	if(NACK)
	{
		NACK = 0;
		return_Value = I2C_OPERATION_FAIL;
	}
	return return_Value;
}
*/


