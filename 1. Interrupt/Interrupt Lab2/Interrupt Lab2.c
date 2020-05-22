
/*************************************************************
//                dsPIC30F Interrupt Lab 2
//
// Purpose : 設定 Timer 中斷，以中斷動方式驅動 LED 閃爍
/* Timer1 每間隔 1 Sec. 閃爍 LED1 一次(_T1Interrupt), Interrupt Priority as 7 */
/* Timer2 每間隔 0.5 Sec. 閃爍 LED1 一次(_T2Interrupt), Interrupt Priority as 5 */
/* Timer3 每間隔 200mS 閃爍 LED1 一次(_T3Interrupt),,Interrupt Priority as 3 */
/* Timer4 每間隔 50mS 閃爍 LED1 一次(_T4Interrupt), Interrupt Priority as 1 */
//
//	CPU interrupt Priority Power-On default is Level 0
//  Press SW5 to increase the Priority Level of CPU
//	Press SW6 to decrease the Priority Level of CPU
//	Both SW5 & SW6 have key debunce function
//
// **************************************************************/


#include <p30fxxxx.h>
#include	<timer.h>
#include 	"C30EVM_LCD.h"			// 將LCD函式的原型宣告檔案含入 

#define		Fosc 	7372800   		// 頻率為 7.3728 MHz  
									
//---------------------------------------------------------------------------
// 請參考此處有關 Configuration bits 於程式中直接宣告的方式
// 更詳細的參數列表請參考 p30F4011.h 
//---------------------------------------------------------------------------
   	_FOSC(CSW_FSCM_OFF & XT);   	 //	XT oscillator, Failsafe clock off
   	_FWDT(WDT_OFF);                  //	Watchdog timer disabled
  	_FBORPOR(PBOR_OFF & MCLR_EN);    //	Brown-out reset disabled, MCLR reset enabled
   	_FGS(CODE_PROT_OFF);             //	Code protect disabled

const char	My_String1[]="T1=7, T2=5, T3=3";	// Declare the LED line 1 String in ROM
const char	My_String2[]="T4=1, CPU Core=0";	// Declare the LED line 2 string in ROM

#define	LED1		LATEbits.LATE0 		//定義LED為特定輸出埠腳位位元的替代符號，
#define	LED2		LATEbits.LATE1 		//以加強程式的可讀性與簡潔
#define	LED3		LATEbits.LATE2
#define	LED4		LATEbits.LATE3
#define	DIR_LED1	TRISebits.TRISE0 	//定義DIR_LED為特定輸出入埠腳位位元
#define	DIR_LED2	TRISebits.TRISE1	//的方向控制位元替代符號
#define	DIR_LED3	TRISebits.TRISE2
#define	DIR_LED4	TRISebits.TRISE3

#define	SW5			PORTEbits.RE8		//定義SW5與SW6為特定輸入埠腳位位元的替代符號
#define	SW6			PORTBbits.RB3		// RB3 有與 QEI Index 共用 (DSW4 第三個 SW 要 Off)
#define	DIR_SW5		TRISEbits.TRISE8 	//定義DIR_LED7與DIR_LED8為特定輸出入埠腳位位元
#define	DIR_SW6		TRISBbits.TRISB3	//的方向控制位元替代符號

unsigned char debounce=0;
unsigned char i;

void Init_IO(void);
void Init_Timer1(void);
void Init_Timer2(void);
void Init_Timer3(void);
void Init_Timer4(void);

int	main( void )
{
	Init_IO( );
	Init_Timer1( );
	Init_Timer2( );
	Init_Timer3( );
	Init_Timer4( );

	OpenLCD( );							// 使用 OpenLCD( )對 LCD 模組作初始化設定
										// 4 bits Data mode
										// 5 * 7 Character 

	setcurLCD(0,0) ;					// 使用 setcurLCD( ) 設定游標於 (0,0)
	putrsLCD( My_String1 ) ;			// 將存在 Program Memory 的字串使用
										// putrsLCD( ) 印出至 LCD
			
	setcurLCD(0,1) ;
	putrsLCD( My_String2 ) ;			// 使用 setcurLCD( ) 設定游標於 (0,1)
										// 將存在 Data Memory 的字串使用	


	while(1)
	{
		if (debounce==0)
			{
				if (!SW5) 
					{
					SRbits.IPL++;
					setcurLCD(15,1);
					putcLCD('0'+ SRbits.IPL);
					debounce=30;
					}
				if (!SW6) 
					{
					SRbits.IPL--;
					setcurLCD(15,1);
					putcLCD('0'+ SRbits.IPL);
					debounce=30;
					}
			}
		else
			{
				if (SW5 & SW6) 
					{
						debounce--;
						for (i=0;i<100;i++);
					}
				else 
					debounce=30;	
			}		 
	}
}


