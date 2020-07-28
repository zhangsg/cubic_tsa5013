

/*******************************************************************************
*                            Macro Definitions                                 *
*******************************************************************************/

/**************Macro used read or change TCA8418EReg by byte*************/
#define TCA5013_REG_VALUE *(unsigned char*)&

/*******************************************************************************
*                                 Definitions                                  *
*******************************************************************************/

/***************************TCA8418E Register Addresses************************/
#define USER_CARD_STAT			0x00
#define USER_CARD_SET			0x01
#define USER_CLK_SET			0x02
#define USER_ASYNC_EARLY_MSB 	0x03
#define USER_ASYNC_EARLY_LSB	0x04
#define USER_ASYNC_MUTE_MSB 	0x05
#define USER_ASYNC_MUTE_LSB		0x06
#define USER_CARD_IO_SLEW		0x07
#define USER_CARD_CLK_SLEW		0x08
#define SYNC_SET				0x09
#define ATR_BYTE_1				0x0A
#define ATR_BYTE_2				0x0B
#define ATR_BYTE_3				0x0C
#define ATR_BYTE_4				0x0D

#define SAM1_STAT 				0x10
#define SAM1_SET 				0x11
#define SAM1_CLK			 	0x12
#define SAM1_ASYNC_EARLY_MSB 	0x13
#define SAM1_ASYNC_EARLY_LSB	0x14
#define SAM1_ASYNC_MUTE_MSB 	0x15
#define SAM1_ASYNC_MUTE_LSB		0x16
//#define SAM1_CARD_IO_SLEW		0x17
//#define SAM1_CARD_CLK_SLEW		0x18

#define SAM2_STAT 				0x20
#define SAM2_SET 				0x21
#define SAM2_CLK			 	0x22
#define SAM2_ASYNC_EARLY_MSB 	0x23
#define SAM2_ASYNC_EARLY_LSB	0x24
#define SAM2_ASYNC_MUTE_MSB 	0x25
#define SAM2_ASYNC_MUTE_LSB		0x26
//#define SAM2_CARD_IO_SLEW		0x27
//#define SAM2_CARD_CLK_SLEW		0x28

#define SAM3_STAT 				0x30
#define SAM3_SET 				0x31
#define SAM3_CLK			 	0x32
#define SAM3_ASYNC_EARLY_MSB 	0x33
#define SAM3_ASYNC_EARLY_LSB	0x34
#define SAM3_ASYNC_MUTE_MSB 	0x35
#define SAM3_ASYNC_MUTE_LSB		0x36
//#define SAM3_CARD_IO_SLEW		0x37
//#define SAM3_CARD_CLK_SLEW		0x38

#define VERSION					0x40
#define INT_STAT				0x41
#define DEVICE_SETTINGS			0x42
#define GPIO_SET				0x43
#define INT_MASK_USER			0x44
#define INT_MASK_SAM1_2			0x45
#define INT_MASK_SAM3_GPIO		0x46



#define IO_Data_Received 	0x01
#define TCA5013_Interrupt 	0x02

/*******************************************************************************
*                            ENUM Definitions                                  *
*******************************************************************************/

enum voltage_Leves{
	V_1pt8 = 0x01,
	V_3pt3 = 0x02,
	V_5 = 0x03
};

enum clk_div{
	div_1 = 0x00,
	div_2 = 0x01,
	div_4 = 0x02,
	div_5 = 0x03,
	div_8 = 0x04
};

/*******************************************************************************
*                            Struct Definitions                                *
*******************************************************************************/
struct sUSER_STAT{
	unsigned char RESERVED:1;
	unsigned char PRES:1;
	unsigned char PRESL:1;
	unsigned char CLKSW:1;
	unsigned char PROT:1;
	unsigned char MUTE:1;
	unsigned char EARLY:1;
	unsigned char ACTIVE:1;

};

struct sUser_Set{
	unsigned char Start_Async:1;
	unsigned char Reserved:1;
	unsigned char Card_detect:1;
	unsigned char Warm:1;
	unsigned char RESERVED:1;
	unsigned char IOEN:1;
	unsigned char Set_VCC:2;
};

