 
#include 	"C30EVM_LCD.h"
#include 	<p30F4011.h>

//
// Defines for I/O ports that provide LCD data & control
// PORTD[0:3]-->DB[4:7]: Higher order 4 lines data bus with bidirectional
//					  : DB7 can be used as a BUSY flag
// PORTA,1 --> [E] : LCD operation start signal control 
// PORTA,2 --> [RW]: LCD Read/Write control
// PORTA,3 --> [RS]: LCD Register Select control
//		      	   : "0" for Instrunction register (Write), Busy Flag (Read)
//				   : "1" for data register (Read/Write)
//
#define CPU_SPEED		16					// CPU speed is 16 Mhz !!

#define LCD_RS			LATFbits.LATF0		// The definition of control pins
#define LCD_RW			LATFbits.LATF1
#define LCD_E			LATBbits.LATB7
#define LCD_E_MODE		ADPCFGbits.PCFG7

#define	DIR_LCD_RS		TRISFbits.TRISF0
#define	DIR_LCD_RW		TRISFbits.TRISF1
#define	DIR_LCD_E		TRISBbits.TRISB7

#define LCD_DATA		LATD				// PORTD[0:3] as LCD DB[4:7]
#define DIR_LCD_DATA	TRISD	


//  LCD Module commands
#define DISP_2Line_8Bit		0x0038
#define DISP_2Line_4Bit		0x0028
#define DISP_ON				0x00C		// Display on
#define DISP_ON_C			0x00E		// Display on, Cursor on
#define DISP_ON_B			0x00F		// Display on, Cursor on, Blink cursor
#define DISP_OFF			0x008		// Display off
#define CLR_DISP			0x001		// Clear the Display
#define ENTRY_INC			0x006		//
#define ENTRY_INC_S			0x007		//
#define ENTRY_DEC			0x004		//
#define ENTRY_DEC_S			0x005		//
#define DD_RAM_ADDR			0x080		// Least Significant 7-bit are for address
#define DD_RAM_UL			0x080		// Upper Left coner of the Display	
		
unsigned char 	Temp_CMD ;
unsigned char 	Str_Temp ;
unsigned char	Out_Mask ;	
int				Temp_LCD_DATA ;	


void OpenLCD(void)

{		
	Temp_LCD_DATA = LCD_DATA ;				// Save the Port Value of LCD_DATA
	
	LCD_E_MODE =1 ;
	LCD_E = 0 ;
	LCD_DATA &= 0xfff0;						// LCD DB[4:7] & RS & R/W --> Low
	DIR_LCD_DATA &= 0xfff0;					// LCD DB[4:7} & RS & R/W are output function
	DIR_LCD_E = 0;							// Set E pin as output
	DIR_LCD_RS = 0 ;
	DIR_LCD_RW = 0 ;

	LCD_DATA &= 0xfff0 ;
	LCD_DATA |= 0x0003 ;
	LCD_CMD_W_Timing() ;
	LCD_L_Delay() ;

	LCD_DATA &= 0xfff0 ;
	LCD_DATA |= 0x0003 ;
	LCD_CMD_W_Timing() ;
	LCD_L_Delay() ;

	LCD_DATA &= 0xfff0 ;
	LCD_DATA |= 0x0003 ;
	LCD_CMD_W_Timing() ;
	LCD_L_Delay() ;

	LCD_DATA &= 0xfff0 ;
	LCD_DATA |= 0x0002 ;
	LCD_CMD_W_Timing() ;
	LCD_L_Delay() ;

	WriteCmdLCD(DISP_2Line_4Bit) ;
	LCD_S_Delay() ;

	WriteCmdLCD(DISP_ON) ;
	LCD_S_Delay() ;

	WriteCmdLCD(ENTRY_INC) ;
	LCD_S_Delay() ;

	WriteCmdLCD(CLR_DISP) ;
	LCD_L_Delay() ;

	LCD_DATA = Temp_LCD_DATA ;			
}