void __attribute__((interrupt, no_auto_psv))_T1Interrupt(void)
{
	LED1=!LED1;							// Flash the LED1
	IFS0bits.T1IF = 0 ;					// Clear Timer1 Interrupt Flag
}
void __attribute__((interrupt, no_auto_psv))_T2Interrupt(void)
{
	LED2=!LED2;
	IFS0bits.T2IF = 0 ;
}
void __attribute__((interrupt, no_auto_psv))_T3Interrupt(void)
{
	LED3=!LED3;
	IFS0bits.T3IF = 0 ;
}
void __attribute__((interrupt, no_auto_psv))_T4Interrupt(void)
{
	LED4=!LED4;
	IFS1bits.T4IF = 0 ;
}

void Init_IO(void)
{
	ADPCFG = 0xFEB8;					// Cnfig. ADC input, ADPCFG = 1111 1110 1011 1000
										// RB0-> VR2, RB1-> Ext AN1, RB2-> Rxt AN2
										// RB3-> Index, RB4-> QEA, RB5-> QEB
										// RB6-> VR1, RB7-> LCDE, RB8-> Analog Key (SW1~SW4)  
	TRISB = 0xFFFF ;					// Set PORTB as Digital Inputs
	TRISBbits.TRISB7=0;					// RB7 as LEDE output control
	LATBbits.LATB7=0;					// Disable LCD module
	
	TRISE=0x0000; 						// LE1~LED6 are Output
	LATE=0x00FF;						// Turn all LED off
	DIR_SW5  = 1 ;						// Define SW5 & SW6 as Digital Inputs (Switch Push Low)
	DIR_SW6  = 1 ;
}

/*************************************************/
// Timer 模組的初始化副程式
// 100mS Interrupt setting
/*************************************************/
void Init_Timer1(void)
{
		ConfigIntTimer1( T1_INT_PRIOR_7 & T1_INT_ON ) ;		// Timer1 的中斷優先等級設 7 (最高)
															// Timer1 的中斷 ON 

		OpenTimer1( T1_ON & T1_IDLE_STOP & T1_GATE_OFF & 	// Timer1 Period as 100mS
					T1_PS_1_256 & T1_SYNC_EXT_OFF & T1_SOURCE_INT , 
						(long)((Fosc/4/1000)*100)/256) ;	// Period= 14400
}
void Init_Timer2(void)
{
		ConfigIntTimer2( T2_INT_PRIOR_5 & T2_INT_ON ) ;		// Timer2 的中斷優先等級設 5 (最高)
															// Timer2 的中斷 ON 

		OpenTimer2( T2_ON & T2_IDLE_STOP & T2_GATE_OFF & 	// Timer2 Period as 100mS 
					T2_PS_1_256 & T2_32BIT_MODE_OFF & T2_SOURCE_INT , 
 						(long)((Fosc/4/1000)* 100 )/256 ) ;	// Period= 7200
}
void Init_Timer3(void)
{
		ConfigIntTimer3( T3_INT_PRIOR_3 & T3_INT_ON ) ;		// Timer3 的中斷優先等級設 3 (最高)
															// Timer3 的中斷 ON 

		OpenTimer3( T3_ON & T3_IDLE_STOP & T3_GATE_OFF & 	// Timer3 Period as 100 ms 
					T3_PS_1_256 & T3_SOURCE_INT , 
 						(long)((Fosc/4/1000)* 100 )/256 ) ;
}
void Init_Timer4(void)
{
		ConfigIntTimer4( T4_INT_PRIOR_1 & T4_INT_ON ) ;		// Timer4 的中斷優先等級設 1 (最高)
															// Timer4 的中斷 ON 

		OpenTimer4( T4_ON & T4_IDLE_STOP & T4_GATE_OFF & 	// Timer4 Period as 100 ms 
					T4_PS_1_256 & T4_SOURCE_INT & T4_32BIT_MODE_OFF ,  
						(long)((Fosc/4/1000)* 100 )/256 ) ;
}
