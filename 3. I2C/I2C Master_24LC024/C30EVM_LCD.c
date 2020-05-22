 
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
#define LCD_E_MODE		ADPCFGbits.PCFG7	// Set RB7 as digital I/O

#define	DIR_LCD_RS		TRISFbits.TRISF0	// Direction of control pins
#define	DIR_LCD_RW		TRISFbits.TRISF1
#define	DIR_LCD_E		TRISBbits.TRISB7

#define LCD_DATA		LATD				// PORTD[0:3] as LCD DB[4:7]
#define DIR_LCD_DATA	TRISD				// Direction of Databus


//  LCD Module commands --- These settings can be found in the LCD datasheet
#define DISP_2Line_8Bit		0x0038		// 2 lines & 8 bits setting
#define DISP_2Line_4Bit		0x0028		// 2 lines & 4 bits setting
#define DISP_ON				0x00C		// Display on
#define DISP_ON_C			0x00E		// Display on, Cursor on
#define DISP_ON_B			0x00F		// Display on, Cursor on, Blink cursor
#define DISP_OFF			0x008		// Display off
#define CLR_DISP			0x001		// Clear the Display
#define ENTRY_INC			0x006		// Entry Increment and Cursor Move
#define ENTRY_INC_S			0x007		// Entry Increment and Display Shift
#define ENTRY_DEC			0x004		// Entry Decrement and Cursor Move
#define ENTRY_DEC_S			0x005		// Entry Decrement and Display Shift
#define DD_RAM_ADDR			0x080		// Least Significant 7-bit are for address
#define DD_RAM_UL			0x080		// Upper Left coner of the Display	
		
unsigned char 	Temp_CMD ;				// Temperary Buffers for Command,
unsigned char 	Str_Temp ;				// for String,
int				Temp_LCD_DATA ;			// for PORT data (This will be restored after Communication w/ LCD)
//unsigned char	Out_Mask ;				//


void OpenLCD(void)

