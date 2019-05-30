#include "pic_specific.h"
#include "main.h"
#include "clcd.h"
#include "ds1307.h"
#include "i2c.h"
#include "matrix_keypad.h"
#include "timer0.h"
#include"adc.h"
#include<string.h>
unsigned char clock_reg[3];
unsigned char calender_reg[4];
unsigned char time[9];
unsigned char date[11];
unsigned char key;
/*password data */
unsigned char password[5] = {'\0'};
unsigned char stored_password[5] = "1234"; 

/*timer data*/
bit flag = 0;
bit flag_1 = 0;
extern unsigned short count_5sec;

/*function declaration*/
char enter_password(void);
char check_password(void);
void change_password();


/*delay*/
void delay(unsigned short delay)
{
    unsigned short  i ,j;

    for(i = 0; i < delay; i++)
	for(j = 0; j < i ; j++);
}


/*dispaly time*/
void display_time(void)
{
    clcd_print(time, LINE2(0));

    if (clock_reg[0] & 0x40)
    {
	if (clock_reg[0] & 0x20)
	{
	    clcd_print("PM", LINE2(12));
	}
	else
	{
	    clcd_print("AM", LINE2(12));
	}
    }
}

static void get_time(void)
{
    clock_reg[0] = read_ds1307(HOUR_ADDR);
    clock_reg[1] = read_ds1307(MIN_ADDR);
    clock_reg[2] = read_ds1307(SEC_ADDR);

    if (clock_reg[0] & 0x40)
    {
	time[0] = '0' + ((clock_reg[0] >> 4) & 0x01);
	time[1] = '0' + (clock_reg[0] & 0x0F);
    }
    else
    {
	time[0] = '0' + ((clock_reg[0] >> 4) & 0x03);
	time[1] = '0' + (clock_reg[0] & 0x0F);
    }
    time[2] = ':';
    time[3] = '0' + ((clock_reg[1] >> 4) & 0x0F);
    time[4] = '0' + (clock_reg[1] & 0x0F);
    time[5] = ':';
    time[6] = '0' + ((clock_reg[2] >> 4) & 0x0F);
    time[7] = '0' + (clock_reg[2] & 0x0F);
    time[8] = '\0';
}

void default_screen(void)
{


    unsigned int adc_reg_val;
    char buff[5]; 
    unsigned char i;


    clcd_print("TIME     EN  SP", LINE1(0));
    adc_reg_val = read_adc(CHANNEL4);
    i = 3;
    do
    {
	buff[i] = (adc_reg_val % 10) + '0';
	adc_reg_val = adc_reg_val / 10;
    } while (i--);
    buff[4] = '\0';

    get_time();
    clcd_print(time , LINE2(0));
    clcd_print("ON" , LINE2(9));
    clcd_print(buff, LINE2(12));
}

unsigned char check_matrix_keypad(void)
{
    unsigned char key, once = 1;
    static unsigned char i = 0;
    unsigned int j;

    key = read_switches(STATE_CHANGE);
    for (j = 50000; j--; );      //to remove debouncing
    if (key != ALL_RELEASED && once)
    {
	once = 0;
	return key;

    }
    else if(key == 0xFF)
    {
	once = 1;
	return key;
    }

    return 0xff;

}

void init_config(void)
{
    TRISB0 = 0;
    RB0 = 0;
    init_clcd();
    init_i2c();
    init_ds1307();
    init_adc();
    init_timer0();
    init_matrix_keypad();


}
/*-----------------------------------------------------------------------------------------------------------------------------*/
/* function  : change_password
 * description : change the password
 ------------------------------------------------------------------------------------------------------------------------------*/
void change_password(void)
{
    unsigned char cout = 2, check;

    unsigned char temp[5] = {'\0'};

    strcpy(temp ,stored_password);
    while(1)
     {
        CLEAR_DISP_SCREEN;
        clcd_print("  Enter password" , LINE1(0));
	while(cout != 0)
	{
	    if(enter_password() == 0) //time out return 0
	    {
		CLEAR_DISP_SCREEN;
		strcpy(stored_password , temp);
		return;
	    }

	    clcd_print("Re Enter again  : " , LINE1(0));
	    if(cout == 2)
		strcpy(stored_password , password);
//	    clcd_print(password, LINE1(0));
            cout--;
	}


	/*if user entered 2 times check previous and 2nd time entered data is same or not*/
	if(cout == 0)
	{
	    cout = 2;
	    check = check_password( );

	    if(check == 0)  //no chance left
	    {
		//clcd_print("0", LINE1(0));
		strcpy(stored_password , temp);
		break;
	    }
	    else if(check == 1) //chance left
	    {
		//CLEAR_DISP_SCREEN;
		//clcd_print("1", LINE1(0));
		//delay(1000);
		continue;
	    }
	    else if(check == 2) //matched
	    {
		//clcd_print("2", LINE1(0));
		strcpy(stored_password,password);
		CLEAR_DISP_SCREEN;
		clcd_print("password changed",LINE1(0));
		delay(1000);
		break;
	    }
	}

    }

}


