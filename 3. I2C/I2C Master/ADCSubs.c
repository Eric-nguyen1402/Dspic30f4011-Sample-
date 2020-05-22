
#include 	<p30F4011.h>
#include	<timer.h>				// 將Timer函式的原型宣告檔案含入
#include	<adc10.h>				// 將adc10函式的原型宣告檔案含入


/***********************************************/
// Subroutine to configure the A/D module
/***********************************************/

void	Init_ADC(void)
{

	unsigned int Channel, PinConfig, Scanselect, Adcon3_reg, Adcon2_reg, Adcon1_reg;

	ADCON1bits.ADON = 0; /* turn off ADC */

	PinConfig = ENABLE_AN0_ANA & ENABLE_AN6_ANA ;	// Select port pins as analog inputs ADPCFG<15:0>

	Adcon1_reg = ADC_MODULE_ON &		// Turn on A/D module (ADON)
		ADC_IDLE_STOP &					// ADC turned off during idle (ADSIDL)
		ADC_FORMAT_INTG &				// Output in integer format (FORM)
		ADC_CLK_MANUAL &				// Conversion trigger manually (SSRC)
		ADC_SAMPLE_INDIVIDUAL &			// Sample channels individually (SIMSAM)
		ADC_AUTO_SAMPLING_OFF;			// Sample trigger manually (ASAM)

	Adcon2_reg = ADC_VREF_AVDD_AVSS &	// Voltage reference : +AVdd, -AVss (VCFG)
		ADC_SCAN_OFF &					// Scan off (CSCNA)
		ADC_ALT_BUF_OFF &				// Use fixed buffer (BUFM)
		ADC_ALT_INPUT_OFF &				// Does not alternate between MUX A & MUX B (ALTS)
		ADC_CONVERT_CH0 &				// Convert only channel 0 (CHPS)
		ADC_SAMPLES_PER_INT_1;			// 1 sample between interrupt (SMPI)

	Adcon3_reg = ADC_SAMPLE_TIME_10 &	// Auto-Sample time (SAMC)
		ADC_CONV_CLK_SYSTEM &			// Use system clock (ADRC)
		ADC_CONV_CLK_4Tcy;				// Conversion clock = 4 Tcy (ADCS)
										// ADCS = 2*(154ns)/(1/Fcy)-1 = 3.5416

	Scanselect = SCAN_NONE;				// ADC scan no channel (ADCSSL)

	OpenADC10(Adcon1_reg, Adcon2_reg, Adcon3_reg, PinConfig, Scanselect);

	Channel = ADC_CH0_POS_SAMPLEA_AN0 & // CH0 Pos. : AN0, Neg. : Nominal Vref- Defined in ADCON2
		ADC_CH0_NEG_SAMPLEA_NVREF ;		// (ADCHS)
	SetChanADC10(Channel);

	ConfigIntADC10(ADC_INT_DISABLE);	// Disable ADC interrupt

}

int	Get_VR1(void)
{
	unsigned char dummy ;
	unsigned int ADCValue;
	unsigned int ADCSetting ;

	ADCSetting = ADC_CH0_POS_SAMPLEA_AN6 & ADC_CH0_NEG_SAMPLEA_NVREF ;
	SetChanADC10( ADCSetting );

	ADCON1bits.SAMP = 1; 		// start sampling ...

	for  ( dummy = 0 ; dummy < 100 ; dummy ++ );

	ConvertADC10();
	while (BusyADC10()); 		// conversion done?
	
	return ( ADCBUF0 ) ;

}

int	Get_VR2(void)
{

	unsigned char 	dummy ;
	unsigned int 	ADCValue;
	unsigned int	ADCSetting ;

	ADCSetting = ADC_CH0_POS_SAMPLEA_AN0 & ADC_CH0_NEG_SAMPLEA_NVREF ;

	SetChanADC10(ADCSetting);

	ADCON1bits.SAMP = 1; 		// start sampling ...

	for  ( dummy = 0 ; dummy < 100 ; dummy ++ );

	ConvertADC10();
	while (BusyADC10()); 		// conversion done?
	
	return ( ADCBUF0 ) ;
}
