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
int input_password[4] = {-1, -1, -1, -1};
int stored_password[4] = {1, 2, 3, 4}; // to be changed to eeprom
int count = 0;
int check_password(void);
void show_password(void);
int password_entered = 0; // flag variable to indicate if password has been entered
void BIRG(int status);
int safe_open = 0; // flag variable to indicate if safe is open 0 - locked, 1 - open
volatile uint8_t latch_state = 0; // flag variable to indicate if button is pressed 0 - not pressed, 1 - pressed
uint8_t read_keypad(void); // function to read the keypad
void activate_buzzer(void);

int main(void)
{	
	DDRD |= (1 << PD5) | (1 << PD6); // set PD5 and PD6 as output
	// Set PD7 as an output
    DDRD |= (1 << BUZZER_PIN);
	
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
		

		// TODO: Add other main loop code
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
		LCD_Clear();
		LCD_String("Enter Password: ");
		LCD_Cmd(0xC0);
		memset(input_password, 0, sizeof(input_password)); // clear out the array
		// return led to original state
		BIRG(2);
		break;

		case 13:
		data = 0;
		LCD_String("*");
		break;
		case 14:
		check_password();
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
				LCD_String("OPEN");
				BIRG(0); // open the safe
				latch_state = 0;
			}
		}
	}
	else if (status == 0) // the safe is open
	{
		PORTD |= (1 << PD5);
	}
	else if (status == 2) // the safe is locked
	{
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
	int i;
	for (i = 0; i < 4; i++)
	{
		if (input_password[i] != stored_password[i])
		{
			
			LCD_Clear();
			LCD_String("Wrong Password");
			count = 0;
			BIRG(3); // wrong password blink 5 times
			activate_buzzer();
			return -1;
		}
	}
	 // if the loop completes without returning, the password is correct
	 LCD_Clear();
	 LCD_String("Correct Password");
	 BIRG(1); // correct password flash green 5 times
	 
	 count = 0;
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
		latch_state = 1;
		//check_password();
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

    // Generate a square wave on PD7 to activate the buzzer
    int duration = 1000; // duration of the "scream" (in milliseconds)
    int frequency = 1500; // starting frequency of the "scream"
    int delta = 50; // amount to increase the frequency with each cycle
    int cycles = duration / 20; // number of cycles to produce the "scream"
    for (int i = 0; i < cycles; i++)
    {
        OCR0 = 128; // set duty cycle to 50%
        _delay_ms(10); // delay for 10 ms
        frequency += delta; // increase the frequency
        OCR0 = 128 + (127 * sin(2 * 3.14 * frequency * i / 1000)); // generate the PWM signal
        _delay_ms(10); // delay for 10 ms
    }

    // Turn off the buzzer
    TCCR0 &= ~(1 << WGM01) & ~(1 << WGM00) & ~(1 << COM01) & ~(1 << CS00);
    PORTD &= ~(1 << BUZZER_PIN);
}