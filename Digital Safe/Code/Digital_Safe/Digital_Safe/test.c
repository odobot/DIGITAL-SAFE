
#define F_CPU 8000000UL
#include <stdio.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/eeprom.h>
#include <util/delay.h>
#include "LCD.h"
#include "ADC.h"


/*
1. ON Standby display Welcome on line 1 and  "Enter Password" on line 2
2. ON Pressing any key, display "Enter Password" on line 1 and "****" on line 2
3. Features to the key pad "=" will be enter and "ON/C" will be clear
4. The password entered will be appended to the password array
5. There will be a show_password button (the ‘X’ key on the keypad) which when pressed will convert
    the * to the actual digits the user entered and show them on the LCD
6. A clear button (the ‘C’ key on the keypad) which when pressed will clear the password array and the display 
    will start from step 1
7. When enter is pressed the password array will be compared to the password stored in the EEPROM

8. If the code entered is incorrect, the Bicolor LED should flash red 5 times with a pulse interval of 1second.
9. The password array will be cleared and the display will start from step 1
10. IF the user enters the Incorrect password 3 times in a row, the system will lock out for 30 seconds a buzzer will sound
11. If the code entered is correct, the Bicolor LED should flash green 5 times with a pulse interval of 1second. 
    The user should then ‘open the latch’ (push the latch button) within this time period.
    If the button is not pressed, the system should return to a locked position
12. When the safe is opened the Bicolor LED should turn green and LCD TO DISPLAY OPEN until the safe is closed
13. The password can be reset only when the safe is open. If the reset_button (the ‘ON/C’ key on the keypad) 
    is pressed, the LCD should display a reset mode. First, the user should enter the current password
     correctly and press the enter_button. If the password is correct, the user should enter the
     new password and press the enter_button. If the password is wrong, the safe should return to safe
     open mode
14. The safe has a tilt sensor (potentiometer) for theft detection. If the value of the tilt sensor exceeds 2.5
    volts, the system should enter into emergency mode and lock up. An alert should be displayed on the
    LCD. The system should remain locked and inactive until the MCU is reset using the rest button.
*/
#define F_CPU 8000000UL
#include <stdio.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/eeprom.h>
#include <util/delay.h>
#include "LCD.h"
#include "ADC.h"

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
		LCD_String("7");
		
		break;
		case 1:
		data = 8;
		LCD_String("8");
		break;
		case 2:
		data = 9;
		LCD_String("9");
		break;
		case 3:
		LCD_String("/");
		break;
		case 4:
		data = 4;
		LCD_String("4");
		break;
		case 5:
		data = 5;
		LCD_String("5");
		break;
		case 6:
		data = 6;
		LCD_String("6");
		break;
		case 7:
		LCD_String("*");
		break;
		case 8:
		data = 1;
		LCD_String("1");
		break;
		case 9:
		data = 2;
		LCD_String("2");
		break;
		case 10:
		data = 3;
		LCD_String("3");
		break;
		case 11:
		LCD_String("-");
		break;
		case 12: // ON/C
		LCD_Clear();
		LCD_String("Enter Password: ");
		LCD_Cmd(0xC0);
		break;
		case 13:
		data = 0;
		LCD_String("0");
		break;
		case 14:
		LCD_String("=");
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
void check_password(void){
	int i;
	for (i = 0; i < 4; i++){
		if (input_password[i] != stored_password[i]){
			LCD_Clear();
			LCD_String("Wrong Password");
			_delay_ms(1000);
			LCD_Clear();
			LCD_String("Enter Password: ");
			LCD_Cmd(0xC0);
			count = 0;
			break;
		}
	}
}

// Showing password
void show_password(void){
	int i;
	for (i = 0; i < 4; i++){
		LCD_Char(input_password[i]);
	}
}

ISR(INT0_vect){
	// return the code of the key pressed and only get 4 digits
	key_code = read_keypad();
	

	_delay_ms(300);
	if (key_code != 255){
		input_password[count] = key_code;
		count ++;
		if (count == 4){
			// display the password entered
			char myString[20];

			sprintf(myString, "%d%d%d%d", input_password[0], input_password[1], input_password[2], input_password[3]);
			
			LCD_Clear();
			LCD_String("Password entered ");
			LCD_Cmd(0xC0);
			LCD_String(myString);
			count = 0;
		}
	}
}