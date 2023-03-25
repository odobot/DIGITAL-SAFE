/*
 * DIGITAL_SAFE.c
 *
 * Created: 23/03/2023 06:34:34
 * Author : Mwas
 */ 

#define F_CPU 8000000UL
#include <stdio.h>
#include <avr/io.h>
#include <stdlib.h>
#include <string.h>
#include <avr/interrupt.h>
#include <avr/eeprom.h>
#include <util/delay.h>
#include "LCD_4bit.h"
#include "ADC.h"

//_______________VAR DECLARATIONS____________________
int counter = 0;//while 1 loop counter
int KEY_Count = 0; //key loop adder
int p=0; // key loop counter

int PASSWORD_LENGTH = 4;
int press;//Translates actual integer values of the KEYPAD
int Open_Close_Counter=0;//helps know if the keypad was open or closed
unsigned int Input_Password[5];//password input from the keypad
unsigned int Actual_Password[4]; //password stored on the eeprom
int password_count; //ntaitaftia tuh kitu 
char buffer[16];   //this variable helps in conversion of integer to string using the itoa() stdlib function
int value, input, TILT, i=0;
uint16_t ENTERED_CODE, STORED_CODE;
int Safe_Status ;//0 for locked , 1 for transitioning, 2 for open


//_______________FUNC DECLARATIONS____________________
void COMPARE_PASSWORD(void);
void Change_Password();



//____________________________________________________KEYPAD mm74c922____________________________________________________________
void KEYCHECK(void);
uint8_t read_keypad(void);


//___________________________________________________ADC TILT SENSOR_____________________________________________________________
void TILT_ANGLE();

//___________________________________________________LATCH INTERUPT______________________________________________________________
ISR(INT1_vect){
	LCD_Clear();
if (Safe_Status==1) //changes the safe state to OPEN
{ 
	Safe_Status=2;
}
	
}


//_________________________________________________KEYPAD  INTERUPT______________________________________________________________

ISR(INT0_vect)
{    
	if (p==0 && Safe_Status!=4){LCD_Clear(); LCD_String("Enter Code"); LCD_Cmd(0Xc0); LCD_String(":");}
	if (Safe_Status == 0){Safe_Status = 1;}
	
	 if (p<=4){
	    value = PINB;
	    KEYCHECK();
	    Input_Password[p]=press;
		p++;
	   }
	   
	_delay_ms(100);
	
}


//______________________________________________________BIRG_____________________________________________________________________
void BIRG(int x){ //1 means flash green 5times & 2==Light green constantly & 3==Light red constantly
	
  if (x==3){
	   for (int t=0;t<=5;t++){ 
		    PORTD=PORTD | 0b00100000; 
		    
			_delay_ms(1000);
			PORTD=PORTD & 0b00011111;
			_delay_ms(500);
	       }
		
	   }else if(x==1){ 
		 	PORTD=PORTD | 0b00100000;
		  }else{
			    PORTD= (PORTD|0b01000000);
				}
		   
		   
}


//________________________________________________ATMEGA 32 EEPROM_______________________________________________________________
void Store_password(){
	
		eeprom_busy_wait();		/* Initialize LCD */
		eeprom_write_block(Input_Password,0,5);
}

void Read_EEPROM_Password(){
	eeprom_read_block(Actual_Password,0,5);
}


int main(void)
{    DDRD=0xE0;
	
   //________________________________________________________LCD____________________________________________________________________
	   LCD_Init();
	   
	
	/*______________________________ Interrupt setup___________________________ */
	
	//___________________________________________________LATCH INTERUPT______________________________________________________________
		/* Enable INT1*/
		
	//_________________________________________________KEYPAD  INTERUPT______________________________________________________________
		GICR = 1<<INT0 |1<<INT1; /* Enable INT0*/
		MCUCR = 1<<ISC01 | 1<<ISC00 |1<<ISC11|1<<ISC10; /* Trigger INT0 and INT1 on rising edge */
		sei(); /* Enable Global Interrupt */
	
	
    //___________________________________________________ADC TILT SENSOR_____________________________________________________________
	ADC_Init();
	
    
    //_________________________________________________________________________________________________________________________________________________________
	Safe_Status=0;
	Read_EEPROM_Password();
	
	
	
while (1)  {  
//............................................................................................................................................................	
//this section confirms if the password is being saved on the eeprom ,it should display only once after reset to confirm if the password is saved ...telemetry
		if(counter==1){
			LCD_Clear(); 
			LCD_String(":");
			for (int i=0;i<=3;i++){  
					itoa (Actual_Password[i],buffer,10);
					LCD_String(buffer);
					_delay_ms(100);
				}
				_delay_ms(3000);
				LCD_Clear();
	    }
	  if (counter==100){counter=2;}
			counter++;
//''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''""''''''''''''''''''
/*__________________________________LIGHTING OF THE BIRG LED BASED ON SAFE STATUS(refer to safe status var declaration)_____________________________________	*/
	 if (Safe_Status==0 && counter==99){BIRG(0);LCD_Clear(); LCD_String("Enter Code"); LCD_Cmd(0Xc0); LCD_String("_ _ _ _"); p=0;} 
     if (Safe_Status==2 && counter==99){LCD_Clear(); LCD_String("Safe: OPEN"); LCD_Cmd(0Xc0); LCD_String("_ _ _ _"); BIRG(1);}
/*____________________________________________READS ADC IF THERE IS THEFT______________________________________________________________________________________ */
	 TILT_ANGLE(); //Checks tilt  Angle
	 	
/*_____________________________________________________AFTER PASSWORD INPUT_____________________________________________________________________________________*/	 
		 if((p>3) && (Input_Password[4]==15) && (Safe_Status==1)){  //it displays the password entered if the X key is pressed X key is read as press=15
			 LCD_Clear();
			 LCD_Cmd(0x80); //Write on the top lcd row
			 LCD_String("Entered Code");
			 LCD_Cmd(0xc0);
			 LCD_String(":");
			 for (int i=0;i<=3;i++)
			 { itoa (Input_Password[i],buffer,10);
				 LCD_String(buffer);
				 _delay_ms(100);
			 }
			 Input_Password[4]=14;
			  _delay_ms(2000);
			 }
	 
			if ((p>=4) && (Input_Password[4]==14) && (Safe_Status==1))
			{   p=0;
				COMPARE_PASSWORD(); 
			}
	 
}
}

