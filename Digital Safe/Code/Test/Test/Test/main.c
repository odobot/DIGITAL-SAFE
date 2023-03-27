#define F_CPU 8000000UL
#include <stdio.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/eeprom.h>
#include <util/delay.h>
#include "LCD.h"
#include "ADC.h"
#include <string.h>
#include <math.h>


#define KEY_DDR     DDRB
#define KEY_PIN     PINB
#define KEY_PORT    PORTB

#define ROW_MASK    0x0F
#define COL_MASK    0xF0
#define BUZZER_PIN PD7

volatile uint8_t key_pressed = 0;
volatile uint8_t key_code = 0;

// Define Global Variables
uint8_t input_password[4] = {-1, -1, -1, -1};
uint8_t stored_password[4] = {1, 2, 3, 4}; // to be changed to eeprom
int count = 0;
int check_password(void);
void show_password(void);
int password_entered = 0; // flag variable to indicate if password has been entered
void BIRG(int status);
int safe_open = 0; // flag variable to indicate if safe is open 0 - locked, 1 - open
volatile uint8_t latch_state = 0; // flag variable to indicate if button is pressed 0 - not pressed, 1 - pressed
uint8_t read_keypad(void); // function to read the keypad
void activate_buzzer(void);
int password_reset = 0; // flag variable to indicate if password has been reset 0 - not reset, 1 - reset
void change_password(void);
void keystroke(void);


int main(void)
{	
	DDRD |= (1 << PD5) | (1 << PD6); // set PD5 and PD6 as output
	// Set PD7 as an output
    DDRD |= (1 << BUZZER_PIN);
	
	// Write the array to EEPROM
    //eeprom_write_block((const void*)stored_password, (void*)0, sizeof(stored_password));

	LCD_Init();
	LCD_String("Enter Password: ");
	LCD_Cmd(0xC0);
	BIRG(2); // Safe starts as locked
	
	// Configure INT0 and INT1 interrupts to trigger on rising edge
	GICR |= (1 << INT0) | (1 << INT1);
	MCUCR |= (1 << ISC00) | (1 << ISC11) | (1 << ISC10);
	sei(); /* Enable Global Interrupt */

	
	// Initialize the BIRG

	while (1)
	{
		if (password_reset==0){//being entered normally
			key_code = read_keypad();
			if (key_code != 255){
				input_password[count] = key_code;
				count ++;
				if (count == 4){
					// display the password entered
					count = 0;
					password_entered = 1;
				}
			}
		}		
	}
}

void keystroke(void)
{
	int pos = 0;
	while (1){
		key_code = read_keypad();
		if (key_code != 255){
			input_password[pos] = key_code;
			pos ++;
			if (pos == 4){
				// display the password entered
				//pos = 0;
				//password_entered = 1;
				break;
			}
		}
	}
}


uint8_t read_keypad(void)
{
	// Wait for the data to be available
	while (!(PIND & (1 << PD2)));

	// Read the data from the keypad
	int value = PINB;
	int data = 255; // initialize to a default value
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
		// reset the password if safe is open
		if (safe_open == 1){
			LCD_Clear();
			LCD_String("Reset Password:");
			LCD_Cmd(0xC0);
			BIRG(0);
			_delay_ms(1000);
			password_reset = 1;
		}
		// LCD_Clear();
		// LCD_String("Enter Password: ");
		// LCD_Cmd(0xC0);
		// memset(input_password, 0, sizeof(input_password)); // clear out the array
		// return led to original state
		break;

		case 13:
		data = 0;
		LCD_String("*");
		break;
		case 14:
		// check password and check if it's a reset or not
		if ((password_reset == 0) && (check_password() == -1)){// normal operation wrong password
			LCD_Clear();
			LCD_String("WRONG PASSWORD!");
			BIRG(3); // wrong password blink 5 times
			LCD_Cmd(0xC0);
		}
		else if ((password_reset == 0) && (check_password() == 1)){ // normal operation correct password
			LCD_Clear();
			LCD_String("Correct Password");
			BIRG(1); // correct password flash green 5 times
		}
		else if ((password_reset == 1) && (safe_open == 1)){ // reset password
			for (int i = 0; i < 4; i++){
				stored_password[i] = input_password[i];
				}				
			password_reset = 0; // reset the password reset flag
			LCD_Clear();
			LCD_String("Code Changed");
			// _delay_ms(1000);
			eeprom_write_block((const void*)input_password, (void*)0, sizeof(input_password));
			_delay_ms(1000);
			LCD_Clear();
			LCD_String("Enter Password: ");
			LCD_Cmd(0xC0);

		}
		//else if ((safe_open == 1) && check_password() == 1){ // check if the current password is set correctly
			//password_reset = 2; // generic flag to indicate that the password is set correctly
		//}
		//else if ((safe_open == 1) && check_password() == -1){ // check if the current password is not set correctly
			//password_reset = 3; // generic flag to indicate that the password is set correctly
		//}
		break;
		case 15:
		LCD_String("+");
		break;
		
	}

	// Debounce delay
	_delay_ms(500);

	return data;
}

