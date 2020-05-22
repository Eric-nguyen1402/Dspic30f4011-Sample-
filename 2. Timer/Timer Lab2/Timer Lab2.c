// ***********************************************************************
//
// Purpose : 練習如何使用 Timer1 模組輸入計算出輸入信號的 High Duty
//           並將所測得到的 Duty Cycle  每隔 100mS 顯示於 LCD 上
//
// ***********************************************************************

#include 	<p30fxxxx.h>
#include 	"C30EVM_LCD.h"			 
#include	<timer.h>

#define		Fcy 	7372800 * 2		// 因為使用頻率為將外部 7.3728 MHz * 8 的模式 

   	_FOSC(CSW_FSCM_OFF & XT_PLL8);   //	XT with 8xPLL oscillator, Failsafe clock off
   	_FWDT(WDT_OFF);                  //	Watchdog timer disabled
  	_FBORPOR(PBOR_OFF & MCLR_EN);    //	Brown-out reset disabled, MCLR reset enabled
   	_FGS(CODE_PROT_OFF);             //	Code protect disabled

const char	My_String1[]="Timer1 Gate Lab2" ;	// 宣告字串於 Program Memory
const  char	My_String2[]="T1CK Duty     uS" ;	 

#define	LED1		LATEbits.LATE0 		//定義LED為特定輸出埠腳位位元的替代符號，
#define	LED2		LATEbits.LATE1 		//以加強程式的可讀性與簡潔
#define	LED3		LATEbits.LATE2
#define	LED4		LATEbits.LATE3
#define	LED5		LATEbits.LATE4
#define	LED6		LATEbits.LATE5

#define	DIR_LED1	TRISebits.TRISE0 	//定義DIR_LED為特定輸出入埠腳位位元
#define	DIR_LED2	TRISebits.TRISE1	//的方向控制位元替代符號
#define	DIR_LED3	TRISebits.TRISE2
#define	DIR_LED4	TRISebits.TRISE3
#define	DIR_LED5	TRISebits.TRISE4
#define	DIR_LED6	TRISebits.TRISE5

#define	SW5			PORTEbits.RE8		//定義SW5與SW6為特定輸入埠腳位位元的替代符號
#define	SW6			PORTBbits.RB3
#define	DIR_SW5		TRISEbits.TRISE8 	//定義DIR_LED7與DIR_LED8為特定輸出入埠腳位位元
#define	DIR_SW6		TRISBbits.TRISB3	//的方向控制位元替代符號

void Init_IO(void);
void Init_Timer1(void);
void Init_Timer2(void);
void update_LCD(void);
void uitoa( unsigned char * , unsigned int );

unsigned char	Int_flag;
unsigned int 	Period;
unsigned char	Convert_Buf[10] ;
unsigned char	Delay_Count=0;

void __attribute__((interrupt, no_auto_psv)) _T1Interrupt(void)		// 輸入捕捉中斷副程式
{	
	Period= ReadTimer1( );
	WriteTimer1(0);
	Int_flag = 1;					// 設定軟體中斷旗標
	IFS0bits.T1IF = 0 ;					// Clear Timer1 Interrupt Flag
}

int main(void)
{

	Init_IO( );
	Init_Timer1( );
	Init_Timer2( );

	OpenLCD( );							// 使用 OpenLCD( )對 LCD 模組作初始化設定
										// 4 bits Data mode
										// 5 * 7 Character 

	setcurLCD(0,0) ;					// 使用 setcurLCD( ) 設定游標於 (0,0)
	putrsLCD( My_String1 ) ;			// 將存在 Program Memory 的字串使用
										// putrsLCD( ) 印出至 LCD
			
	setcurLCD(0,1) ;
	putrsLCD( My_String2 ) ;			// 使用 setcurLCD( ) 設定游標於 (0,1)
										// 將存在 Data Memory 的字串使用
	
 	WriteTimer1(0);						// 將Timer1 歸零，準備脈波寬度量測
	Int_flag = 0;

	while(1)
	{
		if (Int_flag)
		{
			Int_flag = 0;

			if (IFS0bits.T2IF) 					// 100mS?
			{
				IFS0bits.T2IF=0;
				DisableIntT1;					// Disable Interrupt of Timer1
				Period = Period / 14.745600 ;	// Calculate the period
				update_LCD( );					// Update Period to LCD
				WriteTimer1(0);
				EnableIntT1;					// Enable Interrupt of Timer1
			}
		}
 	}
}

