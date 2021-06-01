
/*******************************************************************************
*                                 Prototypes                                   *
*******************************************************************************/

/*******************************************************************************
*                                 Definitions                                  *
*******************************************************************************/
enum I2C_State{
	I2C_OPERATION_SUCCESSFUL =0,
	I2C_OPERATION_FAIL = 1

};
//jiang ning temp

enum I2C_Bus_Speed{
	Standard_mode = 160,
	Fast_mode = 40,
	Fast_mode_plus = 16
};



/************ I2C Interface *******************************/
void I2C_Read (unsigned char byte_Count, unsigned char Slave_Address, unsigned char Register_Address, unsigned char* read_buffer);
unsigned int I2C_Read_Byte ( unsigned char Slave_Address, unsigned char Register_Address);
void I2C_Write (unsigned char byte_Count, unsigned char Slave_Address, unsigned char Register_Address, unsigned char Register_Data[], unsigned char offset);
void I2C_Write_Byte (unsigned char Slave_Address, unsigned char Register_Address, unsigned char Register_Data);



