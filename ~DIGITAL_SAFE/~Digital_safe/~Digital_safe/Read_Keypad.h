/*
 * Read_Keypad.h
 *
 * Created: 3/27/2023 5:34:26 PM
 *  Author: Administrator
 */ 


#ifndef READ_KEYPAD_H_
#define READ_KEYPAD_H_


/* Keypad array holding the keys in a grid arrangement*/
unsigned char keypad[4][4] = {
	{'7','8','9','/'},
	{'4','5','6','*'},
	{'1','2','3','-'},
	{'c','0','=','+'}
};

/* Setup function for the Keypad */
void keypad_Init(){
	DDRD |= (0<<PD2); /* PORTD as input */
	PORTD |= (1<<PD2); /* Activate pull up resistor high */
	
	/* Interrupt setup */
	GICR = 1<<INT0;  /* Enable INT0*/
	MCUCR = 1<<ISC01 | 1<<ISC00;  /* Trigger INT0 on rising edge */
	sei();    /* Enable Global Interrupt */
}

/* Function that checks the key that has been pressed on the keypad*/
unsigned char check_Keypad(char input_val){
	int row = input_val/4;
	int col = input_val%4;
	if((input_val>= 0) & (input_val<16))
	return (keypad[row][col]);
	else
	return 0;
}



#endif /* READ_KEYPAD_H_ */