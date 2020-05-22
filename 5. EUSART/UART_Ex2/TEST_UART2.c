/************************************************************/
/* 															*/
/*				       EUART Lab 2							*/
/* 															*/
/* Note:													*/
/*		DSW1 �V SW1 & SW2 are ON position for the ICD2 		*/
/*             programming & using the PGC, PGD for Debug	*/
/*		DSW2 �V SW1 & SW2 are OFF position for UART1 (Lab1)	*/
/*             SW3 & SW4 are ON position for UART2 (Lab2)	*/
/*		DSW3 �V SW1 & SW2 are ON position for VR input		*/
/*		DSW4 �V All OFF position								*/
/************************************************************/
//

#include 	<p30fxxxx.h>
#include 	"C30EVM_LCD.h"			// �NLCD�禡���쫬�ŧi�ɮקt�J 
#include	<Stdio.h>
#include 	<math.h>

#include 	<uart.h>				// �NUART�禡���쫬�ŧi�ɮקt�J
#define 	UART_ALTRX_ALTTX        0xFFE7  /*Communication through ALT pins*/
#define 	UART_RX_TX              0xFBE7  /*Communication through the normal pins*/
// �ץ� UART.H �ɮפO�惡���w�q�A�ª� C30 �����w�q�A���s�� C30 v3.12 �䤣�즹�w�q�A
// �ҥH�t�~�[�J���w�q�w�ץ�UART.H �ɪ����~


void	Test_UART(void);
void	Init_UART(void);

const char	UART_LCD1[]="* Test UART 2 * " ;
const char	UART_LCD2[]="Send a float out" ;

float Sin_A;
#define PI 3.14159

   	_FOSC(CSW_FSCM_OFF & XT_PLL8);   //	XT with 8xPLL oscillator, Failsafe clock off
   	_FWDT(WDT_OFF);                  //	Watchdog timer disabled
  	_FBORPOR(PBOR_OFF & MCLR_EN);    //	Brown-out reset disabled, MCLR reset enabled
   	_FGS(CODE_PROT_OFF);             //	Code protect disabled


int	main( void )
{
	OpenLCD( ) ;					// �ϥ� OpenLCD( )�� LCD �Ҳէ@��l�Ƴ]�w
	Test_UART( );					// UART �Ҳմ��յ{��

	while(1);
}

void Test_UART(void )
{
	unsigned char n;

	Init_UART( ) ;										// �� UART �Ҳէ@��l�Ƴ]�w

	printf("\x1b[2J");									// Clear VT-100 Termenal Screen

	setcurLCD(0,0) ;									// �ϥ� setcurLCD( ) �]�w��Щ� (0,0)
	putrsLCD( UART_LCD1 ) ;
	setcurLCD(0,1) ;									// �ϥ� setcurLCD( ) �]�w��Щ� (0,1)
	putrsLCD( UART_LCD2 ) ;

	printf("************************************************************\r\n");
	printf("*     Microchip Workshop RTC Training      Exercise 2 :    *\r\n");
    printf("*    Please chang the UART1 to UART2 for send the flaot    *\r\n");
	printf("************************************************************\r\n");

	for	(n=0;n<190;n+=10)
	{
		Sin_A = sinf((n*PI)/180);
		printf("     SIN %3d deg = %f\n\r",n,Sin_A);
	}
}

/***********************************************/
// Subroutine to initialize UART module

void	Init_UART(void)
{
	/* Holds the value of baud register */
	unsigned int baudvalue;
	/* Holds the value of uart config reg */
	unsigned int U2MODEvalue;
	/* Holds the information regarding uart
	TX & RX interrupt modes */
	unsigned int U2STAvalue;
	/* Turn off UART1module */
	CloseUART2();

	/* Configure uart1 receive and transmit interrupt */
	ConfigIntUART2(UART_RX_INT_EN & UART_RX_INT_PR6 &
	UART_TX_INT_DIS & UART_TX_INT_PR2);
	/* Setup the Buad Rate Generator */
	baudvalue = 95;			//UxBRG = ( (FCY/Desired Baud Rate)/16) �V 1
							//UxBRG = ( (7372800*2/9600)/16-1) = 95

	/* Configure UART1 module to transmit 8 bit data with one stopbit.
	Also Enable loopback mode */
	U2MODEvalue = UART_EN & UART_IDLE_CON &
				UART_DIS_WAKE & UART_DIS_LOOPBACK &
				UART_DIS_ABAUD & UART_NO_PAR_8BIT &
				UART_1STOPBIT & UART_RX_TX;
	U2STAvalue = UART_INT_TX_BUF_EMPTY &
				UART_TX_PIN_NORMAL &
				UART_TX_ENABLE & UART_INT_RX_CHAR &
				UART_ADR_DETECT_DIS &
				UART_RX_OVERRUN_CLEAR;
	OpenUART2(U2MODEvalue, U2STAvalue, baudvalue);

	return;

}
