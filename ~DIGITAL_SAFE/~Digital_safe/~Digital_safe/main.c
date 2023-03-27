/*
 * ~Digital_safe.c
 *
 * Created: 3/27/2023 5:31:35 PM
 * Author : Administrator
 */ 

#define F_CPU 8000000UL
#define length 4
#include <avr/io.h>
#include <util/delay.h>
#include <stdlib.h>
#include <string.h>
#include <avr/interrupt.h>
#include <stdbool.h>
#include "ADC.h"
#include "Read_Keypad.h"
#include "LCD.h"

int input_password[4] = {-1, -1, -1, -1};
int stored_password[4] = {1, 2, 3, 4}; // to be changed to eeprom
int count = 0;
//int check_password(void);
//void show_password(void);
int password_entered = 0; // flag variable to indicate if password has been entered
//void BIRG(int status);
//int safe_open = 0; // flag variable to indicate if safe is open 0 - locked, 1 - open
volatile uint8_t latch_state = 0; // flag variable to indicate if button is pressed 0 - not pressed, 1 - pressed

//uint8_t read_keypad(void); // function to read the keypad
//void activate_buzzer(void);
int input[length];
int password[length] = {1,2,3,4};

int counter = 0;
short j = 0;

volatile bool inputs_full = false;
volatile bool safe_open = false;
volatile bool unlock=true;
volatile bool write_star = false;
volatile bool enter_current_password = false;
volatile bool change_code = true;

void delay_led(){
	LCD_Clear();
	_delay_ms(500);
	LCD_String("Theft!");
	LCD_Cmd(0xC0);
	LCD_String("Alert!");
	_delay_ms(500);
	LCD_Clear();
	LCD_String("Theft!");
	LCD_Cmd(0xC0);
	LCD_String("Alert!");
}

void potentiometer_read()
{
	ADC_Init();
	DDRA |= (1<<PA7);
	int half_voltage = 1023/2;

	if (ADC_Read(0) < half_voltage)
	{
		while (1){
			delay_led();
			PORTA =PORTA | (1<<PA7);
			delay_led();
			PORTA = PORTA & (0b00000000);
			
			// Check port
			char value = PINB & (0x0F);
			unsigned char keycheck = check_Keypad(value);
			
			if (keycheck == '+')
			{
				reset();
				break;
			}
		}
	}
}


void home(){
	LCD_Clear();
	LCD_String("Enter Code");
	LCD_Cmd(0xc0);
	LCD_String("_ _ _ _");
};



void system_init(){
	sei();
	home();
	keypad_Init();
	LCD_Init();
	ADC_Init();
	timer_setup();
}


void check_password()
{
	int i;
	if (input_password[4] != stored_password[i])
	{
		LCD_Clear();
		LCD_String("WRONG PASSWORD");
	}
	else
	{
		LCD_String("Correct password");
	}
}

void reset(){
	system_init();
	inputs_full = false;
	counter = 0;
	write_star = false;
	enter_current_password = false;
	safe_open = false;
	unlock = true;
	change_code = true;
	
	//LED BIRG
	DDRD = DDRD | (1<<PD6);
	DDRD = DDRD | (1<<PD7);
	
	// Light the LED-BIRG RED by default
	PORTD = PORTD | (0<<PD6);
	PORTD = PORTD | (1<<PD7);
	
	//LATCH PUSHBUTTON AS INPUT AND ENABLING PULL UP
	DDRD = DDRD & (~(1<<PD4));
	PORTD =   PORTD | (1<<PD4);
	
	LCD_Clear();
	home();
}

/*ISR function: run whenever there is a new key press from the MMC74C922*/
ISR(INT0_vect){
		if (inputs_full == false || enter_current_password)    // HAIWORK
		{
		
			// Read the value from the keypad connected pins
			char value = PINB & (0x0F);
			unsigned char keycheck = check_Keypad(value);
		
			if (keycheck == '+') // YA KUCANCEL
			{
				reset();
				counter = 5;
			}
				switch(counter){				
					case 0:
					LCD_Cmd(0xC0);
					input[counter] = keycheck;
					LCD_String("*");
					counter++;
					break;
				
					case 1:
					LCD_Cmd(0xC2);
					input[counter] = keycheck;
					LCD_String("*");
					counter++;
					break;
				
					case 2:
					LCD_Cmd(0xC4);
					input[counter] = keycheck;
					LCD_String("*");
					counter++;
					break;
				
					case 3:
					LCD_Cmd(0xC6);
					input[counter] = keycheck;
					LCD_String("*");
					counter++;
					break;
				
					case 4:
					if (keycheck == '*')
					{
						LCD_Cmd(0xC0);
						LCD_Char(input[0]);
						LCD_Cmd(0xC2);
						LCD_Char(input[1]);
						LCD_Cmd(0xC4);
						LCD_Char(input[2]);
						LCD_Cmd(0xC6);
						LCD_Char(input[3]);
						counter=4;
						break;
					}
				
				
					if (keycheck == '=')
					{
						int i;
						input_password[4] = input[0], input[1], input[2], input[3];
						check_password();
						/*if (inputs_full == false)
						{
							inputs_full = true;
							LCD_Clear();
							LCD_String("VERYFYING PASS...");
							_delay_ms(1000);
							counter++;
							break;
						}
						if (enter_current_password)
						{
							LCD_Clear();
							counter++;
							break;
						}*/
						
					}
				
				
					default:
					reset();
					break;
			}
		}
}

