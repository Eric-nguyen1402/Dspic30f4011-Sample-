// ***********************************************************************
// File : 		I2CSIM_Main.C

// ***********************************************************************

#include 	<p30F4011.h>
#include 	"C30EVM_LCD.h"			// 將LCD函式的原型宣告檔案含入 
#include	<timer.h>				// 將Timer函式的原型宣告檔案含入
#include	<adc10.h>				// 將adc10函式的原型宣告檔案含入
#include	"ADCSubs.h"
#include	"I2CSubs.h"

#define		I2C_DEVICE_ADDR	0xA0

#define		FCY 	7372800 * 2		// 因為使用頻率為將外部 7.3728 MHz * 8 的模式 , 每一指令週期需 4 個 clock
									// 所以 FCY =  (7.3728 * 8 / 4 ) MHz = 7372800* 2

   	_FOSC(CSW_FSCM_OFF & XT_PLL8);   //	XT with 8xPLL oscillator, Failsafe clock off
   	_FWDT(WDT_OFF);                  //	Watchdog timer disabled
  	_FBORPOR(PBOR_OFF & MCLR_EN);    //	Brown-out reset disabled, MCLR reset enabled
   	_FGS(CODE_PROT_OFF);             //	Code protect disabled

const char	My_String1[]="VR1:    VR2:    " ;	// 宣告字串於 Program Memory (因為 const 宣告)
	  char	My_String2[]="EE1:    EE2:    " ;	// 宣告字串於 Data Memory

unsigned int miliSec ;

void	Init_ADC(void) ;
void	Show_ADC(void) ;

// union 宣告將使8位元變數ByteAccess與SystemFlag結構變數使用相同的記憶體，
// 以利不同格式的位元運算需求
union 	{
			unsigned char ByteAccess ;
			struct 	{
						unsigned Bit0: 1 ;
						unsigned Bit1: 1 ;
						unsigned Bit2: 1 ;
						unsigned unused : 5 ;
					} ;
		} SystemFlag ;

//定義OneSecond旗標等同於SystemFlag.Bit0位元變數，故將其使用一個位元記憶空間
#define	OneSecond	SystemFlag.Bit0 	

void _ISR _T1Interrupt(void)		//Timer1中斷副程式
{

	miliSec += 1 ;

	if (miliSec == 1000)			//每1000次將OneSecond旗標設定為1
		{
			OneSecond = 1 ;
			miliSec  = 0 ;
		} 
	IFS0bits.T1IF = 0 ;				//清除中斷旗標

}


	unsigned int 	ADC_Value;
	unsigned char 	EEPROM_Data1 , EEPROM_Data2 ;
	int				WaitLoop ;

int	main( void )
{


	Init_ADC( ) ;										// 將ADC進行初始化設定
	Init_I2C( ) ;

	OpenLCD( ) ;										// 使用 OpenLCD( )對 LCD 模組作初始化設定
														// 4 bits Data mode
														// 5 * 7 Character 

	setcurLCD(0,0) ;									// 使用 setcurLCD( ) 設定游標於 (0,0)
	putrsLCD( My_String1 ) ;							// 將存在 Program Memory 的字串使用
														// putrsLCD( ) 印出至 LCD
			
	setcurLCD(0,1) ;									// 使用 setcurLCD( ) 設定游標於 (0,1)
	putrsLCD( My_String2 ) ;							// 將存在 Data Memory 的字串使用
														// putsLCD( ) 印出至 LCD

	ConfigIntTimer1( T1_INT_PRIOR_7 & T1_INT_ON ) ;		// Timer1 的中斷優先等級設 7 (最高)
														// Timer1 的中斷 ON 

	OpenTimer1( T1_ON & T1_IDLE_STOP & T1_GATE_OFF & 			// Timer1 的 Period 設為每 1ms 
				T1_PS_1_1 & T1_SYNC_EXT_OFF & T1_SOURCE_INT ,
				(FCY/ 1000) ) ;

	OneSecond = 0 ;

	while(1) 
	{	
		if ( OneSecond ) 								// 詢問 Timer1 的 Period 時間是否已到
														// 可以用軟體模擬來檢查是否為準確的 1 ms
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
			put_Num_LCD( ADC_Value >> 2 ) ;				// 將類比轉換結果以十進位數字顯示至液晶顯示器

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
			put_Num_LCD( ADC_Value >> 2 ) ;				// 將類比轉換結果以十進位數字顯示至液晶顯示器	

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
			put_Num_LCD( EEPROM_Data1 ) ;				// 將類比轉換結果以十進位數字顯示至液晶顯示器
			setcurLCD(12,1) ;							// Set LCD cursor		
			put_Num_LCD( EEPROM_Data2 ) ;				// 將類比轉換結果以十進位數字顯示至液晶顯示器

		}
	}		
}


