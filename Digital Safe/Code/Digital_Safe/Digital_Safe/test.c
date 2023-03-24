
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

//Global Variables
char password[4];
char password_index = 0;

// port definitions
#define LATCH_BUTTON_PORT PORTD 
#define LATCH_BUTTON_PIN PIND
#define LATCH_BUTTON_DDR DDRD
#define LATCH_BUTTON 2





//Function Prototypes
void init(void);
void check_password(void);
void reset_password(void);
void check_tilt(void);
void check_button(void);
void check_reset_button(void);
void check_show_password(void);
void check_enter_button(void);
void check_clear_button(void);
void check_keypad(void);



//Main Function
int main(void)
{
    init();
    while(1)
    {
        check_password();
        check_reset_button();
        check_tilt();
        check_button();
        check_keypad();
    }
    return 0;
}

//Function Definitions
void init(void)
{
    //Initialize LCD
    LCD_init();
    //Initialize ADC
    ADC_init();
    //Initialize Keypad
    keypad_init();
    //Initialize Buzzer
    buzzer_init();
    //Initialize Bicolor LED
    bicolor_led_init();
    //Initialize Latch
    latch_init();
    //Initialize Reset Button
    reset_button_init();
    //Initialize Tilt Sensor
    tilt_sensor_init();
    //Initialize Interrupts
    sei();
}

void check_password(void)
{
    //Display Welcome on line 1 and "Enter Password" on line 2
    LCD_clear();
    LCD_displayString("Welcome");
    LCD_goToRowColumn(1,0);
    LCD_displayString("Enter Password");
    //Wait for user to enter password
    while(1)
    {
        check_keypad();
        check_show_password();
        check_enter_button();
        check_clear_button();
    }
}

void check_reset_button(void)
{
    //Check if reset button is pressed
    if(reset_button_pressed())
    {
        //Display Reset Mode on line 1 and "Enter Password" on line 2
        LCD_clear();
        LCD_displayString("Reset Mode");
        LCD_goToRowColumn(1,0);
        LCD_displayString("Enter Password");
        //Wait for user to enter password
        while(1)
        {
            check_keypad();
            check_show_password();
            check_enter_button();
            check_clear_button();
        }
    }
}

void check_tilt(void)
{
    //Check if tilt sensor is tilted
    if(tilt_sensor_tilted())
    {
        //Display Emergency Mode on line 1 and "System Locked" on line 2
        LCD_clear();
        LCD_displayString("Emergency Mode");
        LCD_goToRowColumn(1,0);
        LCD_displayString("System Locked");
        //Lock the system
        while(1)
        {
            //Do nothing
        }
    }
}


void check_button(void)
{
    //Check if latch button is pressed
    if(latch_button_pressed())
    {
        //Display Open on line 1 and "Safe Open" on line 2
        LCD_clear();
        LCD_displayString("Open");
        LCD_goToRowColumn(1,0);
        LCD_displayString("Safe Open");
        //Turn on bicolor LED
        bicolor_led_green();
        //Wait for user to close the safe
        while(1)
        {
            //Check if latch button is pressed
            if(latch_button_pressed())
            {
                //Display Welcome on line 1 and  "Enter Password" on line 2
                LCD_clear();
                LCD_displayString("Welcome");
                LCD_goToRowColumn(1,0);
                LCD_displayString("Enter Password");
                //Turn off bicolor LED
                bicolor_led_off();
                //Wait for user to enter password
                while(1)
                {
                    check_keypad();
                    check_show_password();
                    check_enter_button();
                    check_clear_button();
                }
            }
        }
    }
}


void check_keypad(void)
{
    //Check if any key is pressed
    if(keypad_key_pressed())
    {
        //Display Enter Password on line 1 and "****" on line 2
        LCD_clear();
        LCD_displayString("Enter Password");
        LCD_goToRowColumn(1,0);
        LCD_displayString("****");
        //Wait for user to enter password
        while(1)
        {
            check_show_password();
            check_enter_button();
            check_clear_button();
        }
    }
}


void check_show_password(void)
{
    //Check if show password button is pressed
    if(show_password_button_pressed())
    {
        //Display Enter Password on line 1 and "****" on line 2
        LCD_clear();
        LCD_displayString("Enter Password");
        LCD_goToRowColumn(1,0);
        LCD_displayString("****");
        //Wait for user to enter password
        while(1)
        {
            check_enter_button();
            check_clear_button();
        }
    }
}


