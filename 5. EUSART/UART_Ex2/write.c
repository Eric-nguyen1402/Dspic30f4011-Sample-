#include <stdio.h>
#include "simio.h"
/*----------------------------------------------------------------------*/
/* Special function registers						*/
/*----------------------------------------------------------------------*/
extern void	__sfr__(int address);	/* SFR address attribute	*/
extern volatile int  __attribute__((__sfr__(0x216))) U2MOD;
extern volatile int  __attribute__((__sfr__(0x218))) U2STA;
extern volatile char __attribute__((__sfr__(0x21A))) TX2REG;
extern volatile char __attribute__((__sfr__(0x21C))) RX2REG;
extern volatile int  __attribute__((__sfr__(0x21E))) BRG2;
#define	SPEN	0x8000u
#define	TXEN	0x0400u
#define	TRMT	0x0100u
#define	TXBF	0x0200u
#define	RCDA	0x0001u
/*----------------------------------------------------------------------*/

static void __inline__
dowrite(PSIMIO psimio)
{
  	__asm__(".pword 0xDAC000" :: "r" (psimio));
}
int __attribute__((__weak__))
write(int handle, void *buffer, unsigned int len)
{
	int i;
	SIMIO simio;

	switch (handle)
	{
	case 0:
	case 1:
	case 2:
		if ((U2MOD & SPEN) == 0)
		{
			BRG2 = 0;
			U2MOD |= SPEN;
		}
		if ((U2STA & TXEN) == 0)
		{
			U2STA |= TXEN;
		}
		for (i = len; i; --i)
		{
			while (U2STA & TXBF)
				;
			TX2REG = *(char*)buffer++;
		}
		break;
	default:
		simio.method = SIM_WRITE;
		simio.u.write.handle = handle;
		simio.u.write.buffer = buffer;
		simio.u.write.len = len;
		dowrite(&simio);

		len = simio.u.write.len;
		break;
	}
	return(len);
}

