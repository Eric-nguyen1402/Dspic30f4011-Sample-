// ***********************************************************************
// Purpose : 使用 INT0 來 Trig ADC//
//           當 APP020 S5 被按下時 , ADC 會轉換並產生中斷 
//			 >> LED1 會被 Toggle 
//			 >> Flags.AD_DONE 會被設為 "1" 以顯示數值在LCD上
// ***********************************************************************

#include 	<timer.h>
#include 	<p30F4011.h>
#include	"C30EVM_LCD.h"
#define		LED1		LATEbits.LATE0 
#define		CTRL_LED1	TRISEbits.TRISE0 

#define		FCY	7372800 * 2
 
void		INT0_Initial( void ) ;
void		ADC10_Initial( void ) ;
void		uitoa( unsigned char *, unsigned int ) ;


union
	{
	unsigned Word ; 
	struct		
		{	
			unsigned AD_DONE : 1 ;
			unsigned : 15 ;
		};
	} Flags ;

unsigned int	ADC_Buf ;
unsigned char	ASCII_Buf[10] ;

//---------------------------------------------------------------------------
// Configuration bits

   _FOSC(CSW_FSCM_OFF & XT_PLL8);   //XT with 4xPLL oscillator, Failsafe clock off
   _FWDT(WDT_OFF);                  //Watchdog timer disabled
   _FBORPOR(PBOR_OFF & MCLR_EN);    //Brown-out reset disabled, MCLR reset enabled
   _FGS(CODE_PROT_OFF);             //Code protect disabled

  

void _ISR _ADCInterrupt(void)
{

		Flags.AD_DONE = 1 ;
		IFS0bits.ADIF = 0 ;
		ADC_Buf = ADCBUF0 ;
		LED1 = !LED1 ;

}


int	main( void )

{
		INT0_Initial( ) ;
		ADC10_Initial( ) ;
		OpenLCD( ) ;

		CTRL_LED1 = 0 ;

		Flags.Word = 0 ;
		  putrsLCD("Lab2:ADC Trigger ") ;
		    setcurLCD(0,1) ;
		      putrsLCD("INT Trig AD=    ") ;

 while (1) 
	{
		while ( Flags.AD_DONE == 0 ) ;

			Flags.AD_DONE = 0 ;
			setcurLCD(12,1) ;
			putrsLCD("    ") ;
			setcurLCD(12,1) ;
			uitoa( ASCII_Buf , ADC_Buf );
			putsLCD(ASCII_Buf) ;

	}
}

void	INT0_Initial( void )
{
	TRISEbits.TRISE8 = 1 ;
	INTCON2bits.INT0EP = 1 ; 		// INT0 interrupt on negative edge
	IEC0bits.INT0IE = 0 ;
}

void ADC10_Initial(void)
{

 	ADPCFG = 0xFFBF;				// AN6/RB6 is Analog , others are Digital;
 	ADCON1 = 0x0026;				// 0b0000 0000 0010 0110 
									// Auto convert using INT0 as trigger source
									// A/D Sample Auto-Start
 	ADCON2 = 0x0000;				// ADCON2 = 0000 0000 0000 0000
									// Don't scan inputs , SMPi = 00 ( Interrupt for each sample/convert )
 	ADCSSL = 0x0000;				// no scan input selected .......
 	ADCON3 = 0x1F3F;				// TAD = 8 Tcy , SAMC = 15 TAD 
	ADCHS =  0x06 ;					// ADCHS = 0b 00000000 00000110

	IEC0bits.ADIE = 1 ;				// Enable AD interrupt
	IPC2bits.ADIP = 7 ;				// Set Priority to 7 	>> highest !!
 
 	ADCON1bits.ADON = 1;			// turn ADC ON
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
