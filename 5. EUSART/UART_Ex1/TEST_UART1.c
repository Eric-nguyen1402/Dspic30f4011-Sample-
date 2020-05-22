/************************************************************/
/* 															*/
/*				       EUART Lab 1							*/
/* 															*/
/* Note:													*/
/*		DSW1 – SW1 & SW2 are ON position for the ICD2 		*/
/*             programming & using the PGC, PGD for Debug	*/
/*		DSW2 – SW1 & SW2 are ON position for UART1 (Lab1)	*/
/*             SW3 & SW4 are OFF position for Dis UART2 	*/
/*		DSW3 – SW1 & SW2 are ON position for VR input		*/
/*		DSW4 – All OFF position								*/
/************************************************************/
//
#include 	<p30fxxxx.h>
#include 	"C30EVM_LCD.h"			// 將LCD函式的原型宣告檔案含入 
#include	<Stdio.h>
#include 	<math.h>

#include 	<uart.h>				// 將UART函式的原型宣告檔案含入
#define 	UART_ALTRX_ALTTX        0xFFE7  /*Communication through ALT pins*/
#define 	UART_RX_TX              0xFBE7  /*Communication through the normal pins*/
// 修正 UART.H 檔案力對此之定義，舊版 C30 有此定義，但新版 C30 v3.12 找不到此定義，
// 所以另外加入此定義已修正UART.H 檔的錯誤

void	Test_UART(void);
void	Init_UART(void);

const char	UART_LCD1[]="* Test UART 1 * " ;
const char	UART_LCD2[]="Send a float out" ;
const char  UART_LCD3[]=" Received UART1 " ;
const char  UART_LCD4[]="                " ;
const char  UART_LCD5[]="Press 'Enter' ??" ;

float Sin_A;
char Rec_Buffer=0;
char Rec_Flag=0;
char Rec_Count=0;
#define PI 3.14159

   	_FOSC(CSW_FSCM_OFF & XT_PLL8);   //	XT with 8xPLL oscillator, Failsafe clock off
   	_FWDT(WDT_OFF);                  //	Watchdog timer disabled
  	_FBORPOR(PBOR_OFF & MCLR_EN);    //	Brown-out reset disabled, MCLR reset enabled
   	_FGS(CODE_PROT_OFF);             //	Code protect disabled

void __attribute__((interrupt, no_auto_psv))_U1RXInterrupt(void)
{
	Rec_Buffer = ReadUART1( );  	// Read data from Receiver FIFO
	Rec_Flag = 1; 		     		// Set the Received Flag
    IFS0bits.U1RXIF = 0 ;	    	// Clear Interrupt Flag
}


int	main( void )
{
	SRbits.IPL=4;




	OpenLCD( ) ;					// 使用 OpenLCD( )對 LCD 模組作初始化設定
	Test_UART( );					// UART 模組測試程式
}

void Test_UART(void )
{
	unsigned char n;

	Init_UART( ) ;										// 對 UART 模組作初始化設定

	printf("\x1b[2J");

	setcurLCD(0,0) ;									// 使用 setcurLCD( ) 設定游標於 (0,0)
	putrsLCD( UART_LCD1 ) ;
	setcurLCD(0,1) ;									// 使用 setcurLCD( ) 設定游標於 (0,1)
	putrsLCD( UART_LCD2 ) ;

	printf("************************************************************\r\n");
	printf("*    Microchip Workshop RTC Training         Exercise 1 :  *\r\n");
    printf("*    Please send the folat value form SIN 0 ~ 180 deg      *\r\n");
	printf("************************************************************\r\n");

	for	(n=0;n<190;n+=10)
	{
		Sin_A = sinf((n*PI)/180);
		printf("     SIN %3d deg = %f\n\r",n,Sin_A);
	}

	setcurLCD(0,1) ;									// 使用 setcurLCD( ) 設定游標於 (0,1)
	putrsLCD( UART_LCD5 ) ;


		while (1)
		{
			while ( !Rec_Flag) ;

			Rec_Flag=0;
			if (Rec_Buffer==0x0d)     		// Received the "Enter" key ?
				{
					setcurLCD(0,0) ;		// Yes, show the " Received UART1 " 
					putrsLCD( UART_LCD3 ) ;  
					setcurLCD(0,1) ;
					putrsLCD( UART_LCD4 ) ; //Clear Line 2 of LCD
					setcurLCD(0,1) ;
					Rec_Count=0;
				}
			else
				{	
					putcLCD(Rec_Buffer);
					Rec_Count++; 
					if (Rec_Count>=16)
					{
						setcurLCD(0,1) ;
						Rec_Count=0;
					}
				}
		
		}
}

/***********************************************/
// Subroutine to initialize UART module

void	Init_UART(void)
{
	/* Holds the value of baud register */
	unsigned int baudvalue;
	/* Holds the value of uart config reg */
	unsigned int U1MODEvalue;
	/* Holds the information regarding uart
	TX & RX interrupt modes */
	unsigned int U1STAvalue;
	/* Turn off UART1module */
	CloseUART1();
	/* Configure uart1 receive and transmit interrupt */
	ConfigIntUART1(UART_RX_INT_EN & UART_RX_INT_PR6 &
	UART_TX_INT_DIS & UART_TX_INT_PR2);
	/* Setup the Buad Rate Generator */
	baudvalue = 95;			//UxBRG = ( (FCY/Desired Baud Rate)/16) – 1
							//UxBRG = ( (7372800*2/9600)/16-1) = 95
	/* Configure UART1 module to transmit 8 bit data with one stopbit.
	Also Enable loopback mode */
	U1MODEvalue = UART_EN & UART_IDLE_CON &
				UART_ALTRX_ALTTX &
				UART_DIS_WAKE &
				UART_DIS_LOOPBACK & UART_DIS_ABAUD &
				UART_NO_PAR_8BIT & UART_1STOPBIT;

	U1STAvalue = UART_INT_TX_BUF_EMPTY &
				UART_TX_PIN_NORMAL &
				UART_TX_ENABLE & UART_INT_RX_CHAR &
				UART_ADR_DETECT_DIS &
				UART_RX_OVERRUN_CLEAR;

	OpenUART1(U1MODEvalue, U1STAvalue, baudvalue);

	return;

}
