// ***********************************************************************
//                Microchip FAE Training        Exercise 5
//
// Purpose : �m�ߦp��ϥ� QEI �Ҳժ���ӿ�J QEA & QEB �p��X��t
//           �ñN��t�H���� 300mS ���ɶ���ܩ� LCD �W
//
// ***********************************************************************

#include 	<p30fxxxx.h>
#include 	<timer.h>
#include	<qei.h>					
#include 	"C30EVM_LCD.h"			// �NLCD�禡���쫬�ŧi�ɮקt�J 

#define		FCY 	7372800 * 2		// �]���ϥ��W�v���N�~�� 7.3728 MHz * 8 ���Ҧ� , �C�@���O�g���� 4 �� clock
									// �ҥH FCY =  (7.3728 * 8 / 4 ) MHz = 7372800* 2

//---------------------------------------------------------------------------
// �аѦҦ��B���� Configuration bits ��{���������ŧi���覡
// ��ԲӪ��ѼƦC��аѦ� p30F4011.h 
//---------------------------------------------------------------------------
   	_FOSC(CSW_FSCM_OFF & XT_PLL8);   //	XT with 8xPLL oscillator, Failsafe clock off
   	_FWDT(WDT_OFF);                  //	Watchdog timer disabled
  	_FBORPOR(PBOR_OFF & MCLR_EN);    //	Brown-out reset disabled, MCLR reset enabled
   	_FGS(CODE_PROT_OFF);             //	Code protect disabled

const char	My_String1[]="RPM Measurement" ;	// �ŧi�r��� Program Memory
const char	My_String2[]=" RPM :         " ;	// �ŧi�r��� Data Memory

void	Init_Timer1(void) ;
void	Init_QEI (void) ;
void	uitoa( unsigned char * , unsigned int ) ;

unsigned char	Minute = 0 ;
unsigned char	Second = 0 ;
unsigned char	TimeBase = 0 ;
unsigned int	QEI_Old = 0 ;
unsigned int	QEI_New = 0 ;
unsigned int	QEI_Diff ;
unsigned int	RPM ;
unsigned int	LCD_Update_Count=3;  			// 300mS update on LCD

unsigned char	Convert_Buf[10] ;

union 	{
			unsigned char ByteAccess ;
			struct 	{
						unsigned Bit0: 1 ;
						unsigned Bit1: 1 ;
						unsigned Bit2: 1 ;
						unsigned unused : 5 ;
					} ;
		} SystemFlag ;

#define	OneSecond	SystemFlag.Bit0 
#define	QEI_Update 	SystemFlag.Bit2

void _ISR _T1Interrupt(void)
{
	TimeBase += 1 ;

	QEI_New = ReadQEI( ) ;
	QEI_Update = 1 ;

	if (TimeBase >= 10)				// Time is over 1 Second?
		{
			OneSecond = 1 ;
			TimeBase  = 0 ;
		} 

	if (LCD_Update_Count!=0) LCD_Update_Count--;

	IFS0bits.T1IF = 0 ;

}

int	main( void )
{

	OpenLCD( ) ;											// �ϥ� OpenLCD( )�� LCD �Ҳէ@��l�Ƴ]�w
															// 4 bits Data mode
															// 5 * 7 Character 
	ADPCFG = 0xffff ;										// AN3 , AN4 , AN5 as difital for QEI !!

	QEI_Old = 0 ;
	QEI_New = 0 ;
	Init_QEI( ) ;
	Init_Timer1( );


		setcurLCD(0,0) ;									// �ϥ� setcurLCD( ) �]�w��Щ� (0,0)
		putrsLCD( My_String1 ) ;							// �N�s�b Program Memory ���r��ϥ�
															// putrsLCD( ) �L�X�� LCD
			
		setcurLCD(0,1) ;
		putrsLCD( My_String2 ) ;							// �ϥ� setcurLCD( ) �]�w��Щ� (0,1)
															// �N�s�b Data Memory ���r��ϥ�	
															// putsLCD( ) �L�X�� LCD
		OneSecond = 0 ;

		while(1) 
		{	
 			if ( QEI_Update )								// RPM calclation every 100mS
			{
				QEI_Update = 0 ;

				setcurLCD(8,1) ;

				if ( QEICONbits.UPDN )		putcLCD('+') ;		// �ھ�UPDN�줸�M�w���઺��V
				else						putcLCD('-') ;
					
				
				if ( QEICONbits.UPDN )							// ���F����ɪ��p��
				{
					if ( QEI_New >= QEI_Old )					// �p����ɶ���������ƶq
						QEI_Diff = QEI_New - QEI_Old ;
					else										// ����ɪ��B�z
						QEI_Diff = QEI_New + ( 65536 - QEI_Old ) ;	
				}
				else											// ���F����ɪ��p��
				{
					if (QEI_New < QEI_Old )
						QEI_Diff = QEI_Old - QEI_New ;
					else
						QEI_Diff = ( 65536 - QEI_New )+ QEI_Old ; // �ɦ�ɪ��B�z
				}
				
				QEI_Old = QEI_New ;					// ��s��QEI�ƭ�
				
				RPM = ((long)QEI_Diff*600/256);		// ��t���p�� (100mS * 600= 1 Minute, Index=64 ��4����� = 256)


				if (LCD_Update_Count==0)
				{
					uitoa ( Convert_Buf , RPM ) ;		// �ഫ���Q�i���A�ন ASCII Code

					setcurLCD(10,1) ;					// �����t�b LCD 
					putrsLCD( "     " ) ;

					setcurLCD(10,1) ;
					putsLCD( Convert_Buf ) ;

					LCD_Update_Count=3;					// Set LCD update time for 300mS
				}

			}
	
		}		
}

/*************************************************/
// Timer1 �Ҳժ���l�ưƵ{��
// 100mS Interrupt setting
/*************************************************/
void Init_Timer1(void)
{
		ConfigIntTimer1( T1_INT_PRIOR_7 & T1_INT_ON ) ;		// Timer1 �����_�u�����ų] 7 (�̰�)
															// Timer1 �����_ ON 

		OpenTimer1( T1_ON & T1_IDLE_STOP & T1_GATE_OFF & 	// Timer1 �� Period �]���C 100 ms 
					T1_PS_1_64 & T1_SYNC_EXT_OFF & T1_SOURCE_INT ,
						(long)((FCY/ 1000)* 100 )/64 ) ;
}
/*************************************************/
// QEI�Ҳժ���l�ưƵ{��
//
/*************************************************/
void Init_QEI(void)
{
	
	ConfigIntQEI(QEI_INT_DISABLE&QEI_INT_PRI_0) ;

	OpenQEI( 	QEI_MODE_x4_MATCH & QEI_IDLE_CON &				// �]�w��4���Ҧ��A���m���~��ާ@
				QEI_INPUTS_NOSWAP & QEI_NORMAL_IO &				// �O���w�]�}��A���`��X�J
		     	QEI_INDEX_RESET_DISABLE & QEI_DIR_SEL_CNTRL ,	// �������Э��m�\��A��V��ܱ���]�w
			 	QEI_QE_CLK_DIVIDE_1_4 & QEI_QE_OUT_DISABLE ) ;
			 
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
