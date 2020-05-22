
/*************************************************************
//                dsPIC30F Interrupt Lab 1
//
// Purpose : �]�w Timer ���_�A�H���_�ʤ覡�X�� LED �{�{
/* Timer1 �C���j 1 Sec. �{�{ LED1 �@��(_T1Interrupt) */
/* Timer2 �C���j 0.5 Sec. �{�{ LED1 �@��(_T2Interrupt) */
/* Timer3 �C���j 200mS �{�{ LED1 �@��(_T3Interrupt) */
/* Timer4 �C���j 50mS �{�{ LED1 �@��(_T4Interrupt) */
//
// **************************************************************/

#include 	<p30fxxxx.h>
#include	<timer.h>
#include 	"C30EVM_LCD.h"			// �NLCD�禡���쫬�ŧi�ɮקt�J 

#define		Fosc 	7372800   		// �W�v�� 7.3728 MHz  
									
//---------------------------------------------------------------------------
// �аѦҦ��B���� Configuration bits ��{���������ŧi���覡
// ��ԲӪ��ѼƦC��аѦ� p30F4011.h 
//---------------------------------------------------------------------------
   	_FOSC(CSW_FSCM_OFF & XT);   	 //	XT oscillator, Failsafe clock off
   	_FWDT(WDT_OFF);                  //	Watchdog timer disabled
  	_FBORPOR(PBOR_OFF & MCLR_EN);    //	Brown-out reset disabled, MCLR reset enabled
   	_FGS(CODE_PROT_OFF);             //	Code protect disabled

const char	My_String1[]="T1=00    T2=00 ";	// Declare the LED line 1 String in ROM
const char	My_String2[]="T3=00    T4=00 ";	// Declare the LED line 2 string in ROM

#define	LED1		LATEbits.LATE0 		//�w�qLED���S�w��X��}��줸�����N�Ÿ��A
#define	LED2		LATEbits.LATE1 		//�H�[�j�{�����iŪ�ʻP²��
#define	LED3		LATEbits.LATE2
#define	LED4		LATEbits.LATE3
#define	DIR_LED1	TRISebits.TRISE0 	//�w�qDIR_LED���S�w��X�J��}��줸
#define	DIR_LED2	TRISebits.TRISE1	//����V����줸���N�Ÿ�
#define	DIR_LED3	TRISebits.TRISE2
#define	DIR_LED4	TRISebits.TRISE3

#define	SW5			PORTEbits.RE8		//�w�qSW5�PSW6���S�w��J��}��줸�����N�Ÿ�
#define	SW6			PORTBbits.RB3
#define	DIR_SW5		TRISEbits.TRISE8 	//�w�qDIR_LED7�PDIR_LED8���S�w��X�J��}��줸
#define	DIR_SW6		TRISBbits.TRISB3	//����V����줸���N�Ÿ�

struct
{
	unsigned T1_Flag:1;					
	unsigned T2_Flag:1;
	unsigned T3_Flag:1;
	unsigned T4_Flag:1;
} Timer; 

unsigned char T1_Count=0;
unsigned char T2_Count=0;
unsigned char T3_Count=0;
unsigned char T4_Count=0;

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

	OpenLCD( );							// �ϥ� OpenLCD( )�� LCD �Ҳէ@��l�Ƴ]�w
										// 4 bits Data mode
										// 5 * 7 Character 

	setcurLCD(0,0) ;					// �ϥ� setcurLCD( ) �]�w��Щ� (0,0)
	putrsLCD( My_String1 ) ;			// �N�s�b Program Memory ���r��ϥ�
										// putrsLCD( ) �L�X�� LCD
			
	setcurLCD(0,1) ;
	putrsLCD( My_String2 ) ;			// �ϥ� setcurLCD( ) �]�w��Щ� (0,1)
										// �N�s�b Data Memory ���r��ϥ�	


	while(1)
	{
		if (Timer.T1_Flag)				// Update LCD Dsiplay?
			{ 	
				Timer.T1_Flag=0;		// CLear update flag
 			 	setcurLCD(3,0);			// Set LCD course
				puthexLCD(T1_Count);
			}
		if (Timer.T2_Flag)
			{ 	
				Timer.T2_Flag=0;
 			 	setcurLCD(12,0);
				puthexLCD(T2_Count);
			}
		if (Timer.T3_Flag)
				{ 	
				Timer.T3_Flag=0;
 			 	setcurLCD(3,1);
				puthexLCD(T3_Count);
			}
		if (Timer.T4_Flag)
			{ 	
				Timer.T4_Flag=0;
 			 	setcurLCD(12,1);
				puthexLCD(T4_Count);
			}
	} 
}


