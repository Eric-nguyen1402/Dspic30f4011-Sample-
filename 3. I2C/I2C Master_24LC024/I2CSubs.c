
#include 	<p30F4011.h>
#include	<I2C.h>
#include 	"I2CSubs.h"

#define		REPORT_NACK			{	\
									IFS0bits.MI2CIF = 0 ; \
									StopI2C( ) ; \
									while ( I2CCONbits.PEN ) ;	\
									return -1 ; \
								} 

unsigned char  I2C_ACKPolling(unsigned char CMD)
{
		unsigned char ACK_Result ;

		IdleI2C( ) ;
		StartI2C( ) ;

		while ( I2CCONbits.SEN) ;				// Send START bit ! SEN will be clear automatically once START bits completed 

		IFS0bits.MI2CIF = 0 ;
		MasterWriteI2C((CMD & 0xFE)) ;
		while(! IFS0bits.MI2CIF ) ;				// MI2CIF will be clear after data transmitted completely !

			if ( I2CSTATbits.ACKSTAT ) 		ACK_Result =  1 ;
			else							ACK_Result =  0 ;

		IFS0bits.MI2CIF = 0 ;
		StopI2C( ) ;
		while ( I2CCONbits.PEN ) ;
		IFS0bits.MI2CIF = 0 ;	

		return		ACK_Result ;
}

void	Init_I2C(void)
{
	unsigned int 	config1 , config2 ;

	config2 = 0xff;
	config1 =	 I2C_ON & I2C_IDLE_STOP&I2C_CLK_REL
				& I2C_IPMI_DIS & I2C_7BIT_ADD
				& I2C_SLW_DIS & I2C_SM_DIS 
				& I2C_GCALL_DIS & I2C_STR_DIS
				& I2C_NACK & I2C_ACK_DIS & I2C_RCV_DIS 
				& I2C_STOP_DIS & I2C_RESTART_DIS
				& I2C_START_DIS ;

				ConfigIntI2C(MI2C_INT_OFF & MI2C_INT_PRI_0 ) ;
				OpenI2C(config1,config2) ;

				 	TRISFbits.TRISF2 = 1 ;
				 	TRISFbits.TRISF3 = 1 ;
}

int		EEPROM_ByteWrite(unsigned char CMD , unsigned char Addr , unsigned char Data ) 
{
		IdleI2C( ) ;
		StartI2C( ) ;

		while ( I2CCONbits.SEN) ;				// Send START bit ! SEN will be clear automatically once START bits completed 

		IFS0bits.MI2CIF = 0 ;
		MasterWriteI2C((CMD & 0xFE)) ;
		while(! IFS0bits.MI2CIF ) ;				// MI2CIF will be clear after data transmitted completely !

			if ( I2CSTATbits.ACKSTAT ) 			
				REPORT_NACK


		IFS0bits.MI2CIF = 0 ;
		MasterWriteI2C(Addr) ;
		while(! IFS0bits.MI2CIF ) ;

			if ( I2CSTATbits.ACKSTAT ) 			
				REPORT_NACK

		IFS0bits.MI2CIF = 0 ;
		MasterWriteI2C(Data) ;
		while(! IFS0bits.MI2CIF ) ;

			if ( I2CSTATbits.ACKSTAT ) 			
				REPORT_NACK		


		IFS0bits.MI2CIF = 0 ;
		StopI2C( ) ;
		while ( I2CCONbits.PEN ) ;	
		IFS0bits.MI2CIF = 0 ;
				return 0 ;

}


int		EEPROM_ByteRead(unsigned char CMD , unsigned char Addr  )
{
		unsigned char Temp_Buffer ;

		unsigned		ReadDelay ;
		
		IdleI2C( ) ;
		StartI2C( ) ;
		while ( I2CCONbits.SEN) ;

		IFS0bits.MI2CIF = 0 ;
		MasterWriteI2C((CMD & 0xFE)) ;				// Mask bit 0 -> Write command 
		while(! IFS0bits.MI2CIF ) ;

			if ( I2CSTATbits.ACKSTAT ) 			
				REPORT_NACK


		IFS0bits.MI2CIF = 0 ;
		MasterWriteI2C(Addr) ;
		while(! IFS0bits.MI2CIF ) ;

			if ( I2CSTATbits.ACKSTAT ) 			
				REPORT_NACK


		IFS0bits.MI2CIF = 0 ;
		RestartI2C( ) ;
		while ( I2CCONbits.RSEN) ;

		IFS0bits.MI2CIF = 0 ;
		MasterWriteI2C((CMD | 0x01)) ;					// Set bit 0 for "Read" command
		while(! IFS0bits.MI2CIF ) ;

			if ( I2CSTATbits.ACKSTAT ) 			
				REPORT_NACK

		IFS0bits.MI2CIF = 0 ;

		Temp_Buffer  = MasterReadI2C( ) ;
		while(! IFS0bits.MI2CIF ) ;

		IFS0bits.MI2CIF = 0 ;
		NotAckI2C( ) ;
		while(! IFS0bits.MI2CIF ) ;
		IFS0bits.MI2CIF = 0 ;

		StopI2C( ) ;
		while ( I2CCONbits.PEN ) ;	

		return (int)Temp_Buffer ;

}


