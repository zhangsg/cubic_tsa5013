#include "cubic_I2C.h"
#include <i2c.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

unsigned char NACK=0;
i2c_t *i2c;
const char *i2c_bus_path="/dev/i2c-4";
#define I2C_MAX_BYTE 100

int I2C_Init(void)
{
    int ret = 0;
    /* Allocate I2C */
    i2c = i2c_new();
    ret = i2c_open(i2c, i2c_bus_path);
    printf("i2c_open: 0x%x \r\n",ret);
	return ret;
}

void I2C_Deinit(void)
{
	i2c_close(i2c);
    /* Free I2C */
	if(i2c != NULL)
	{
		i2c_free(i2c);
	}
}


void I2C_Read (unsigned char byte_Count, unsigned char Slave_Address, unsigned char Register_Address, unsigned char* read_buffer)
{
    /* Read byte at address 0x100 of EEPROM */
    uint8_t msg_addr[1];
    uint8_t msg_data[1] = { 0xff, };

	//check para, don't supprot offset
	if(byte_Count>I2C_MAX_BYTE)
	{
		fprintf(stderr, "I2C_Write error byte_Count=0x%x\n",byte_Count);
		return;
	}

	memset(read_buffer,0,byte_Count);
	msg_addr[0] = Register_Address;
    struct i2c_msg msgs[2] =
        {
            /* Write 16-bit address */
            { .addr = Slave_Address, .flags = 0, .len = 1, .buf = msg_addr },
            /* Read 8-bit data */
            { .addr = Slave_Address, .flags = I2C_M_RD, .len = byte_Count, .buf = read_buffer},
        };


    /* Transfer a transaction with two I2C messages */
    if (i2c_transfer(i2c, msgs, 2) < 0) 
	{
        fprintf(stderr, "I2C_Read_Byte i2c_transfer(): %s\n", i2c_errmsg(i2c));
    }
}
unsigned int I2C_Read_Byte ( unsigned char Slave_Address, unsigned char Register_Address)
{
    /* Read byte at address 0x100 of EEPROM */
    uint8_t msg_addr[1];
    uint8_t msg_data[1] = { 0xff, };
	msg_addr[0] = Register_Address;
    struct i2c_msg msgs[2] =
        {
            /* Write 16-bit address */
            { .addr = Slave_Address, .flags = 0, .len = 1, .buf = msg_addr },
            /* Read 8-bit data */
            { .addr = Slave_Address, .flags = I2C_M_RD, .len = 1, .buf = msg_data},
        };


    /* Transfer a transaction with two I2C messages */
    if (i2c_transfer(i2c, msgs, 2) < 0) 
	{
        fprintf(stderr, "I2C_Read_Byte i2c_transfer(): %s\n", i2c_errmsg(i2c));
    }
	return msg_data[0];
}
void I2C_Write (unsigned char byte_Count, unsigned char Slave_Address, unsigned char Register_Address, unsigned char Register_Data[], unsigned char offset)
{
    uint8_t msg1[I2C_MAX_BYTE];
    struct i2c_msg msgs[1];
	int i;

	//check para, don't supprot offset
	if((byte_Count<1)||(byte_Count>I2C_MAX_BYTE))
	{
        fprintf(stderr, "I2C_Write error byte_Count=0x%x\n",byte_Count);
		return;
	}
	if(offset!=0)
	{
        fprintf(stderr, "I2C_Write error offset=0x%x\n",offset);
		return;
	}

	//init msg1
	memset(msg1,0,100);
	msg1[0] = Register_Address;
	for(i=0; i<byte_Count; i++)
		{
		msg1[1+i] = Register_Data[i];
		}

	//set i2c_transfer para
    msgs[0].addr = Slave_Address;
    msgs[0].flags = 0; /* Write */
    msgs[0].len = byte_Count+1; /*added one byte for address*/
    msgs[0].buf = msg1;
	if(i2c_transfer(i2c, msgs, 1) < 0)
	{
        fprintf(stderr, "I2C_Write_Byte i2c_transfer(): %s\n", i2c_errmsg(i2c));
	}
}
void I2C_Write_Byte (unsigned char Slave_Address, unsigned char Register_Address, unsigned char Register_Data)
{
    uint8_t msg1[2];
    struct i2c_msg msgs[1];
    /* S [ TCA5013_ADDRESS W ] [0xaa] [0xbb] [0xcc] [0xdd] */
	msg1[0] = Register_Address;
	msg1[1] = Register_Data;
	
    msgs[0].addr = Slave_Address;
    msgs[0].flags = 0; /* Write */
    msgs[0].len = sizeof(msg1);
    msgs[0].buf = msg1;
	if(i2c_transfer(i2c, msgs, 1) < 0)
		{
        fprintf(stderr, "I2C_Write_Byte i2c_transfer(): %s\n", i2c_errmsg(i2c));
		}
}