struct sUser_CLK{
	unsigned char Reserved:2;
	unsigned char ClockDiv:3;
	unsigned char Clk1:2;
	unsigned char Internal_Clk:1;
};


struct sASYNCH_EARLY_MSB{
	unsigned char all:8;
};

struct sASYNCH_EARLY_LSB{
	unsigned char reserved:6;
	unsigned char all:2;
};

struct sAsync_Early{
	struct sASYNCH_EARLY_MSB MSB;
	struct sASYNCH_EARLY_LSB LSB;
};

struct sASYNCH_MUTE_MSB{
	unsigned char MuteCountMSB:8;
};

struct sASYNCH_MUTE_LSB{
	unsigned char MuteCountLSB:8;
};

struct sAsync_Mute{
	struct sASYNCH_EARLY_MSB MSB;
	struct sASYNCH_EARLY_MSB LSB;
};

struct sIO_Slew{
	unsigned char Reserved: 3;
	unsigned char Fall_Time: 2;
	unsigned char Rise_Time: 3;
};

struct sCLK_slew{
	unsigned char Reserved:4;
	unsigned char Rise_Fall_Time:4;
};

struct sSync_Set{
	unsigned char Start_Sync:1;
	unsigned char Edge:1;
	//CLK_Disable was changed to CLK_enable during development
	//1 in CLK_Disable will enable the sync CLK
	unsigned char CLK_Disable:1;
	unsigned char RST:1;
	unsigned char C8:1;
	unsigned char C4:1;
	unsigned char Activation_type:1;
	unsigned char Card_type:1;
};

struct sSam_STAT{
	unsigned char Reserved:3;
	unsigned char CLCKSW:1;
	unsigned char PROT:1;
	unsigned char MUTE:1;
	unsigned char EARLY:1;
	unsigned char ACTIVE:1;
};

struct sSam_SET{
	unsigned char Start_async:1;
	unsigned char Reserved:2;
	unsigned char Warm:1;
	unsigned char Reserved2:1;
	unsigned char IO_EN:1;
	unsigned char Set_VCC:2;
};

struct sSam_CLK{
	unsigned char Reserved:2;
	unsigned char Clock_Divide:3;
	unsigned char Clk1:2;
	unsigned char Intern_clk:1;
};

struct sProd_vers
{
	unsigned char all:8;
};

struct sInt_Stat{
	unsigned char GPIO:1;
	unsigned char Sync_Complete:1;
	unsigned char Supervisor:1;
	unsigned char OTP:1;
	unsigned char Sam3:1;
	unsigned char Sam2:1;
	unsigned char Sam1:1;
	unsigned char UserCard:1;
};

struct sGPIO_CFG{
	unsigned char Reserved:2;
	unsigned char GPIO1:1;
	unsigned char GPIO2:1;
	unsigned char GPIO3:1;
	unsigned char GPIO4:1;
	unsigned char Reserved2:2;
};

struct sGPIO_Set{
	unsigned char GPIO1_OUT:1;
	unsigned char GPIO2_OUT:1;
	unsigned char GPIO3_OUT:1;
	unsigned char GPIO4_OUT:1;
	unsigned char GPIO1_IN:1;
	unsigned char GPIO2_IN:1;
	unsigned char GPIO3_IN:1;
	unsigned char GPIO4_IN:1;
};

struct sInt_Mask{

	unsigned char Reserved:1;
	unsigned char GPIO:1;
	unsigned char Supervisor:1;
	unsigned char OTP:1;
	unsigned char Sync_Complete:1;
	unsigned char Prot_UserCard:1;
	unsigned char Mute_UserCard:1;
	unsigned char Early_UserCard:1;
};

struct sMask_Sam1_2{
	unsigned char Reserved:2;
	unsigned char Prot_Sam2:1;
	unsigned char Early_Sam2:1;
	unsigned char Prot_Sam1:1;
	unsigned char Mute_Sam1:1;
	unsigned char Early_Sam1:1;
	unsigned char Mute_Sam2:1;
};

