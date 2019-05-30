#include "pic_specific.h"
#include "timer0.h"

void init_timer0(void)
{
	/*
	 * Setting instruction cycle clock (Fosc / 4) as the source of
	 * timer0
	 */
	T0CS = 0;
	T0CONbits.T08BIT = 1;
	T0CONbits.PSA = 0;
	T0PS2 = 0;
	T0PS1 = 0;
	T0PS0 = 1;
	TMR0 = 6;
	/* Clearing timer0 overflow interrupt flag bit */
	TMR0IF = 0;
	/* Enabling timer0 overflow interrupt */
	TMR0IE = 1;
	GIE = 1;
}
