// ***********************************************************************
//                Microchip FAE Training        Exercise 5
//
// Purpose : 練習如何使用 QEI 模組的兩個輸入 QEA & QEB 計算出轉速
//           並將轉速以間格 300mS 的時間顯示於 LCD 上
//
// ***********************************************************************

#include 	<p30fxxxx.h>
#include 	<timer.h>
#include	<qei.h>					
#include 	"C30EVM_LCD.h"			// 將LCD函式的原型宣告檔案含入 

#define		FCY 	7372800 * 2		// 因為使用頻率為將外部 7.3728 MHz * 8 的模式 , 每一指令週期需 4 個 clock
									// 所以 FCY =  (7.3728 * 8 / 4 ) MHz = 7372800* 2

//---------------------------------------------------------------------------
// 請參考此處有關 Configuration bits 於程式中直接宣告的方式
// 更詳細的參數列表請參考 p30F4011.h 
//---------------------------------------------------------------------------
   	_FOSC(CSW_FSCM_OFF & XT_PLL8);   //	XT with 8xPLL oscillator, Failsafe clock off
   	_FWDT(WDT_OFF);                  //	Watchdog timer disabled
  	_FBORPOR(PBOR_OFF & MCLR_EN);    //	Brown-out reset disabled, MCLR reset enabled
   	_FGS(CODE_PROT_OFF);             //	Code protect disabled

const char	My_String1[]="RPM Measurement" ;	// 宣告字串於 Program Memory
const char	My_String2[]=" RPM :         " ;	// 宣告字串於 Data Memory

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

	OpenLCD( ) ;											// 使用 OpenLCD( )對 LCD 模組作初始化設定
															// 4 bits Data mode
															// 5 * 7 Character 
	ADPCFG = 0xffff ;										// AN3 , AN4 , AN5 as difital for QEI !!

	QEI_Old = 0 ;
	QEI_New = 0 ;
	Init_QEI( ) ;
	Init_Timer1( );


		setcurLCD(0,0) ;									// 使用 setcurLCD( ) 設定游標於 (0,0)
		putrsLCD( My_String1 ) ;							// 將存在 Program Memory 的字串使用
															// putrsLCD( ) 印出至 LCD
			
		setcurLCD(0,1) ;
		putrsLCD( My_String2 ) ;							// 使用 setcurLCD( ) 設定游標於 (0,1)
															// 將存在 Data Memory 的字串使用	
															// putsLCD( ) 印出至 LCD
		OneSecond = 0 ;

		while(1) 
		{	
 			if ( QEI_Update )								// RPM calclation every 100mS
			{
				QEI_Update = 0 ;

				setcurLCD(8,1) ;

				if ( QEICONbits.UPDN )		putcLCD('+') ;		// 根據UPDN位元決定旋轉的方向
				else						putcLCD('-') ;
					
				
				if ( QEICONbits.UPDN )							// 馬達正轉時的計算
				{
					if ( QEI_New >= QEI_Old )					// 計算單位時間內的旋轉數量
						QEI_Diff = QEI_New - QEI_Old ;
					else										// 溢位時的處理
						QEI_Diff = QEI_New + ( 65536 - QEI_Old ) ;	
				}
				else											// 馬達反轉時的計算
				{
					if (QEI_New < QEI_Old )
						QEI_Diff = QEI_Old - QEI_New ;
					else
						QEI_Diff = ( 65536 - QEI_New )+ QEI_Old ; // 借位時的處理
				}
				
				QEI_Old = QEI_New ;					// 更新的QEI數值
				
				RPM = ((long)QEI_Diff*600/256);		// 轉速的計算 (100mS * 600= 1 Minute, Index=64 採4倍精度 = 256)


				if (LCD_Update_Count==0)
				{
					uitoa ( Convert_Buf , RPM ) ;		// 轉換成十進制後再轉成 ASCII Code

					setcurLCD(10,1) ;					// 顯示轉速在 LCD 
					putrsLCD( "     " ) ;

					setcurLCD(10,1) ;
					putsLCD( Convert_Buf ) ;

					LCD_Update_Count=3;					// Set LCD update time for 300mS
				}

			}
	
		}		
}

/*************************************************/
// Timer1 模組的初始化副程式
// 100mS Interrupt setting
/*************************************************/
void Init_Timer1(void)
{
		ConfigIntTimer1( T1_INT_PRIOR_7 & T1_INT_ON ) ;		// Timer1 的中斷優先等級設 7 (最高)
															// Timer1 的中斷 ON 

		OpenTimer1( T1_ON & T1_IDLE_STOP & T1_GATE_OFF & 	// Timer1 的 Period 設為每 100 ms 
					T1_PS_1_64 & T1_SYNC_EXT_OFF & T1_SOURCE_INT ,
						(long)((FCY/ 1000)* 100 )/64 ) ;
}
/*************************************************/
// QEI模組的初始化副程式
//
/*************************************************/
void Init_QEI(void)
{
	
	ConfigIntQEI(QEI_INT_DISABLE&QEI_INT_PRI_0) ;

	OpenQEI( 	QEI_MODE_x4_MATCH & QEI_IDLE_CON &				// 設定為4倍模式，閒置時繼續操作
				QEI_INPUTS_NOSWAP & QEI_NORMAL_IO &				// 保持預設腳位，正常輸出入
		     	QEI_INDEX_RESET_DISABLE & QEI_DIR_SEL_CNTRL ,	// 關閉指標重置功能，方向選擇控制設定
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