void check_enter_button(void)
{
    //Check if enter button is pressed
    if(enter_button_pressed())
    {
        //Check if password is correct
        if(password_correct())
        {
            //Display Open on line 1 and "Safe Open" on line 2
            LCD_clear();
            LCD_displayString("Open");
            LCD_goToRowColumn(1,0);
            LCD_displayString("Safe Open");
            //Turn on bicolor LED
            bicolor_led_green();
            //Wait for user to close the safe
            while(1)
            {
                //Check if latch button is pressed
                if(latch_button_pressed())
                {
                    //Display Welcome on line 1 and  "Enter Password" on line 2
                    LCD_clear();
                    LCD_displayString("Welcome");
                    LCD_goToRowColumn(1,0);
                    LCD_displayString("Enter Password");
                    //Turn off bicolor LED
                    bicolor_led_off();
                    //Wait for user to enter password
                    while(1)
                    {
                        check_keypad();
                        check_show_password();
                        check_enter_button();
                        check_clear_button();
                    }
                }
            }
        }
        else
        {
            //Display Wrong Password on line 1 and "Try Again" on line 2
            LCD_clear();
            LCD_displayString("Wrong Password");
            LCD_goToRowColumn(1,0);
            LCD_displayString("Try Again");
            //Wait for user to enter password
            while(1)
            {
                check_keypad();
                check_show_password();
                check_enter_button();
                check_clear_button();
            }
        }
    }
}



void check_clear_button(void)
{
    //Check if clear button is pressed
    if(clear_button_pressed())
    {
        //Display Enter Password on line 1 and "****" on line 2
        LCD_clear();
        LCD_displayString("Enter Password");
        LCD_goToRowColumn(1,0);
        LCD_displayString("****");
        //Wait for user to enter password
        while(1)
        {
            check_enter_button();
            check_clear_button();
        }
    }
}

void keypad_init(void)
{
    //Set keypad pins as input
    KEYPAD_DDR &= ~((1<<KEYPAD_PIN_1) | (1<<KEYPAD_PIN_2) | (1<<KEYPAD_PIN_3) | (1<<KEYPAD_PIN_4));
    //Enable pull-up resistors
    KEYPAD_PORT |= (1<<KEYPAD_PIN_1) | (1<<KEYPAD_PIN_2) | (1<<KEYPAD_PIN_3) | (1<<KEYPAD_PIN_4);
}

uint8_t keypad_key_pressed(void)
{
    //Check if any key is pressed
    if((KEYPAD_PIN & (1<<KEYPAD_PIN_1)) == 0)
    {
        //Wait for key to be released
        while((KEYPAD_PIN & (1<<KEYPAD_PIN_1)) == 0);
        //Return 1
        return 1;
    }
    else if((KEYPAD_PIN & (1<<KEYPAD_PIN_2)) == 0)
    {
        //Wait for key to be released
        while((KEYPAD_PIN & (1<<KEYPAD_PIN_2)) == 0);
        //Return 1
        return 1;
    }
    else if((KEYPAD_PIN & (1<<KEYPAD_PIN_3)) == 0)
    {
        //Wait for key to be released
        while((KEYPAD_PIN & (1<<KEYPAD_PIN_3)) == 0);
        //Return 1
        return 1;
    }
    else if((KEYPAD_PIN & (1<<KEYPAD_PIN_4)) == 0)
    {
        //Wait for key to be released
        while((KEYPAD_PIN & (1<<KEYPAD_PIN_4)) == 0);
        //Return 1
        return 1;
    }
    else
    {
        //Return 0
        return 0;
    }
}

void latch_button_init(void)
{
    //Set latch button pin as input
    LATCH_BUTTON_DDR &= ~(1<<LATCH_BUTTON_PIN);
    //Enable pull-up resistor
    LATCH_BUTTON_PORT |= (1<<LATCH_BUTTON_PIN);
}

uint8_t latch_button_pressed(void)
{
    //Check if latch button is pressed
    if((LATCH_BUTTON_PIN & (1<<LATCH_BUTTON_PIN)) == 0)
    {
        //Wait for latch button to be released
        while((LATCH_BUTTON_PIN & (1<<LATCH_BUTTON_PIN)) == 0);
        //Return 1
        return 1;
    }
    else
    {
        //Return 0
        return 0;
    }
}

void enter_button_init(void)
{
    //Set enter button pin as input
    ENTER_BUTTON_DDR &= ~(1<<ENTER_BUTTON_PIN);
    //Enable pull-up resistor
    ENTER_BUTTON_PORT |= (1<<ENTER_BUTTON_PIN);
}

uint8_t enter_button_pressed(void)
{
    //Check if enter button is pressed
    if((ENTER_BUTTON_PIN & (1<<ENTER_BUTTON_PIN)) == 0)
    {
        //Wait for enter button to be released
        while((ENTER_BUTTON_PIN & (1<<ENTER_BUTTON_PIN)) == 0);
        //Return 1
        return 1;
    }
    else
    {
        //Return 0
        return 0;
    }
}

void clear_button_init(void)
{
    //Set clear button pin as input
    CLEAR_BUTTON_DDR &= ~(1<<CLEAR_BUTTON_PIN);
    //Enable pull-up resistor
    CLEAR_BUTTON_PORT |= (1<<CLEAR_BUTTON_PIN);
}

