#include "TCA5013.h"
unsigned char TCA5013_ADDRESS   =   0x39;


volatile TCA5013_Register_Map TCA5013_Regs;


int main(int argc, char *argv[])
{
	unsigned char i2c_retval;
    printf("Press enter to start TCA5013 testing...\r\n");
    getc(stdin);
	TCA5013_read_Interrupt((TCA5013_Register_Map*)&TCA5013_Regs);
	print_UART("Interrupt status register: 0x%x \r\n",TCA5013_Regs.Int_Mask_Registers.Interrupt_status);
	i2c_retval = TCA5013_Read_All((TCA5013_Register_Map*)&TCA5013_Regs);
	TCA5013_Write_All((TCA5013_Register_Map*)&TCA5013_Regs);
    printf("Press enter to stop TCA5013 testing...\r\n");
    getc(stdin);
}

