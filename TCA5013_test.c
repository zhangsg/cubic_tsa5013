#include "TCA5013.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <errno.h>

#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <i2c.h>


unsigned char TCA5013_ADDRESS   =   0x39;


volatile TCA5013_Register_Map TCA5013_Regs;

int read_input(void) 
{
	int i;
	i=0;
	char a[100];
	char ch;
	for(ch;ch = getchar();ch != EOF)
	{
		if(ch != '\n')
		{
			a[i++]= ch;
		}
		else
		{
			break;
		}
	}
	i= atoi(a);//atoi()将字符串转换为数字，在stdlib中 
	
	
	printf("%d",i);
	return i;

}

int main(int argc, char *argv[])
{
	int input;
	unsigned char write_add;
	unsigned char write_data;
	unsigned char read_add;
	unsigned char read_data;
	int i;

	I2C_Init();
    printf("input address that you want to WRITE...\r\n");
	write_add = read_input();
    printf("input data that you want to WRITE...\r\n");
	write_data = read_input();
    printf("get write_add = 0x%x, write_data = 0x%x\r\n",write_add, write_data);
	I2C_Write_Byte(TCA5013_ADDRESS, write_add, write_data);
	
    printf("input address that you want to READ...\r\n");
	read_add = read_input();
	read_data = I2C_Read_Byte(TCA5013_ADDRESS, read_add);
    printf("TCA5013 I2C_Read_Byte = 0x%x\r\n",read_data);

	unsigned char write_data_array[7];
	unsigned char read_data_array[7];
	write_data_array[0] = 1;
	write_data_array[1] = 22;
	write_data_array[2] = 3;
	write_data_array[3] = 44;
	write_data_array[4] = 5;
	write_data_array[5] = 66;
	write_data_array[6] = 7;
	I2C_Write(7, TCA5013_ADDRESS, VERSION | 0x80, write_data_array, 0);
	I2C_Read(7, TCA5013_ADDRESS, SAM1_STAT | 0x80, read_data_array);
	printf("TCA5013 I2C_Read Register_Address= 0x%x, byte_Count\r\n",Register_Address,byte_Count);
    for(i=0;i<7;i++)
    	{
    	printf("0x%x,\t",read_data_array[i]);
    	}

	
	TCA5013InitDefault((TCA5013_Register_Map*)&TCA5013_Regs);
	
    printf("Press enter to stop TCA5013 testing...\r\n");
    getc(stdin);
	I2C_Deinit();
}