void update_LCD(void)
{
		uitoa ( Convert_Buf , Period ) ;
		setcurLCD(10,1) ;
		putrsLCD( "    " ) ;
		setcurLCD(10,1) ;
		putsLCD( Convert_Buf ) ;
}

void	uitoa( unsigned char *ASCII_Buf , unsigned int IntegerValue )
{
	
	unsigned int TempValue ;
	unsigned char ZeroDisable ;
	unsigned int  BaseQty ;
	unsigned int  Loop ;

	ZeroDisable = 1 ;
	BaseQty = 10000 ;

	for ( Loop = 0 ; Loop < 4 ; Loop ++)
	{
		TempValue = IntegerValue / BaseQty ;

			if 	( TempValue > 0) 	
				{
					*ASCII_Buf++ = (unsigned char)TempValue + '0' ;
					ZeroDisable = 0 ;
				}
			else if	( ZeroDisable == 0 ) 
					*ASCII_Buf++ = '0' ;
		IntegerValue = IntegerValue - ( TempValue * BaseQty ) ;
		BaseQty = BaseQty / 10 ;

	}
		*ASCII_Buf++ = (unsigned char)IntegerValue + '0' ;
		*ASCII_Buf = (unsigned char) 0x00 ;
		
}

void Init_IO(void)
{
	ADPCFG = 0xFEB8;					// Cnfig. ADC input, ADPCFG = 1111 1110 1011 1000
										// RB0-> VR2, RB1-> Ext AN1, RB2-> Rxt AN2
										// RB3-> Index, RB4-> QEA, RB5-> QEB
										// RB6-> VR1, RB7-> LCDE, RB8-> Analog Key (SW1~SW4)  
	TRISB = 0xFFFF ;					// Set PORTB as Digital Inputs
	TRISBbits.TRISB7=0;					// RB7 as LCDE output control
	LATBbits.LATB7=0;					// Disable LCD module
	
	TRISE=0x0000; 						// LE1~LED6 are Output
	LATE=0x00FF;						// Turn all LED off
	DIR_SW5  = 1 ;						// Define SW5 & SW6 as Digital Inputs (Switch Push Low)
	DIR_SW6  = 1 ;
}

/*************************************************/
// Timer 模組的初始化副程式
// 10mS Interrupt setting
/*************************************************/
void Init_Timer1(void)
{
		ConfigIntTimer1( T1_INT_PRIOR_7 & T1_INT_ON ) ;		// Timer1 的中斷優先等級設 7 (最高)
															// Timer1 的中斷 ON 

		OpenTimer1( T1_ON & T1_IDLE_STOP & T1_GATE_ON & 	// Timer1 Period as 10mS設為閘控量測模式
					T1_PS_1_1 & T1_SYNC_EXT_OFF & T1_SOURCE_INT , 0xFFFF);

}
void Init_Timer2(void)
{
		ConfigIntTimer2( T2_INT_PRIOR_5 & T2_INT_OFF ) ;	// Disable Timer2 Inerrupt
		OpenTimer2( T2_ON & T2_IDLE_STOP & T2_GATE_OFF & 	// Timer2 Period as 100mS
			T2_PS_1_256 & T2_32BIT_MODE_OFF & T2_SOURCE_INT , 
 						(long)((Fcy/1000)* 100 )/256 ) ; 
}