struct sMask_GPIO{
	unsigned char Reserved:1;
	unsigned char GPIO1:1;
	unsigned char GPIO2:1;
	unsigned char GPIO3:1;
	unsigned char GPIO4:1;
	unsigned char Prot_Sam3:1;
	unsigned char Mute_Sam3:1;
	unsigned char Early_Sam3:1;
};


struct sUC_CARD{
	struct sUSER_STAT		Status;
	struct sUser_Set		Settings;
	struct sUser_CLK		Clock_Settings;
	struct sAsync_Early		Early;
	struct sAsync_Mute		Mute;
	struct sIO_Slew			IO_Slew;
	struct sCLK_slew		Clk_Slew;
	struct sSync_Set		Synchronous_Settings;
	struct sProd_vers		ATR_Byte1;
	struct sProd_vers		ATR_Byte2;
	struct sProd_vers		ATR_Byte3;
	struct sProd_vers		ATR_Byte4;
};

struct sSAM1{
	struct sSam_STAT		Status;
	struct sSam_SET			Settings;
	struct sSam_CLK			Clock_Settings;
	struct sAsync_Early		Early;
	struct sAsync_Early		Mute;
	struct sIO_Slew			IO_Slew;
	struct sCLK_slew		Clk_Slew;
};

struct sSAM2{
	struct sSam_STAT		Status;
	struct sSam_SET			Settings;
	struct sSam_CLK			Clock_Settings;
	struct sAsync_Early		Early;
	struct sAsync_Early		Mute;
};

struct sSAM3{
	struct sSam_STAT		Status;
	struct sSam_SET			Settings;
	struct sSam_CLK			Clock_Settings;
	struct sAsync_Early		Early;
	struct sAsync_Early		Mute;
};

struct sFinal{
	struct sProd_vers 		Product_Version;
	struct sInt_Stat		Interrupt_status;
	struct sGPIO_CFG		Device_Settings;
	struct sGPIO_Set		GPIO_Settings;
	struct sInt_Mask		UC_Int_Mask;
	struct sMask_Sam1_2		Sam1_2_Int_Mask;
	struct sMask_GPIO		Sam3_GPIO_Int_Mask;
};

typedef struct{
	struct sUC_CARD			User_Card_Registers;
	struct sSAM1			Sam1_Registers;
	struct sSAM2			Sam2_Registers;
	struct sSAM3			Sam3_Registers;
	struct sFinal			Int_Mask_Registers;

}TCA5013_Register_Map;


void TCA5013InitDefault(TCA5013_Register_Map* Regs);
unsigned char TCA5013_Read_All(TCA5013_Register_Map* Regs);

unsigned char TCA5013_Write_All(TCA5013_Register_Map* Regs);
unsigned char TCA5013_Read_All(TCA5013_Register_Map* Regs);
unsigned char TCA5013_Read_All_noAI(TCA5013_Register_Map* Regs);
unsigned char TCA5013_Read_UC(TCA5013_Register_Map* Regs);
unsigned char TCA5013_Read_UC_noAI(TCA5013_Register_Map* Regs);
unsigned char TCA5013_Read_SAM1(TCA5013_Register_Map* Regs);
unsigned char TCA5013_Read_SAM1_noAI(TCA5013_Register_Map* Regs);
unsigned char TCA5013_Read_SAM2(TCA5013_Register_Map* Regs);
unsigned char TCA5013_Read_SAM2_noAI(TCA5013_Register_Map* Regs);
unsigned char TCA5013_Read_SAM3(TCA5013_Register_Map* Regs);
unsigned char TCA5013_Read_SAM3_noAI(TCA5013_Register_Map* Regs);
unsigned char TCA5013_Read_INTSTAT(TCA5013_Register_Map* Regs);
unsigned char TCA5013_Read_INTSTAT_noAI(TCA5013_Register_Map* Regs);
void TCA5013_To_value(TCA5013_Register_Map* Regs, unsigned char value);
unsigned char TCA5013_read_Interrupt(TCA5013_Register_Map* Regs);
