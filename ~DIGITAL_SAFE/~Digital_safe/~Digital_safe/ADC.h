/*
 * ADC.h
 *
 * Created: 3/27/2023 5:34:59 PM
 *  Author: Administrator
 */ 


#ifndef ADC_H_
#define ADC_H_


void ADC_Init(){
	DDRA = 0x0;  /* Make ADC port as input */
	ADCSRA = 0x87;  /* Enable ADC, fr/128 */
	ADMUX = 0x40;
}

int ADC_Read(int channel){
	int Ain,AinLow;
	
	ADMUX = ADMUX|(channel); /* Set input channel to read */
	ADCSRA |= (1<<ADSC);  /* Start conversion */
	while((ADCSRA & (1<<ADIF)) == 0);/* Monitor end of conversion interrupt */
	_delay_us(10);
	
	AinLow = (int)ADCL; /* Read lower byte*/
	Ain = (int)ADCH*256; /* Read higher 2 bits and Multiply with weight */
	Ain = Ain + AinLow;
	return(Ain);  /* Return digital value*/
}


void timer_setup(){
	TIMSK |= (1<<TOIE1); //Timer overflow interrupt enable
	TCCR1B = (1<<CS11) || (1<<CS10); //prescalar 64
	TCNT1 = 3036; // Countdown value for 500ms
}


#endif /* ADC_H_ */