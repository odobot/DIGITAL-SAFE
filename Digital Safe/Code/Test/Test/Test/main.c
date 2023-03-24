#define F_CPU 8000000UL
#include <stdio.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/eeprom.h>
#include <util/delay.h>
#include "LCD.h"
#include "ADC.h"
#include <string.h>


#define KEY_DDR     DDRB
#define KEY_PIN     PINB
#define KEY_PORT    PORTB

#define ROW_MASK    0x0F
#define COL_MASK    0xF0

volatile uint8_t key_pressed = 0;
volatile uint8_t key_code = 0;

// Define Global Variables
int input_password[4];
int stored_password[4] = {1, 2, 3, 4}; // to be changed to eeprom
int count = 0;
void check_password(void);
void show_password(void);




int main(void)
{	
	LCD_Init();
	LCD_String("Enter Password: ");
	LCD_Cmd(0xC0);
	
	/* Interrupt setup */
	GICR = 1<<INT0; /* Enable INT0*/
	MCUCR = 1<<ISC01 | 1<<ISC00; /* Trigger INT0 on rising edge */
	sei(); /* Enable Global Interrupt */


	while (1)
	{
		

		// TODO: Add other main loop code
	}
}



uint8_t read_keypad(void)
{
	// Wait for the data to be available
	while (!(PIND & (1 << PD2)));

	// Read the data from the keypad
	int value = PINB;
	uint8_t data = 255; // initialize to a default value
	switch (value)
	{
		case 0:
		data = 7;
		LCD_String("*");
		break;
		case 1:
		data = 8;
		LCD_String("*");
		break;
		case 2:
		data = 9;
		LCD_String("*");
		break;
		case 3:
		LCD_String("/");
		break;
		case 4:
		data = 4;
		LCD_String("*");
		break;
		case 5:
		data = 5;
		LCD_String("*");
		break;
		case 6:
		data = 6;
		LCD_String("*");
		break;
		case 7:
		//LCD_String("*");
		//data = 200; // A generic value to indicate that the key is *
		show_password();
		break;
		case 8:
		data = 1;
		LCD_String("*");
		break;
		case 9:
		data = 2;
		LCD_String("*");
		break;
		case 10:
		data = 3;
		LCD_String("*");
		break;
		case 11:
		LCD_String("-");
		break;
		case 12: // ON/C
		LCD_Clear();
		LCD_String("Enter Password: ");
		LCD_Cmd(0xC0);
		memset(input_password, 0, sizeof(input_password)); // clear out the array
		break;
		case 13:
		data = 0;
		LCD_String("*");
		break;
		case 14:
		//LCD_String("=");
		// data = 201; // A generic value to indicate that the key is =
		check_password();
		break;
		case 15:
		LCD_String("+");
		break;
	}

	// Debounce delay
	_delay_ms(50);

	return data;
}

// Checking password
void check_password(void)
{
	int i;
	for (i = 0; i < 4; i++)
	{
		if (input_password[i] != stored_password[i])
		{
			LCD_Cmd(0x01);
			LCD_String("Wrong Password");
			_delay_ms(1000);
			LCD_Cmd(0x01);
			LCD_String("Enter Password: ");
			LCD_Cmd(0xC0);
			count = 0;
			break;
		}
	}
	if (i == 3)
	{
		LCD_Cmd(0x01);
		LCD_String("Correct Password");
		_delay_ms(1000);
		LCD_Cmd(0x01);
		LCD_String("Enter Password: ");
		LCD_Cmd(0xC0);
		count = 0;
	}
}

// Showing password
void show_password(void){
	int i = count;
	char num[10];
	//int arr_size = sizeof(input_password) / sizeof(int);
	for (i = 0; i < count; i++){
		if (count == 0){
			memset(num, 0, sizeof(num));
		}
		else{
		sprintf(num+strlen(num), "%d", input_password[i]);
		}
	}
	LCD_Clear();
	LCD_String("Enter Password: ");
	LCD_Cmd(0xC0);
	LCD_String(num);
}


ISR(INT0_vect){
	// return the code of the key pressed and only get 4 digits
	key_code = read_keypad();
	

	if (key_code != 255){
		
		input_password[count] = key_code;
		
		count ++;
		if (count == 4){
			// display the password entered
			count = 0;
			memset(input_password, 0, sizeof(input_password)); // clear out the array
		}
	}
}