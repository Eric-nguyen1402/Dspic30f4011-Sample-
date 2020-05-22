// ***********************************************************************
// File : 		I2CSIM_Main.C

// ***********************************************************************

#include 	<p30F4011.h>
#include 	"C30EVM_LCD.h"			// �NLCD�禡���쫬�ŧi�ɮקt�J 
#include	<timer.h>				// �NTimer�禡���쫬�ŧi�ɮקt�J
#include	<adc10.h>				// �Nadc10�禡���쫬�ŧi�ɮקt�J
#include	"ADCSubs.h"
#include	"I2CSubs.h"

#define		I2C_DEVICE_ADDR	0xA0

#define		FCY 	7372800 * 2		// �]���ϥ��W�v���N�~�� 7.3728 MHz * 8 ���Ҧ� , �C�@���O�g���� 4 �� clock
									// �ҥH FCY =  (7.3728 * 8 / 4 ) MHz = 7372800* 2

   	_FOSC(CSW_FSCM_OFF & XT_PLL8);   //	XT with 8xPLL oscillator, Failsafe clock off
   	_FWDT(WDT_OFF);                  //	Watchdog timer disabled
  	_FBORPOR(PBOR_OFF & MCLR_EN);    //	Brown-out reset disabled, MCLR reset enabled
   	_FGS(CODE_PROT_OFF);             //	Code protect disabled

const char	My_String1[]="VR1:    VR2:    " ;	// �ŧi�r��� Program Memory (�]�� const �ŧi)
	  char	My_String2[]="EE1:    EE2:    " ;	// �ŧi�r��� Data Memory

unsigned int miliSec ;

void	Init_ADC(void) ;
void	Show_ADC(void) ;

// union �ŧi�N��8�줸�ܼ�ByteAccess�PSystemFlag���c�ܼƨϥάۦP���O����A
// �H�Q���P�榡���줸�B��ݨD
union 	{
			unsigned char ByteAccess ;
			struct 	{
						unsigned Bit0: 1 ;
						unsigned Bit1: 1 ;
						unsigned Bit2: 1 ;
						unsigned unused : 5 ;
					} ;
		} SystemFlag ;

//�w�qOneSecond�X�е��P��SystemFlag.Bit0�줸�ܼơA�G�N��ϥΤ@�Ӧ줸�O�ЪŶ�
#define	OneSecond	SystemFlag.Bit0 	

void _ISR _T1Interrupt(void)		//Timer1���_�Ƶ{��
{

	miliSec += 1 ;

	if (miliSec == 1000)			//�C1000���NOneSecond�X�г]�w��1
		{
			OneSecond = 1 ;
			miliSec  = 0 ;
		} 
	IFS0bits.T1IF = 0 ;				//�M�����_�X��

}


	unsigned int 	ADC_Value;
	unsigned char 	EEPROM_Data1 , EEPROM_Data2 ;
	int				WaitLoop ;

int	main( void )
{


	Init_ADC( ) ;										// �NADC�i���l�Ƴ]�w
	Init_I2C( ) ;

	OpenLCD( ) ;										// �ϥ� OpenLCD( )�� LCD �Ҳէ@��l�Ƴ]�w
														// 4 bits Data mode
														// 5 * 7 Character 

	setcurLCD(0,0) ;									// �ϥ� setcurLCD( ) �]�w��Щ� (0,0)
	putrsLCD( My_String1 ) ;							// �N�s�b Program Memory ���r��ϥ�
														// putrsLCD( ) �L�X�� LCD
			
	setcurLCD(0,1) ;									// �ϥ� setcurLCD( ) �]�w��Щ� (0,1)
	putrsLCD( My_String2 ) ;							// �N�s�b Data Memory ���r��ϥ�
														// putsLCD( ) �L�X�� LCD

	ConfigIntTimer1( T1_INT_PRIOR_7 & T1_INT_ON ) ;		// Timer1 �����_�u�����ų] 7 (�̰�)
														// Timer1 �����_ ON 

	OpenTimer1( T1_ON & T1_IDLE_STOP & T1_GATE_OFF & 			// Timer1 �� Period �]���C 1ms 
				T1_PS_1_1 & T1_SYNC_EXT_OFF & T1_SOURCE_INT ,
				(FCY/ 1000) ) ;

	OneSecond = 0 ;

	while(1) 
	{	
		if ( OneSecond ) 								// �߰� Timer1 �� Period �ɶ��O�_�w��
														// �i�H�γn��������ˬd�O�_���ǽT�� 1 ms
		{
			OneSecond = 0 ;		

				WaitLoop = 0 ;
				do
				{
					if ( !I2C_ACKPolling(I2C_DEVICE_ADDR )) 		// Return value : 0= ACK  , 1 = NACK
						{		
							EEPROM_Data1 = EEPROM_ByteRead( I2C_DEVICE_ADDR  , 0x10  ) ;
							EEPROM_Data2 = EEPROM_ByteRead( I2C_DEVICE_ADDR , 0x20 ) ;	
							WaitLoop = 1000 ;
						}
					else
							WaitLoop ++ ;

				}while (WaitLoop < 1000) ;					 
					
			ADC_Value = Get_VR1( ) ;								
			setcurLCD(4,0) ;							// Set LCD cursor		
			put_Num_LCD( ADC_Value >> 2 ) ;				// �N�����ഫ���G�H�Q�i��Ʀr��ܦܲG����ܾ�

				WaitLoop = 0 ;
				do
				{
					if ( !I2C_ACKPolling(I2C_DEVICE_ADDR )) 		// Return value : 0= ACK  , 1 = NACK
						{		
							EEPROM_ByteWrite( I2C_DEVICE_ADDR , 0x10 , ADC_Value >> 2 ) ;
							WaitLoop = 1000 ;
						}
					else
							WaitLoop ++ ;
				}while (WaitLoop < 1000) ;

			ADC_Value = Get_VR2( ) ;								
			setcurLCD(12,0) ;							// Set LCD cursor		
			put_Num_LCD( ADC_Value >> 2 ) ;				// �N�����ഫ���G�H�Q�i��Ʀr��ܦܲG����ܾ�	

					WaitLoop = 0 ;
				do
				{
					if ( !I2C_ACKPolling(I2C_DEVICE_ADDR )) 		// Return value : 0= ACK  , 1 = NACK
						{		
							EEPROM_ByteWrite( I2C_DEVICE_ADDR , 0x20 , ADC_Value >> 2 ) ;
							WaitLoop = 1000 ;
						}
					else
							WaitLoop ++ ;
				}while (WaitLoop < 1000) ;

					


			setcurLCD(4,1) ;							// Set LCD cursor		
			put_Num_LCD( EEPROM_Data1 ) ;				// �N�����ഫ���G�H�Q�i��Ʀr��ܦܲG����ܾ�
			setcurLCD(12,1) ;							// Set LCD cursor		
			put_Num_LCD( EEPROM_Data2 ) ;				// �N�����ഫ���G�H�Q�i��Ʀr��ܦܲG����ܾ�

		}
	}		
}


