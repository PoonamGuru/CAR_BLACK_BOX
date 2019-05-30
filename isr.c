#include "pic_specific.h"
extern bit flag;
unsigned short int count_5sec;
void interrupt isr(void)
{


	if (TMR0IF)
	{
		TMR0 = TMR0 + 8;
		//TMR0 = 8;

		if (count_5sec++ == 25000)
		{
			count_5sec= 0;
			flag = 1;
//			TMR0ON = 0;
		}

		TMR0IF = 0;
	}
}