//_________________________________________________FUNCTIONS__________________________________________________________________________________________________________________________________________________________________
void KEYCHECK(void){
	if (p==0)
{
	//LCD_Cmd(0xc0);
}
	switch(value)
	{
		case 0:
		LCD_String("7");
		LCD_Cmd(0x14);
		press=7;
		KEY_Count++;
		break;
		
		case 1:
		LCD_String("8");
		LCD_Cmd(0x14);
		press=8;
		KEY_Count++;
		break;
		
		case 2:
		LCD_String("9");
		press=9;
		LCD_Cmd(0x14);
		KEY_Count++;
		break;
		
		case 3:
		LCD_String("/");
		LCD_Cmd(0x14);
		break;
		
		case 4:
		LCD_String("4");
		LCD_Cmd(0x14);
		press=4;
		KEY_Count++;
		break;
		
		case 5:
		LCD_String("5");
		LCD_Cmd(0x14);
		press=5;
		KEY_Count++;
		break;
		
		case 6:
		LCD_String("6");
		LCD_Cmd(0x14);
		press=6;
		KEY_Count++;
		break;
		
		case 7:
		press=15;
		KEY_Count++;
		break;
		
		case 8:
		LCD_String("1");
		LCD_Cmd(0x14);
		press=1;
		KEY_Count++;
		break;
		
		case 9:
		LCD_String("2");
		LCD_Cmd(0x14);
		press=2;
		KEY_Count++;
		break;
		
		case 10:
		LCD_String("3");
		LCD_Cmd(0x14);
		press=3;
		KEY_Count++;
		break;
		
		case 11:
		LCD_String("-");
		LCD_Cmd(0x14);
		break;
		
		case 12:
		Change_Password();
		KEY_Count = 4;
		break;
		
		case 13:
		LCD_String("0");
		LCD_Cmd(0x14);
		press=0;
		KEY_Count++;
		break;
		
		case 14:
		press=14;
		KEY_Count++;
		break;
		
		case 15:
		LCD_String("+");
		break;
	}
}

void TILT_ANGLE(void)
{
	TILT = ADC_Read(0);
	if(TILT>512)
	{
		LCD_Clear();
		PORTD = 0x80;
		LCD_String("THEFT");
		_delay_ms(2000);
		LCD_Clear();
		PORTD |=(1<<PD7);
		}
	}
	

void COMPARE_PASSWORD(void){  
	Read_EEPROM_Password();
	  Safe_Status=1;
	if ((Input_Password[0]==Actual_Password[0]) && (Input_Password[1]==Actual_Password[1]) && (Input_Password[2]==Actual_Password[2]) && (Input_Password[3]==Actual_Password[3])){
		BIRG(3);//Flash the BIRG Green 5 times
		if (Safe_Status!=2){Safe_Status=0; main();}
	    }else{
			 Safe_Status=0; main();
			 }
}

void Change_Password(){ 
	if(Safe_Status==2){ //Allows for change password if only the safe is open
		Safe_Status=4;
		p=0;
		LCD_Clear();
		LCD_String("RESET MODE");
		_delay_ms(1000);
		LCD_Clear();
		LCD_String("OLD PASSCODE:");
		LCD_Cmd(0XC0);
		LCD_String(":");
			// Read the entered OLD password from the keypad
		for(int z=0;z<=100000;z++){
			if (p>=4){break;}
		}
			if ((Input_Password[0]==Actual_Password[0]) && (Input_Password[1]==Actual_Password[1]) && (Input_Password[2]==Actual_Password[2]) && (Input_Password[3]==Actual_Password[3])){
				
						p=0;
						LCD_Clear();
						LCD_String("NEW PASSCODE:");
						LCD_Cmd(0XC0);
						// Read the entered NEW password from the keypad
						for(int z=0;z<=10000000;z++){
							if (p>=4){break;}
							}
							p=0;//wait for p=4 thus 5 key interupts
			         // Display the entered password from the keypad
						 LCD_Clear(); 
						LCD_String("NEW PASSCODE");
						 LCD_Cmd(0XC0);
						 LCD_String(":");
						 for (int i=0;i<=3;i++)
						 { itoa (Input_Password[i],buffer,10);
							 LCD_String(buffer);
							 _delay_ms(100);
						 }
				Store_password(); //store the password in eeprom
			} else{Safe_Status=2;}
	 
	 
	
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