void __attribute__((interrupt, no_auto_psv)) _T1Interrupt(void)
{
	LED1=!LED1;							// Flash the LED1
	Timer.T1_Flag=1;					
	T1_Count++;							// Increase Timer1 display counter
	IFS0bits.T1IF = 0 ;					// Clear Timer1 Interrupt Flag
}
void __attribute__((interrupt, no_auto_psv)) _T2Interrupt(void)
{
	LED2=!LED2;
	Timer.T2_Flag=1;
	T2_Count++;
	IFS0bits.T2IF = 0 ;
}
void __attribute__((interrupt, no_auto_psv))_T3Interrupt(void)
{
	LED3=!LED3;
	Timer.T3_Flag=1;
	T3_Count++;
	IFS0bits.T3IF = 0 ;
}
void __attribute__((interrupt, no_auto_psv))_T4Interrupt(void)
{
	LED4=!LED4;
	Timer.T4_Flag=1;
	T4_Count++;
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
// Timer �Ҳժ���l�ưƵ{��
// 100mS Interrupt setting
/*************************************************/
void Init_Timer1(void)
{
		ConfigIntTimer1( T1_INT_PRIOR_7 & T1_INT_ON ) ;		// Timer1 �����_�u�����ų] 7 (�̰�)
															// Timer1 �����_ ON 

		OpenTimer1( T1_ON & T1_IDLE_STOP & T1_GATE_OFF & 	// Timer1 Period as 2 Second
					T1_PS_1_256 & T1_SYNC_EXT_OFF & T1_SOURCE_INT , 
						(long)((Fosc/4/1000)*2000)/256) ;	// Period= 14400
}
void Init_Timer2(void)
{
		ConfigIntTimer2( T2_INT_PRIOR_5 & T2_INT_ON ) ;		// Timer2 �����_�u�����ų] 5 (�̰�)
															// Timer2 �����_ ON 

		OpenTimer2( T2_ON & T2_IDLE_STOP & T2_GATE_OFF & 	// Timer2 Period as 1 Second 
					T2_PS_1_256 & T2_32BIT_MODE_OFF & T2_SOURCE_INT , 
 						(long)((Fosc/4/1000)* 1000 )/256 ) ;	// Period= 7200
}
void Init_Timer3(void)
{
		ConfigIntTimer3( T3_INT_PRIOR_3 & T3_INT_ON ) ;		// Timer3 �����_�u�����ų] 3 (�̰�)
															// Timer3 �����_ ON 

		OpenTimer3( T3_ON & T3_IDLE_STOP & T3_GATE_OFF & 	// Timer3 Period as 500 ms 
					T3_PS_1_256 & T3_SOURCE_INT , 
 						(long)((Fosc/4/1000)* 500 )/256 ) ;
}
void Init_Timer4(void)
{
		ConfigIntTimer4( T4_INT_PRIOR_1 & T4_INT_ON ) ;		// Timer4 �����_�u�����ų] 1 (�̰�)
															// Timer4 �����_ ON 

		OpenTimer4( T4_ON & T4_IDLE_STOP & T4_GATE_OFF & 	// Timer4 Period as 100 ms 
					T4_PS_1_256 & T4_SOURCE_INT & T4_32BIT_MODE_OFF ,  
						(long)((Fosc/4/1000)* 100 )/256 ) ;
}