// Lighting the BIRG LED
// If the code entered is correct, the Bicolor LED should flash green 5 times with a pulse interval of 1second
void BIRG(int status)
{
	int i;
	if (status == 1){
		for (i = 0; i < 5; i++)
		{
			// Blink green but remove state either green or red
			PORTD &= ~(1 << PD6);
			PORTD |= (1 << PD5);
			_delay_ms(1000);
			PORTD &= ~(1 << PD5);
			_delay_ms(1000);
			if (latch_state == 1){
				
				LCD_Clear();
				LCD_String("OPEN MODE");
				BIRG(0); // open the safe
				latch_state = 0;
				safe_open = 1;
				break;
			}
			else if ((i ==4) && (latch_state ==0)){
				latch_state = 0;
				safe_open =0;
				LCD_Clear();
				LCD_String("LOCKED MODE");
				LCD_Cmd(0xC0);
				BIRG(2); // lock the safe
			}
		}
	}
	else if (status == 0) // the safe is open
	{
		// Blink green but remove state either green or red
		PORTD &= ~(1 << PD6);
		PORTD |= (1 << PD5);
	}
	else if (status == 2) // the safe is locked
	{
		// Blink red but remove state either green or red
		PORTD &= ~(1 << PD5);
		PORTD |= (1 << PD6);
	}
	else if (status == 3) // wrong password blink 5 times
	{
		for (i = 0; i < 5; i++)
		{
			// Blink red but remove state either green or red
			PORTD &= ~(1 << PD5);
			PORTD |= (1 << PD6);
			_delay_ms(1000);
			PORTD &= ~(1 << PD6);
			_delay_ms(1000);
		}
	}
}

// Checking password
int check_password(void)
{
	//while (!(PIND & (1 << PD3)));
	// read the password from eeeprom
	eeprom_read_block((void *)&stored_password, (const void *)0, 4);

	int i;
	for (i = 0; i < 4; i++)
	{
		if (input_password[i] != stored_password[i])
		{
			//count = 0;
			activate_buzzer();
			return -1;
		}
	}
	 // if the loop completes without returning, the password is correct
	//count = 0;
	return 1;
}

// Showing password
// Function to show the entered password
void show_password(void){
	char buffer[40];
	char str[10];

	LCD_Clear();
	LCD_String("Enter Password: ");
	LCD_Cmd(0xC0);
	
	if (password_entered == 1){
	for (int i = 0; i < 4; i++){
		if (input_password[i] != -1){
			sprintf(str, "%d", input_password[i]);
			strcat(buffer, str);
		}
	}

	LCD_String(buffer);
	memset(input_password, -1, sizeof(input_password)); // reset to -1
	memset(buffer, 0, sizeof(buffer));
	password_entered = 0;
	}
}



ISR(INT0_vect){
	// Check if password is being reset or entered
	if (password_reset == 1){
		count = 0;
		change_password();
	}
}

ISR(INT1_vect)
{
	// Debounce delay
	_delay_ms(50);
	// Wait for the data to be available
	//while (!(PIND & (1 << PD3)));
	// Check if the button is pressed
	if (PIND & (1 << PD3))
	{
		// if the button is pressed, read the data change latch_state and check if safe is open
		// if (latch_state == 0 && safe_open == 1){
		// 	latch_state = 1;
		// }
		if (latch_state == 0 && safe_open == 1){
			latch_state = 0;
			safe_open =0;
			LCD_Clear();
			LCD_String("LOCKED MODE");
			LCD_Cmd(0xC0);
			BIRG(2); // lock the safe
		}
		else if (latch_state == 0 && safe_open == 0){
			latch_state = 1;
		}
		// else if (latch_state == 1 && safe_open == 0){
		// 	latch_state = 0;
		// }
	}

	// Clear the INT1 flag
	GIFR |= (1 << INTF1);

}




void activate_buzzer(void)
{
	// Set PD7 as an output
	DDRD |= (1 << BUZZER_PIN);

	// Set up Timer/Counter 0 for PWM operation
	TCCR0 |= (1 << WGM01) | (1 << WGM00) | (1 << COM01) | (1 << CS00);

	// Generate a police siren on PD7
	int duration = 5000; // duration of the siren (in milliseconds)
	int frequency = 800; // starting frequency of the siren
	int delta = 50; // amount to increase the frequency with each cycle
	int cycles = duration / 20; // number of cycles to produce the siren
	for (int i = 0; i < cycles; i++)
	{
		OCR0 = 128 + (127 * sin(2 * 3.14 * frequency * i / 1000)); // generate the PWM signal
		_delay_ms(10); // delay for 10 ms
		frequency += delta; // increase the frequency
	}

	// Turn off the buzzer
	TCCR0 &= ~(1 << WGM01) & ~(1 << WGM00) & ~(1 << COM01) & ~(1 << CS00);
	PORTD &= ~(1 << BUZZER_PIN);
}

// When on/c is pressed change the
void change_password(void){
	// ask for current password
	LCD_Clear();
	LCD_String("Current Password!!");
	LCD_Cmd(0xC0);
	// wait for the user to enter the current password
	//password_reset = 23; // password has not been reset
	//key_code = read_keypad();
	keystroke();
	// check if the password is correct
	//LCD_String("TEST");
	if (check_password() == 1){ // correct password
		LCD_Clear();
		LCD_String("Enter New CODE: ");
		LCD_Cmd(0xC0);
		password_reset = 1; // password has been reset
		// ask for the new password
		//key_code = read_keypad();
		keystroke();
	}
	else if (check_password() == -1){ // incorrect password
		LCD_Clear();
		LCD_String("Wrong Password");
		_delay_ms(1000);
		LCD_Clear();
		LCD_String("OPEN MODE");
		BIRG(0); // open the safe
		latch_state = 0;
		safe_open = 1;
	}
	//password_reset = 1; // to continue the loop
	// if the current password is correct, ask for the new password and press enter

	//if incorrect return to open mode
}

// reading and storing values in EEPROM
void read_eeprom_password(void){
	eeprom_read_block((void*)&stored_password, (const void*)0, 4);
}

void write_eeprom_password(void){
	eeprom_write_block((const void*)&input_password, (void*)0, 4);
}