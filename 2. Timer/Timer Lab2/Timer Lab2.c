// ***********************************************************************
//
// Purpose : �m�ߦp��ϥ� Timer1 �Ҳտ�J�p��X��J�H���� High Duty
//           �ñN�Ҵ��o�쪺 Duty Cycle  �C�j 100mS ��ܩ� LCD �W
//
// ***********************************************************************

#include 	<p30fxxxx.h>
#include 	"C30EVM_LCD.h"			 
#include	<timer.h>

#define		Fcy 	7372800 * 2		// �]���ϥ��W�v���N�~�� 7.3728 MHz * 8 ���Ҧ� 

   	_FOSC(CSW_FSCM_OFF & XT_PLL8);   //	XT with 8xPLL oscillator, Failsafe clock off
   	_FWDT(WDT_OFF);                  //	Watchdog timer disabled
  	_FBORPOR(PBOR_OFF & MCLR_EN);    //	Brown-out reset disabled, MCLR reset enabled
   	_FGS(CODE_PROT_OFF);             //	Code protect disabled

const char	My_String1[]="Timer1 Gate Lab2" ;	// �ŧi�r��� Program Memory
const  char	My_String2[]="T1CK Duty     uS" ;	 

#define	LED1		LATEbits.LATE0 		//�w�qLED���S�w��X��}��줸�����N�Ÿ��A
#define	LED2		LATEbits.LATE1 		//�H�[�j�{�����iŪ�ʻP²��
#define	LED3		LATEbits.LATE2
#define	LED4		LATEbits.LATE3
#define	LED5		LATEbits.LATE4
#define	LED6		LATEbits.LATE5

#define	DIR_LED1	TRISebits.TRISE0 	//�w�qDIR_LED���S�w��X�J��}��줸
#define	DIR_LED2	TRISebits.TRISE1	//����V����줸���N�Ÿ�
#define	DIR_LED3	TRISebits.TRISE2
#define	DIR_LED4	TRISebits.TRISE3
#define	DIR_LED5	TRISebits.TRISE4
#define	DIR_LED6	TRISebits.TRISE5

#define	SW5			PORTEbits.RE8		//�w�qSW5�PSW6���S�w��J��}��줸�����N�Ÿ�
#define	SW6			PORTBbits.RB3
#define	DIR_SW5		TRISEbits.TRISE8 	//�w�qDIR_LED7�PDIR_LED8���S�w��X�J��}��줸
#define	DIR_SW6		TRISBbits.TRISB3	//����V����줸���N�Ÿ�

void Init_IO(void);
void Init_Timer1(void);
void Init_Timer2(void);
void update_LCD(void);
void uitoa( unsigned char * , unsigned int );

unsigned char	Int_flag;
unsigned int 	Period;
unsigned char	Convert_Buf[10] ;
unsigned char	Delay_Count=0;

void __attribute__((interrupt, no_auto_psv)) _T1Interrupt(void)		// ��J�������_�Ƶ{��
{	
	Period= ReadTimer1( );
	WriteTimer1(0);
	Int_flag = 1;					// �]�w�n�餤�_�X��
	IFS0bits.T1IF = 0 ;					// Clear Timer1 Interrupt Flag
}

int main(void)
{

	Init_IO( );
	Init_Timer1( );
	Init_Timer2( );

	OpenLCD( );							// �ϥ� OpenLCD( )�� LCD �Ҳէ@��l�Ƴ]�w
										// 4 bits Data mode
										// 5 * 7 Character 

	setcurLCD(0,0) ;					// �ϥ� setcurLCD( ) �]�w��Щ� (0,0)
	putrsLCD( My_String1 ) ;			// �N�s�b Program Memory ���r��ϥ�
										// putrsLCD( ) �L�X�� LCD
			
	setcurLCD(0,1) ;
	putrsLCD( My_String2 ) ;			// �ϥ� setcurLCD( ) �]�w��Щ� (0,1)
										// �N�s�b Data Memory ���r��ϥ�
	
 	WriteTimer1(0);						// �NTimer1 �k�s�A�ǳƯߪi�e�׶q��
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
// Timer �Ҳժ���l�ưƵ{��
// 10mS Interrupt setting
/*************************************************/
void Init_Timer1(void)
{
		ConfigIntTimer1( T1_INT_PRIOR_7 & T1_INT_ON ) ;		// Timer1 �����_�u�����ų] 7 (�̰�)
															// Timer1 �����_ ON 

		OpenTimer1( T1_ON & T1_IDLE_STOP & T1_GATE_ON & 	// Timer1 Period as 10mS�]���h���q���Ҧ�
					T1_PS_1_1 & T1_SYNC_EXT_OFF & T1_SOURCE_INT , 0xFFFF);

}
void Init_Timer2(void)
{
		ConfigIntTimer2( T2_INT_PRIOR_5 & T2_INT_OFF ) ;	// Disable Timer2 Inerrupt
		OpenTimer2( T2_ON & T2_IDLE_STOP & T2_GATE_OFF & 	// Timer2 Period as 100mS
			T2_PS_1_256 & T2_32BIT_MODE_OFF & T2_SOURCE_INT , 
 						(long)((Fcy/1000)* 100 )/256 ) ; 
}