ISR(TIMER1_OVF_vect){
	potentiometer_read();
	TCNT1 = 3036;
}


void check_latch(){
	int latch_button = ~PIND & (1<<PD4);
	if (latch_button)
	{
		while (1){
					volatile bool out = false;
					safe_open = true;	
					// Light green LED
					PORTD = PORTD | (1<<PD6);
					PORTD = PORTD | (0<<PD7);
		
					//Display LCD	
					LCD_Clear();
					LCD_String("Safe: OPEN");
					LCD_Cmd(0xc0);
					LCD_String("_ _ _ _");
					_delay_ms(2);		
					// Check press in order to reset the safe
					char value = PINB & (0x0F);
					unsigned char keycheck = check_Keypad(value);
					
					if (keycheck == '+' )
					{
						break;
					}
		
					if (keycheck == 'c' && !out) 
					{
						LCD_Clear();
						LCD_String("RESETTING CODE:");
						_delay_ms(1000);
						LCD_Clear();
						while (1){
							enter_current_password = true;
							
							counter = 0;
							
// 							// Read the value from the keypad connected pins
// 							char value = PINB & (0x0F);
// 							unsigned char keycheck = check_Keypad(value);
// 							if (/*keycheck != '=' &&*/ (counter != 4))
// 							{
								LCD_Clear();
								LCD_String("Current Code:");
								LCD_Cmd(0xc0);
								LCD_String("_ _ _ _");	
								_delay_ms(5000);
/*							}*/
							
							int i = 0;
							while(i != length){
								if(input[i] != password[i])
								{
									change_code = true;																//should be false
								}
								i++;
							}
							if (change_code)
							{
								counter = 0;
								
// 								// Read the value from the keypad connected pins
// 								char value = PINB & (0x0F);
// 								unsigned char keycheck = check_Keypad(value);
// 								if (/*keycheck != '=' &&*/ (counter != 4))
// 								{
									LCD_Clear();
									LCD_String("New Code:");
									LCD_Cmd(0xc0);
									LCD_String("_ _ _ _");
									_delay_ms(5000);
/*								}	*/
								for (int j = 0; j<length; j++)
								{
									password[j] = input[i];
								}
								LCD_Clear();
								LCD_String("CHANGED");
								LCD_Cmd(0xc0);
								LCD_String("SUCCESSFULLY");
								_delay_ms(2000);
								out = true;
								break;
							}
							else{
								out = true;
								break;
							}
							
						}
						/*
			
						QUESTION 3, HAPO KWA IF YOU PRESS C
			
						*/
					}
		}
	}
}


int main(void)
{
    reset();
    while (1) 
    {
		if (inputs_full == true)
		{
				int i = 0;									
				while(i != length){
					if(input[i] != password[i])																				// SHOULD BE FALSE
					{
						unlock = true;
					}
					i++;
				}
					
				if ((unlock))//if to be granted access,the LED-BIRG blinks green 4 times.
				{
					for (int i=0;i<6;++i)
					{
							LCD_Clear();
							LCD_String("PRESS THE");
							LCD_Cmd(0xc0);
							LCD_String("LATCH BUTTON");
					
						PORTD = PORTD | (1<<PD6);
						PORTD = PORTD | (0<<PD7);
					
						check_latch();
						_delay_ms(1000);
						
					LCD_Clear();
					LCD_String("TO OPEN");
					LCD_Cmd(0xc6);
					LCD_String("THE SAFE");
						PORTD &= 0b00000000;
					
						check_latch();
						_delay_ms(1000);					
					}
					if (!(safe_open))
					{
						reset();
					}
				}
				else{
				reset();
				LCD_Clear();
				LCD_String("RESETTING");
				_delay_ms(1000);
				}
		}
    }
}
