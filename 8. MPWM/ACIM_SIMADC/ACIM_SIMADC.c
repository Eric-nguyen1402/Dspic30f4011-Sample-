// ***********************************************************************
// File : Ex4_ACIM_SIMADC.C
// Purpose : 使用 Motor Control PWM 送出 60 Hz 的  sin wave  於 PWM1L/H
//			 使用 10-bit ADC 的同步取樣及轉換功能來擷取信號
// ***********************************************************************

//#include <uart.h>
//#include <adc12.h>
#include 	<timer.h>
#include 	<math.h>
#include 	<p30F4011.h>
#include	<uart.h>
#include	<stdio.h>
#include	"C30EVM_LCD.h"

#define		pi	3.1415926 
#define		FCY	7372800 * 2

#define	MAX_HALF_DUTY	730	// Max Duty is 180 , but multiplex with 100 

#define	Volt0			736

void	Timer1_Initial( void ) ;
void	Timer3_Initial( void ) ;
void 	DelayNmSec(unsigned int ) ;
void 	ADC10_Initial(void);
void	MotPWM_Initial(void);


//---------------------------------------------------------------------------
// Configuration bits

//   _FOSC(CSW_FSCM_OFF & EC_PLL4);   //XT with 4xPLL oscillator, Failsafe clock off
//   _FWDT(WDT_OFF);                  //Watchdog timer disabled
//   _FBORPOR(PBOR_OFF & MCLR_EN);    //Brown-out reset disabled, MCLR reset enabled
//   _FGS(CODE_PROT_OFF);             //Code protect disabled

typedef struct tagLEDBITS {
   
        unsigned     	:8;
        unsigned LED_Val:8;
} PORTDBITSS;
extern volatile PORTDBITSS LED_Ctrl __attribute__((__near__));


unsigned char	TxData[10] = {0,0,0,0,0,0,0,0,0,0} ;

unsigned int	ValuePDC ;

int				SinTable[92] ;
int				AN0_Buf[128] ;
int				AN1_Buf[128] ;
int				AN2_Buf[128] ;

int				LoopVar1 ;
int				U_Degree ;
int				V_Degree ;
int				W_Degree ;
int				T1IF_Flag ;
int				T1IF_Counter ;

int				ANxBuf_Index ;

unsigned int	Temp_Uint ;

void __attribute__((__interrupt__)) _T1Interrupt(void)
{

		U_Degree += 3 ;
		if ( U_Degree > 360 ) 	U_Degree = 0 ;
		{

			if ( U_Degree <= 90 )
				ValuePDC = ( SinTable[U_Degree] ) + Volt0 ;

			else if ( U_Degree > 90 && U_Degree <= 180 )
				ValuePDC = ( SinTable[180 - U_Degree] )+ Volt0 ;

			else if ( U_Degree > 180 && U_Degree <= 270 )
				ValuePDC = Volt0 - ( SinTable[U_Degree - 180] ) ;

			else if ( U_Degree > 270 && U_Degree <= 360 )
				ValuePDC = Volt0 - ( SinTable[360-U_Degree] ) ;
	
				PDC1 = ValuePDC ;
		}

			V_Degree = U_Degree + 120 ;
			if ( V_Degree > 360 )	V_Degree = V_Degree - 360 ; 
			
		{
			if ( V_Degree <= 90 )
				ValuePDC = ( SinTable[V_Degree] )+ Volt0 ;

			else if ( V_Degree > 90 && V_Degree <= 180 )
				ValuePDC = ( SinTable[180 - V_Degree ] )+ Volt0 ;

			else if ( V_Degree > 180 && V_Degree <= 270 )
				ValuePDC = Volt0 - ( SinTable[V_Degree - 180]) ;

			else if ( V_Degree > 270 && V_Degree <= 360 )
				ValuePDC = Volt0 - ( SinTable[360-V_Degree] ) ;
	
				PDC2 = ValuePDC ;
		}

			W_Degree = U_Degree + 240 ;
			if ( W_Degree > 360 )	W_Degree = W_Degree - 360 ; 
			
		{
			if ( W_Degree <= 90 )
				ValuePDC = ( SinTable[W_Degree] )+ Volt0 ;

			else if ( W_Degree > 90 && W_Degree <= 180 )
				ValuePDC = ( SinTable[180 - W_Degree ])+ Volt0 ;

			else if ( W_Degree > 180 && W_Degree <= 270 )
				ValuePDC = Volt0 - ( SinTable[W_Degree - 180]) ;

			else if ( W_Degree > 270 && W_Degree <= 360 )
				ValuePDC = Volt0 - ( SinTable[360-W_Degree]) ;
	
				PDC3 = ValuePDC ;
		}

	T1IF_Counter += 1 ;

	if (T1IF_Counter > 4000)
		{
			T1IF_Counter = 0 ;
			T1IF_Flag = 1 ;
		} 
	IFS0bits.T1IF = 0 ;

}

void __attribute__((__interrupt__)) _PWMInterrupt(void)
{

		IFS2bits.PWMIF = 0 ;
}

void _ISR _ADCInterrupt(void)
{

		IFS0bits.ADIF = 0 ;

			if ( ANxBuf_Index < 128 )
			{
				AN0_Buf[ANxBuf_Index] = ADCBUF1 ;
				AN1_Buf[ANxBuf_Index] = ADCBUF2 ;
				AN2_Buf[ANxBuf_Index++] = ADCBUF3 ;
			}

		LATFbits.LATF1 = ! LATFbits.LATF1 ;
}


