
/*************************************************************
//                dsPIC30F Timer Lab 1
//
//  以每 10mS 中斷的按鍵掃描方式進行 SW1 ~ SW6 的按鍵輸入偵測
//  及處理案件彈跳的問題。
//
// **************************************************************/

#include 	<p30fxxxx.h>
#include	<timer.h>
#include 	<adc10.h>
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
/*
_FUID0(1357);
_FUID1(0xabcd);
_FUID2(0xaa55);
_FUID3(4);
_FUID4(5);
_FUID5(6);
*/
const char	My_String1[]="S1=0, S2=0, S3=0";	// Declare the LED line 1 String in ROM
const char	My_String2[]="S4=0, S5=0, S6=0";	// Declare the LED line 2 string in ROM

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

unsigned char debounce=0;
unsigned char SW_Input = 0x00;
unsigned char AD_Key_Input=0x00;
unsigned char S1='0';
unsigned char S2='0';
unsigned char S3='0';
unsigned char S4='0';
unsigned char S5='0';
unsigned char S6='0';

struct
{
	unsigned Timer1_Update_Flag :1;
} Timer;

void Init_IO(void);
void Init_Timer1(void);
void Init_Timer3(void);
void Init_ADC10(void);
void Update_LCD(unsigned char, unsigned char, unsigned char);

int	main( void )
{
	Init_IO( );
	Init_Timer1( );
	Init_Timer3( );
	Init_ADC10( );

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
		if(Timer.Timer1_Update_Flag)
		{
			Timer.Timer1_Update_Flag=0;
			if (SW_Input=='1') 
			{ 
				LED1=!LED1;
				if (S1++ =='9') S1='0';
				Update_LCD (3,0,S1);
			}
			if (SW_Input=='2') 
			{ 
				LED2=!LED2;
				if (S2++=='9') S2='0';
				Update_LCD (9,0,S2);
			}
			if (SW_Input=='3') 
			{ 
				LED3=!LED3;
				if (S3++=='9') S3='0';
				Update_LCD (15,0,S3);
			}
			if (SW_Input=='4') 
			{ 
				LED4=!LED4;
				if (S4++=='9') S4='0';
				Update_LCD (3,1,S4);
			}
			if (SW_Input=='5') 
			{ 
				LED5=!LED5;
				if (S5++=='9') S5='0';
				Update_LCD (9,1,S5);
			}
			if (SW_Input=='6') 
			{ 
				LED6=!LED6;
				if (S6++=='9') S6='0';
				Update_LCD (15,1,S6);
			}
		}
	}
}

void __attribute__((interrupt, no_auto_psv)) _T1Interrupt(void)
{
	if (debounce ==0)						// Do it need to debounce processing?
	{
		if (!SW5) 							// No need to debounce, Checking the key status
		{
			SW_Input = '5';
			debounce = 8;
			Timer.Timer1_Update_Flag=1; 
		}
		if (!SW6) 
		{
			SW_Input = '6';
			debounce = 8;
			Timer.Timer1_Update_Flag=1;
		}
		if (AD_Key_Input!=0)				// Any Analog key press yet?
		{
			SW_Input = AD_Key_Input;
			debounce = 8;
			Timer.Timer1_Update_Flag=1;
		}
	}
	else
	{
		if (!SW5|!SW6|(AD_Key_Input!=0)) debounce=8;    	// check all key have released
		else debounce--;
	}

	IFS0bits.T1IF = 0 ;					// Clear Timer1 Interrupt Flag
}

