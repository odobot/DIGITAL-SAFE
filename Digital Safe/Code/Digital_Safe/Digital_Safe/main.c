/*
 * Digital_Safe.c
 *
 * Created: 3/13/2023 2:58:12 PM
 * Author : Administrator
 */ 

#define F_CPU 8000000UL
#include <stdio.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/eeprom.h>
#include <util/delay.h>
#include "LCD.h"
#include "ADC.h"

int PASSWORD_LENGTH = 4;
int value, input, TILT, i=0;
uint16_t ENTERED_CODE, STORED_CODE;


//eeprom_write_byte(uint8_t *5946, uint8_t 64)

void COMPARE_PASSWORD(void)
{
	if (input == eeprom_read_byte((const uint8_t*)64)){
		
	}
}

uint8_t read_keypad(void)
{
	// Wait for the data to be available
	while (!(PIND & (1 << PD2)));

	// Read the data from the keypad
	int value = PINB;
	int data;
	switch(value)
	{
		case 0:
		data = 7;
		break;
		case 1:
		data = 8;
		break;
		case 2:
		data = 9;
		break;
		case 4:
		data = 4;
		break;
		case 5:
		data = 5;
		break;
		case 6:
		data = 6;
		break;
		case 8:
		data = 1;
		break;
		case 9:
		data = 2;
		break;
		case 10:
		data = 3;
		break;
		case 13:
		data = 0;
		break;
	}
	return data;	
}

void Change_Password()
{
	char strn[5];
	uint16_t ENTERED_CODE = 0;
	LCD_Clear();
	LCD_String("RESET MODE");
	_delay_ms(1000);
	LCD_Clear();
	LCD_String("OLD PASSCODE:");
	LCD_Cmd(0XC0);
	// Read the entered password from the keypad
	for (int i = 0; i < PASSWORD_LENGTH; i++)
	{
		uint8_t key = 0;
		while (!key)
		{
			key = read_keypad();
		}
		LCD_String(key);
		//ENTERED_CODE = (ENTERED_CODE * 10) + (key - 1);
		//itoa(ENTERED_CODE, strn, 10);
		//LCD_String(strn); // Mask the password input with asterisks
		//_delay_ms(100);
	}
	/*for (int i = 0; i < PASSWORD_LENGTH; i++)
	{
		while (!(UCSRA & (1 << RXC)));
		ENTERED_CODE = (ENTERED_CODE * 10) + (UDR - '0');
		LCD_String("*"); // Mask the password input with asterisks
	}*/
	//scanf("%d", &ENTERED_CODE);
	//if(value )
}

void KEYCHECK(void)
{
	switch(value)
	{
		case 0:
		LCD_String("7");
		break;
		case 1:
		LCD_String("8");
		break;
		case 2:
		LCD_String("9");
		break;
		case 3:
		LCD_String("/");
		break;
		case 4:
		LCD_String("4");
		break;
		case 5:
		LCD_String("5");
		break;
		case 6:
		LCD_String("6");
		break;
		case 7:
		LCD_String("*");
		break;
		case 8:
		LCD_String("1");
		break;
		case 9:
		LCD_String("2");
		break;
		case 10:
		LCD_String("3");
		break;
		case 11:
		LCD_String("-");
		break;
		case 12:
		Change_Password();
		break;
		case 13:
		LCD_String("0");
		break;
		case 14:
		LCD_String("=");
		break;
		case 15:
		LCD_String("+");
		break;
	}
}
 
/*Interrupt routine for INT0*/
/*What should happen when the interrupt is triggered*/
ISR(INT0_vect)
{
	if (TILT>512)
	{
		LCD_Clear();
		//PORTD |= (1<<PD7);
		//LCD_String("THEFT");
		while(i<1000000)
		{
			PORTD |= (1<<PD7);
			LCD_String("THEFT");
			_delay_ms(2000);
		}
	}
	else
	{
		PORTD &= (~(1<<PD7));
		LCD_Clear();
		value = PINB;
		KEYCHECK();
		_delay_ms(100);
	}
	
	
}
void TILT_ANGLE(void)
{
	TILT = ADC_Read(0);
	if(TILT>512)
	{
		LCD_Clear();
		//PORTD |= (1<<PD7);
		//LCD_String("THEFT");
		//_delay_ms(2000);
		//LCD_Clear();
		while(i<1000000)
		{
			PORTD |= (1<<PD7);
			LCD_String("THEFT");
			_delay_ms(2000);
			LCD_Clear();
		}
	}
	else
	{
		PORTD &= (~(1<<PD7));
		LCD_String("PORT");
		_delay_ms(2000);
		LCD_Clear();
	}
}


int main(void)
{
	/* Interrupt setup */
	GICR = 1<<INT0; /* Enable INT0*/
	MCUCR = 1<<ISC01 | 1<<ISC00; /* Trigger INT0 on rising edge */
	sei(); /* Enable Global Interrupt */
	ADC_Init();
	LCD_Init();
	DDRD |= (1<<PD7);
	
    while (1) 
    {
		TILT_ANGLE();	
    }
}