//*********************************************
//     _    ______________________________
// RS  _>--<______________________________
//     _____
// RW       \_____________________________
//                  __________________
// E   ____________/                  \___
//     _____________                ______
// DB  _____________>--------------<______
//***********************************************
// Write Command to LCD module
//
void WriteCmdLCD( unsigned char LCD_CMD) 
{

	Temp_LCD_DATA = LCD_DATA ;	

	Temp_CMD = (LCD_CMD & 0xF0)>>4 ;			// Send high nibble to LCD bus
	LCD_DATA= (LCD_DATA & 0xfff0)|Temp_CMD ;
	LCD_CMD_W_Timing () ;

	Temp_CMD = LCD_CMD & 0x0F ;				// Send low nibble to LCD bus
	LCD_DATA= (LCD_DATA & 0xfff0)|Temp_CMD ;
	LCD_CMD_W_Timing () ;
	LCD_DATA = Temp_LCD_DATA ;
	LCD_S_Delay() ;							// Delay 100uS for execution

}

//***********************************************
// Write Data to LCD module
//
void WriteDataLCD( unsigned char LCD_CMD) 
{
	
	Temp_LCD_DATA = LCD_DATA ;

	Temp_CMD = (LCD_CMD & 0xF0)>>4 ;			// Send high nibble to LCD bus
	LCD_DATA= (LCD_DATA & 0xfff0)|Temp_CMD ;
	LCD_DAT_W_Timing () ;

	Temp_CMD = LCD_CMD & 0x0F ;					// Send low nibble to LCD bus
	LCD_DATA= (LCD_DATA & 0xfff0)|Temp_CMD ;
	LCD_DAT_W_Timing () ;
	LCD_DATA = Temp_LCD_DATA ;
	LCD_S_Delay() ;								// Delay 100uS for execution


}

void putcLCD(unsigned char LCD_Char)
{
	WriteDataLCD(LCD_Char) ;

}
void LCD_CMD_W_Timing( void )
{
	LCD_RS = 0 ;	// Set for Command Input
	Nop();
	LCD_RW = 0 ;
	Nop();
	LCD_E = 1 ;
	Nop();
	Nop();
	Nop();
	Nop();
	LCD_E = 0 ;
}

void LCD_DAT_W_Timing( void )
{
	LCD_RS = 1 ;	// Set for Data Input
	Nop( );
	LCD_RW = 0 ;
	Nop( );
	LCD_E = 1 ;
	Nop( );
	Nop( );
	Nop( );
	Nop( );
	LCD_E = 0 ;
}

//***********************************************
//     Set Cursor position on LCD module
//			CurY = Line (0 or 1)
//      	CurX = Position ( 0 to 15)
//
void setcurLCD(unsigned char CurX, unsigned char CurY)
{
	WriteCmdLCD( 0x80 + CurY * 0x40 + CurX) ;
	LCD_S_Delay() ;
}

//***********************************************
//    Put a ROM string to LCD Module
//
void putrsLCD( const char *Str )
{
   while (1)
   {
	Str_Temp = *Str ;

		if (Str_Temp != 0x00 )
		   {
			WriteDataLCD(Str_Temp) ;
			Str ++ ;
		   }
		else
			return ;
   }
}

//***********************************************
//    Put a RAM string to LCD Module
//
void putsLCD( char *Str)
{
   while (1)
   {
	Str_Temp = *Str ;

		if (Str_Temp != 0x00 )
		   {
			WriteDataLCD(Str_Temp) ;
			Str ++ ;
		   }
		else
			return ;
   }
}


void puthexLCD(unsigned char HEX_Val)
{
	unsigned char Temp_HEX ;

	Temp_HEX = (HEX_Val >> 4) & 0x0f ;

	if ( Temp_HEX > 9 )Temp_HEX += 0x37 ;
    else Temp_HEX += 0x30 ;

	WriteDataLCD(Temp_HEX) ;

	Temp_HEX = HEX_Val  & 0x0f ;
	if ( Temp_HEX > 9 )Temp_HEX += 0x37 ;
    else Temp_HEX += 0x30 ;

	WriteDataLCD(Temp_HEX) ;
}

// *********************************************************************************
// Delay for atleast 10 ms 
// *********************************************************************************
void LCD_L_Delay(void)
{
	int		L_Loop ;

	for 	( L_Loop = 0 ; L_Loop < 100 ; L_Loop ++ )
				LCD_S_Delay( ) ;		
}

// *********************************************************************************
// Delay for 100 us
// *********************************************************************************
void LCD_S_Delay(void)
{
	int		S_Loop ;
	int		Null_Var ;

	for  	( S_Loop = 0 ; S_Loop < 200 ; S_Loop ++ )
				Null_Var += 1 ;
		
}