void Update_LCD(unsigned char X_Cur, unsigned char Y_Cur, unsigned char Key_ASCII)
{

			setcurLCD(X_Cur,Y_Cur) ;
			putcLCD(Key_ASCII);
}
void __attribute__((interrupt, no_auto_psv)) _ADCInterrupt(void)
{
	unsigned char ADCValue;

	ADCValue = ADCBUF0 >>6 ;							// ADC result shift right 6 bits (/64)
		
	if (ADCValue == 0)	(AD_Key_Input = '1') ;			// Press SW1 ?
	else 
	{	if(ADCValue <= 8) (AD_Key_Input = '2') ;		// Press SW2 ?
		else 
		{	if(ADCValue <= 10) (AD_Key_Input = '3') ;
			else 
			{	if(ADCValue <= 12) (AD_Key_Input = '4') ;
				else (AD_Key_Input = 0x00) ;			// Didn't any key has been press return 0x00
			}
		}
	}
	IFS0bits.ADIF = 0 ;
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

		OpenTimer1( T1_ON & T1_IDLE_STOP & T1_GATE_OFF & 	// Timer1 Period as 10mS
					T1_PS_1_1 & T1_SYNC_EXT_OFF & T1_SOURCE_INT , 
						(long)((Fosc/4/1000)*10)/1) ;	    
}
void Init_Timer3(void)
{
		ConfigIntTimer3( T3_INT_PRIOR_1 & T3_INT_OFF ) ;
		OpenTimer3( T3_ON & T3_IDLE_STOP & T3_GATE_OFF & T3_PS_1_1 & T3_SOURCE_INT ,
				    (long)((Fosc/4/1000)* 3/1)) ;			// Timer3 trigger ADC with 3mS setting
}
/***********************************************/
// Subroutine to initialize ADC module

void Init_ADC10(void)
{

	unsigned int Channel, PinConfig, Scanselect, Adcon3_reg, Adcon2_reg, Adcon1_reg;

	ADCON1bits.ADON = 0; /* turn off ADC */

	PinConfig = ENABLE_AN0_ANA & ENABLE_AN6_ANA & ENABLE_AN8_ANA ;	// Select AN0,AN6 & AN8 as analog inputs
/* ADPCFCG = 0xFEB8 */

	Adcon1_reg= ADC_MODULE_ON &					// Turn on A/D module (ADON)
				ADC_IDLE_STOP &					// ADC turned off during idle (ADSIDL)
				ADC_FORMAT_INTG &				// Output in integer format (FORM)
				ADC_CLK_TMR &				// Timer3 compare ends sampling and starts conversion
				ADC_SAMPLE_INDIVIDUAL &			// Sample channels individually (SIMSAM)
				ADC_AUTO_SAMPLING_ON &			// Sample trigger automatically (ASAM)
				ADC_SAMP_ON ;					// sample / hold amplifiers are sampling
/* ADCON1 = 0x0046 */

	Adcon2_reg= ADC_VREF_AVDD_AVSS &			// Voltage reference : +AVdd, -AVss (VCFG)
				ADC_SCAN_OFF &					// Do not scan input Selection (CSCNA)
				ADC_ALT_BUF_OFF &				// Use fixed buffer (BUFM)
				ADC_ALT_INPUT_OFF &				// Does not alternate between MUX A & MUX B (ALTS)
				ADC_CONVERT_CH0 &				// Convert only channel 0 (CHPS)
				ADC_SAMPLES_PER_INT_1;			// Every 1 samples between interrupt (SMPI)
/* ADCON2 = 0x0000 */

	Adcon3_reg= ADC_SAMPLE_TIME_16 &			// Auto-Sample time are 16 Tad(SAMC)
				ADC_CONV_CLK_SYSTEM &			// Use system clock (ADRC)
				ADC_CONV_CLK_4Tcy;				// Conversion clock = 4 Tcy (ADCS)
/* ADCON3 = 0x0F07*/										

	Scanselect= SCAN_NONE ; 					// Skip AN0-AN15 for Input Scan 
/* ADCSSL = 0x0000 */

	OpenADC10(Adcon1_reg, Adcon2_reg, Adcon3_reg, PinConfig, Scanselect);


	Channel = 	ADC_CH0_POS_SAMPLEA_AN8 &
				ADC_CH0_NEG_SAMPLEA_NVREF ;	  
/* ADCHS = 0x0007 */

	SetChanADC10(Channel);

	ConfigIntADC10(ADC_INT_PRI_3 & ADC_INT_ENABLE);	// Enable ADC interrupt

}