int	main( void )

{

	OpenLCD( ) ;

	for ( LoopVar1 = 0 ; LoopVar1 < 91 ; LoopVar1 ++ )
	{
		SinTable[LoopVar1] = sin( (3.1415926 * LoopVar1) / (double)180 ) * MAX_HALF_DUTY  ;
	}

	Timer1_Initial( ) ;
	Timer3_Initial( ) ;
	MotPWM_Initial( ) ;
	ADC10_Initial( ) ;

		ANxBuf_Index = 0 ;

		TRISD &= 0x00ff ;
		U_Degree = 0 ;

		TRISDbits.TRISD0 = 0 ;
		TRISDbits.TRISD1 = 0 ;

		TRISFbits.TRISF0 = 0 ;
		TRISFbits.TRISF1 = 0 ;

		TRISFbits.TRISF4 = 0 ;
		TRISFbits.TRISF5 = 0 ;

		LATDbits.LATD0 = 0 ;
		LATDbits.LATD1 = 1 ;

		LATFbits.LATF0 = 0 ;
		LATFbits.LATF1 = 1 ;

		T1IF_Flag = 0 ;
		T1IF_Counter = 0 ;
	
		putrsLCD("dsPIC30F4011 Dem") ;
		setcurLCD(0,1) ;
		putrsLCD("PWM Running     ") ;

 while (1) 
	{
		while ( T1IF_Flag == 0 ) ;

			T1IF_Flag = 0 ;

			IFS0bits.T1IF = 0 ;
			LATDbits.LATD0 = ! LATDbits.LATD0 ;
			LATDbits.LATD1 = ! LATDbits.LATD1 ;

	}
}

void DelayNmSec(unsigned int N)
{
unsigned int j;
while(N--)
 	for(j=0;j < 1000;j++);
}


void	Timer1_Initial( void )
{
		ConfigIntTimer1( T1_INT_PRIOR_7 & T1_INT_ON ) ;
		OpenTimer1( T1_ON & T1_IDLE_STOP & T1_GATE_OFF & T1_PS_1_1 & T1_SYNC_EXT_OFF & T1_SOURCE_INT ,
					2048 ) ;
}

void	Timer3_Initial( void )
{
		ConfigIntTimer3( T3_INT_PRIOR_7 & T3_INT_OFF ) ;
		OpenTimer3( T3_ON & T3_IDLE_STOP & T3_GATE_OFF & T3_PS_1_1 & T3_SOURCE_INT ,
				    (((long)FCY/1000 )) ) ;
}

void 	MotPWM_Initial(void)
{
		IEC2bits.PWMIE = 0 ;	// Disable PWM Interrupt !!
		IEC2bits.FLTAIE = 0 ;	

		OVDCON = 0xff00 ;			// Inactive all PWM OUTPUT !!

		TRISE = 0xffc0 ;		
		PTCON = 0xa000 ;		// Configure as 0b1010 0000 0000 0000 
								// PWM Time Base OFF , PWM Time Base OP in free running Mode 
		PWMCON1 = 0x0077 ;		// Configure as 0b0000000000010001
								// PWM I/O in complementary Mode and only PWM1L/H as PWM output 
		PWMCON2 = 0x0000 ;		// Configure as 0b0000000000000000 

		DTCON1 = 0x0101 ;		// Configure as 0b0000001000000010 ;

		FLTACON = 0x0000 ;

		IPC9bits.PWMIP = 6 ;

		// ---------------------------------------------------------------------------------
		// The Switching Frequency !!
		// PWM resolution >= 10 bits , 
		// PDCx[1:15] compare with PTMR [0:14]
		// PDCx(0) compare with MSB of prescaler counter
		// So, PTPER is 9 bit if resolution of PDCx is 10 bit
		// Setting PWM Frequency = 20K
		// PTPER = ( (7372800*2)/ 20000 ) -1 = 736.28 = 736
		// PWM Frequency will be Fcy/736 = 20.0K
		// Formular !! PTPER = (Fcy/(FPWM*PTMR Prescaler)) - 1 
		// ---------------------------------------------------------------------------------
		PTPER = 736 ;			// PWM Time Base Period Register 
		ValuePDC = 0x00 ;

			PDC1 = ValuePDC ;
			PDC2 = ValuePDC ;
			PDC3 = ValuePDC ;		

}


void ADC10_Initial(void)
{

 	ADPCFG = 0xFFF8;				// all PORTB = Digital; RB0-RB2 = analog
 	ADCON1 = 0x034C;				// Auto convert using TMR3 as trigger source
									// then convert , Sample CH0 , CH1 , CH2 & CH3 Simultaneously
 	ADCON2 = 0x0300;				// scan inputs and interrupt after 12 samples
									// ADCON2 = 0000 0011 0000 0000
 	ADCSSL = 0x0000;				// no scan input selected .......
 	ADCON3 = 0x0F0F;				// TMR3 = 10ms , Tad = 8Tcy 

	ADCHS =  0x08 ;					// ADCHS = 0b 00000000 00000000

	IEC0bits.ADIE = 1 ;				// Enable AD interrupt
	IPC2bits.ADIP = 7 ;				// Set Priority to 7 	>> highest !!
 
 	ADCON1bits.ADON = 1;			// turn ADC ON
}