char enter_password(void)
{
    unsigned char i;

    //CLEAR_DISP_SCREEN;
    if(flag_1)
    clcd_print("Enter Password :" , LINE1(0));

    clcd_write(0x0F , INSTRUCTION_COMMAND);  //blink the curser

    clcd_write(0xC0 , INSTRUCTION_COMMAND);  //blink the curser
    
    for( i = 0; i < 4; i++)
    {

	count_5sec = 0;
	TMR0ON = 1;

	while((key = check_matrix_keypad()) == 0xFF)
	{
	    if(flag)
	    {
		//  CLEAR_DISP_SCREEN;
		flag = 0;
		TMR0ON = 0;
		clcd_write(0x0C,INSTRUCTION_COMMAND);
			//	default_screen();
		return 0;
	    }
	}

	TMR0ON = 0;


	clcd_putch('*' , LINE2(i));
	password[i] = key % 10 + '0';
    }

    CLEAR_DISP_SCREEN;
    //remove the blinking
    clcd_write(0x0C,INSTRUCTION_COMMAND);

    return 1;
}

char check_password(void)
{
    static    unsigned char count = '3';
    if(strcmp(stored_password , password) == 0)
    {
	clcd_print("Password matched" , LINE1(0));
	delay(1000);
	count = '3';
	return 2;  //password matched
    }
    else
    {         

	CLEAR_DISP_SCREEN;
	clcd_print("Password not", LINE1(2));
	clcd_print("matched" , LINE2(4));
	delay(1000);


	CLEAR_DISP_SCREEN;
	delay(750);
	count--;
	clcd_putch(count , LINE1(0));
	clcd_print(" chance is" , LINE1(1));
	clcd_print(" remaining" , LINE2(3));
	delay(1000);


    }


    /*--------------------------------------------when no try is remaining--------------------------*/
    if(count == '0')
    {
	CLEAR_DISP_SCREEN;
	clcd_print("all chances" , LINE1(0));
	clcd_print("over", LINE2(3));
	delay(1000);

	/*wait for 1 min */
	for(short i = 0 ; i < 500; i ++)
	{


	    if((key = check_matrix_keypad()) == 0xFF)
	    {
		if(flag_1)
		{    
		CLEAR_DISP_SCREEN;
		default_screen();
		}
	    }
	    else
	    {
		CLEAR_DISP_SCREEN;
		clcd_print("wait for some" , LINE1(0));
		clcd_print("time",LINE2(5));
		delay(1000);
	    }
	}


	count = '3';
	return 0; //3 chance over
    }

    return 1;

}
/*rtc write*/
void RTC_write(void)
{
    signed    char i, j;
    unsigned char period[3];

    j = 0;
    /* convert 16 bits to 8 bit hexa*/
    for(i = 7; i >= 0; i -= 3)
    {
//	CLEAR_DISP_SCREEN;
//	clcd_print("done",LINE1(0));
//	delay(100);
//	clcd_putch( i + '0' , LINE1(0));
//	delay(1000);

	period[j] = ((time[i-1] - 48) << 4) | (time[i] - 48);
	j++;
    }



    write_ds1307(SEC_ADDR, period[0]);
    write_ds1307(MIN_ADDR,  period[1]);
    write_ds1307(HOUR_ADDR,  period[2]);

}
/*set the mini,sec,hr filed*/
void set_field(char i)
{
    char arr[3];
    while((key = check_matrix_keypad()))
    {
	clcd_putch(time[i-1] , LINE2(i -1));
	clcd_putch(time[i] , LINE2(i));


	if(key == 11)
	{
	    time[i]++;
	    /*minute and sec field*/
	    if(i == 7 || i == 4)
	    {
		if(time[i - 1] == '5' && time[i] == ':')
		{
		    time[i] = '0';
		    time[i - 1] = '0';
		}
	    }

	    /*hour field*/
	    if( i == 1)
	    {
		if(time[i - 1] == '2' && time[i] == '4')
		{
		    time[i] = '0';
		    time[i-1] = '0';
		}
	    }

	    if(time[i] == ':')
	    {
		time[i  - 1]++;
		time[i] = '0';
	    }


	}


	if(key == 10)
	{
	    //  CLEAR_DISP_SCREEN;
	    //clcd_print("done with edit" , LINE1(0));
	    // delay(10000);
	    return;
	}
    }

}
/*set the time*/
void set_time(void)
{
    signed char i = 7;
    CLEAR_DISP_SCREEN;
    clcd_print("HH:MM:SS",LINE1(0));
    /*get the current timing*/
    get_time();
    //clcd_print("Time", LINE1(0));
    clcd_print(time , LINE2(0));
    //delay(1000);

    while((key = check_matrix_keypad()))
    {

	if(key == 11)// write to rtc
	{
	    //CLEAR_DISP_SCREEN;
	   // clcd_print("done with ", LINE1(0));
	  //  clcd_print("Enterning", LINE2(0));
	    delay(1000);
	    RTC_write();
	    get_time();
	   // CLEAR_DISP_SCREEN;
	   // clcd_print(time, LINE1(0));
	   // delay(1000);
	    break;

	}

	if(key == 10) //allow to edit the field
	{
	   // CLEAR_DISP_SCREEN;
	    clcd_print(time , LINE2(0));
	    set_field(i);
	}

	if(key == 12) //select the filed

	{
	    clcd_putch(time[i] , LINE2(i));
	    clcd_putch(time[i-1] , LINE2(i-1));
	    i -= 2 ;
	}


	if(i != -1 && time[i] != ':') //blink the filed
	{
	    clcd_putch(time[i] , LINE2(i));

	    clcd_putch(time[i - 1] , LINE2(i -1));

	    delay(350);
	    clcd_putch(' '  , LINE2(i));
	    clcd_putch(' '  , LINE2(i - 1));
	}

	else if(i == -1) //roll back
	    i = 7;

	else if(time[i] == ':') //skip
	{
	    i--;
	    continue;

	}

    }

    return;


}