{		
	Temp_LCD_DATA = LCD_DATA ;				// Save the Port Value of LCD_DATA
	
	LCD_E_MODE =1 ;							// Initialize RB7 as digital I/O
	LCD_E = 0 ;								// 
	LCD_DATA &= 0xfff0;						// LCD DB[4:7] & RS & R/W --> Low
	DIR_LCD_DATA &= 0xfff0;					// LCD DB[4:7} & RS & R/W are output function
	DIR_LCD_E = 0;							// Set E pin as output
	DIR_LCD_RS = 0 ;
	DIR_LCD_RW = 0 ;

// Initialize the LCD following the standard operations
											// 1st
	LCD_DATA &= 0xfff0 ;					// Clear PORTDbits.RD0 ~ RD3 but save others by &
	LCD_DATA |= 0x0003 ;					// Send Data of 0x03 but keep others by |
	LCD_CMD_W_Timing() ;					// LCD Command Write Sequence Function
	LCD_L_Delay() ;							// Delay for long enough time (4.1ms minimum)
											// 2nd
	LCD_DATA &= 0xfff0 ;					// Clear PORTDbits.RD0 ~ RD3
	LCD_DATA |= 0x0003 ;					// Send Data of 0x03
	LCD_CMD_W_Timing() ;					// LCD Command Write Sequence Function
	LCD_L_Delay() ;							// Delay for long enough time (100us minimum)
											// 3rd
	LCD_DATA &= 0xfff0 ;					// Clear PORTDbits.RD0 ~ RD3
	LCD_DATA |= 0x0003 ;					// Send Data of 0x03
	LCD_CMD_W_Timing() ;					// LCD Command Write Sequence Function
	LCD_L_Delay() ;							// Delay for long enough time (Not Required)

	LCD_DATA &= 0xfff0 ;					// Clear PORTDbits.RD0 ~ RD3
	LCD_DATA |= 0x0002 ;					// Send Data of 0x02 for 4-bit databus interface
	LCD_CMD_W_Timing() ;
	LCD_L_Delay() ;

	WriteCmdLCD(DISP_2Line_4Bit) ;			// Configure LCD - 2 lines display & 4-bit long bus
	LCD_S_Delay() ;

	WriteCmdLCD(DISP_ON) ;					// Configure LCD - Turn on display
	LCD_S_Delay() ;

	WriteCmdLCD(ENTRY_INC) ;				// Configure LCD - Entry increment (to the right)
	LCD_S_Delay() ;

	WriteCmdLCD(CLR_DISP) ;					// Configure LCD - Clear Display
	LCD_L_Delay() ;

	LCD_DATA = Temp_LCD_DATA ;				// Restore Port Data (Useful if Port is shared, such as w/ LED)
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
// Subroutine to 
// Write Command to LCD module
//
void WriteCmdLCD( unsigned char LCD_CMD) 
{

	Temp_LCD_DATA = LCD_DATA ;					// Save Port data to Temp buffer

	Temp_CMD = (LCD_CMD & 0xF0)>>4 ;			// Send high nibble to LCD bus
	LCD_DATA= (LCD_DATA & 0xfff0)|Temp_CMD ;	
	LCD_CMD_W_Timing () ;

	Temp_CMD = LCD_CMD & 0x0F ;					// Send low nibble to LCD bus
	LCD_DATA= (LCD_DATA & 0xfff0)|Temp_CMD ;
	LCD_CMD_W_Timing () ;

	LCD_DATA = Temp_LCD_DATA ;					// Restore Port data
	LCD_S_Delay() ;								// Delay 100uS for execution

}

//***********************************************
// Subroutine to 
// Write Data to LCD module
//
void WriteDataLCD( unsigned char LCD_CMD) 
{
	
	Temp_LCD_DATA = LCD_DATA ;					// Save Port data to Temp buffer

	Temp_CMD = (LCD_CMD & 0xF0)>>4 ;			// Send high nibble to LCD bus
	LCD_DATA= (LCD_DATA & 0xfff0)|Temp_CMD ;
	LCD_DAT_W_Timing () ;

	Temp_CMD = LCD_CMD & 0x0F ;					// Send low nibble to LCD bus
	LCD_DATA= (LCD_DATA & 0xfff0)|Temp_CMD ;
	LCD_DAT_W_Timing () ;

	LCD_DATA = Temp_LCD_DATA ;					// Restore Port data
	LCD_S_Delay() ;								// Delay 100uS for execution


}

//***********************************************
// Subroutine to 
// Write a character to LCD module
//
void putcLCD(unsigned char LCD_Char)
{
	WriteDataLCD(LCD_Char) ;

}

//***********************************************
// Subroutine to 
// Write a command to LCD module
// RS=0, R/W=0, E=H->L F(alling Edge)

void LCD_CMD_W_Timing( void )					// LCD Command writing timming
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

//***********************************************
// Subroutine to 
// Write a command to LCD module
// RS=1, R/W=0, E=H->L F(alling Edge)

void LCD_DAT_W_Timing( void )					// LCD Data writing timming
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

//***********************************************
//    Put a byte in Hex format to LCD Module
//

void puthexLCD(unsigned char HEX_Val)
{
	unsigned char Temp_HEX ;

	Temp_HEX = (HEX_Val >> 4) & 0x0f ;		// high nibble

	if ( Temp_HEX > 9 )Temp_HEX += 0x37 ;	// A~F
    else Temp_HEX += 0x30 ;					// 0~9

	WriteDataLCD(Temp_HEX) ;

	Temp_HEX = HEX_Val  & 0x0f ;			// low nibble
	if ( Temp_HEX > 9 )Temp_HEX += 0x37 ;
    else Temp_HEX += 0x30 ;

	WriteDataLCD(Temp_HEX) ;
}

//---------------------------------------------------
//    Put a integer in decimal format to LCD Module
//	  0~9999 Only. 
//---------------------------------------------------
void	put_int_LCD( unsigned int The_Number )
{

	unsigned char	Temp_Char_1000 , Temp_Char_100, Temp_Char_10, Temp_Char ;
		
		Temp_Char_1000 = The_Number / 1000 ;
		putcLCD(Temp_Char_1000+'0') ;	
		Temp_Char_100 = (The_Number - Temp_Char_1000*1000) / 100 ;	//κ计计
		putcLCD( Temp_Char_100 + '0' ) ;							//锣传ASCII絪絏'0'絪絏计
		Temp_Char_10 = (The_Number - (Temp_Char_1000*1000 +Temp_Char_100*100))  /10 ;		//计计
		putcLCD( Temp_Char_10 + '0' ) ;
		Temp_Char = The_Number - (Temp_Char_1000*1000+Temp_Char_100*100 + Temp_Char_10 * 10 ) ;	//计计
		putcLCD( Temp_Char + '0' ) ;
}

//***********************************************
//    Put a byte in decimal format to LCD Module
//	  0~99 Only. Uncomment the first 2 lines for hundreds

void	put_Num_LCD( unsigned char The_Number )
{

	unsigned char	Temp_Char_100, Temp_Char_10, Temp_Char ;

		Temp_Char_100 = The_Number /100 ;			//κ计计
		putcLCD( Temp_Char_100 + '0' ) ;			//锣传ASCII絪絏'0'絪絏计
		Temp_Char_10 = (The_Number - Temp_Char_100*100)  /10 ;		//计计
		putcLCD( Temp_Char_10 + '0' ) ;
		Temp_Char = The_Number - ( Temp_Char_100*100 + Temp_Char_10 * 10 ) ;	//计计
		putcLCD( Temp_Char + '0' ) ;
}

// **********************************************
// Delay for atleast 10 ms 
// **********************************************
void LCD_L_Delay(void)
{
	int		L_Loop ;

	for 	( L_Loop = 0 ; L_Loop < 100 ; L_Loop ++ )
				LCD_S_Delay( ) ;		
}

// ***********************************************
// Delay for 100 us
// ***********************************************
void LCD_S_Delay(void)
{
	int		S_Loop ;
	int		Null_Var ;

	for  	( S_Loop = 0 ; S_Loop < 200 ; S_Loop ++ )
				Null_Var += 1 ;
		
}
