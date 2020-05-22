
void		OpenLCD (void) ;
void		WriteCmdLCD ( unsigned char ) ;
void		WriteDataLCD( unsigned char ) ;
void 		putsLCD( char * ) ;
void		putrsLCD( const char * ) ;
void		putcLCD( unsigned char ) ;
void		puthexLCD( unsigned char ) ;
void		setcurLCD( unsigned char , unsigned char ) ;	// ( X,Y )
void		LCD_CMD_W_Timing( void ) ;
void		LCD_L_Delay( void ) ;
void		LCD_S_Delay( void ) ;
void		LCD_DAT_W_Timing ( void ) ;
 