/* ----------------------------------------------------------------------------------------------------------------------------------

   function name : dispaly_menu
description   : show the menu of black box 

----------------------------------------------------------------------------------------------------------------------------------*/
int display_menu(void)
{
    char *menu[6] = {"  View Log" , "  Clear Log","  Change PW" , "  Download Log" , "  Set Time" , ""} ;
    char i = -1;
    char temp[20];
    char count = 1;


    CLEAR_DISP_SCREEN;
    clcd_print("## MAIN_MENU ##" , LINE1(0));
    delay(2000);
    CLEAR_DISP_SCREEN;
    while(1)
    {


	if(i ==  (char) -1)
	{
	    //	    CLEAR_DISP_SCREEN;

	    clcd_print(menu[0] , LINE1(1));
	    clcd_print(menu[1], LINE2(1));
	}






         count_5sec = 0;
         TMR0ON = 1;
 
         while((key = check_matrix_keypad()) == 0xFF)
         {
             if(flag)
             {
                 //  CLEAR_DISP_SCREEN;
                 flag = 0;
                 TMR0ON = 0;
                 clcd_write(0x0C,INSTRUCTION_COMMAND);
                 return -1;
             }
         }
 
         TMR0ON = 0;



	/*-------------------------navigation through menu------------------------------------------*/
	if(key == 12)
	    i++;

	if(key == 11)
	    return; //exit
	/*------------------------------------------select the menu-------------------------------*/
	if(key == 10)
	{
	    switch(i)
	    {
		case 4:
		    set_time();
		    break;
		case 2:
		    change_password();
		    break;

	    }
	}



	if(i == 5)
	{
	    CLEAR_DISP_SCREEN;
	    i = -1;
	}

	else
	{	    
	    if( i % 2 == 0 || i == 0)
	    {
		CLEAR_DISP_SCREEN;
		clcd_print(menu[i] , LINE1(2));
		clcd_putch('*' , LINE1(0));
		clcd_putch(' ' , LINE2(0));
		clcd_print(menu[i+1], LINE2(2));
	    }

	    if(i % 2 == 1)
	    {

		clcd_putch(' ' , LINE1(0));
		if(i != 5)
		    clcd_putch('*' , LINE2(0));
	    }
	}
#if 0
	/*if non of the key is pressed exit*/
	if(key == 0xFF)
	{
	    if(flag)
	    {
		CLEAR_DISP_SCREEN;
		flag = 0;
		TMR0ON = 0;
		clcd_print("time up" , LINE1(0));
		//clcd_write(0x0C,INSTRUCTION_COMMAND);
		//default_screen();
		return -1;
	    }
	}

	TMR0ON = 0;
#endif
    }

    return 0;

}
void main(void)
{
    unsigned char key = 0;
    char check;
    init_config();

    while (1)
    {


	CLEAR_DISP_SCREEN;
	delay(1000);

	while((key = check_matrix_keypad()) == 0xFF)
	    default_screen();

	if(key == 10) //enter into the password enter
	{
	    while(1)	   
	    {
		CLEAR_DISP_SCREEN;
		flag_1 = 1;
		if(enter_password() != 1)
		    break;
		check = check_password();
		if(check == 1) //chances remaining
		    continue;
		else if(check == 0)  //0 chances 
		    break;
		else if(check == 2)
		{
		    CLEAR_DISP_SCREEN;

		   flag_1 = 0;

			if(display_menu() == -1)  //dispaly the menu

		                break;

		}
	    }

	}



    }
}
