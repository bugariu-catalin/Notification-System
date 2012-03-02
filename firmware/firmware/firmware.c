#include "firmware.h"

int main(void)
{
	uchar   i;
	timerCounter = TEST_MODE_TC;
	
	set_output(DDRD,PD6);
	set_output(DDRB,PB0);
	set_output(DDRB,PB1);
	set_output(DDRB,PB2);
	set_output(DDRB,PB4);
	output_high(PORTB,PB0);
	
	GIMSK = _BV (PCIE);
	PCMSK = _BV (PCINT3);
	MCUCR = _BV (ISC00);
		
	//Set timer at 1 Hz
	TCCR1B |= (1 << WGM12);
	OCR1A   = F_CPU / 256; 
	TCCR1B |= (1 << CS12);
	TIMSK |= (1 << OCIE1A);
	
	//Set timer 2 for buzzer
	TCCR0B |= (1 << WGM12);
	OCR0A = F_CPU / 1024 * 10; 
	TCCR0B |= (1 << CS12);
	TIMSK |= (1 << OCIE0A); 

	wdt_enable(WDTO_1S);
	usbInit();
	usbDeviceDisconnect();
	i = 0;
    while(--i){             // fake USB disconnect for > 250 ms 
        wdt_reset();
        _delay_ms(1);
    }
	
    usbDeviceConnect();

    sei();

    while(1){
        wdt_reset();
        usbPoll();
    }
}

usbMsgLen_t usbFunctionSetup(uchar data[8])
{
	usbRequest_t    *rq = (void *)data;
	static uchar    dataBuffer[4];  /* buffer must stay valid when usbFunctionSetup returns */

	if(rq->bRequest == CUSTOM_RQ_SET_START){
		start();
        if(rq->wValue.bytes[0] & 1){    /* set LED */
            //LED_PORT_OUTPUT |= _BV(LED_BIT);
			
        }else{                          /* clear LED */
            //LED_PORT_OUTPUT &= ~_BV(LED_BIT);
			//stop();
        }
    }else if(rq->bRequest == CUSTOM_RQ_SET_STOP){
		stop();
	}else if(rq->bRequest == CUSTOM_RQ_SET_TEST){
		//Enter test mode
		test();
    }else if(rq->bRequest == CUSTOM_RQ_GET_STATUS){
        //dataBuffer[0] = ((LED_PORT_OUTPUT & _BV(LED_BIT)) != 0);
		int ret = getStatus(dataBuffer);
        usbMsgPtr = dataBuffer;         /* tell the driver which data to return */
        return ret;                       /* tell the driver to send 1 byte */
	}else if(rq->bRequest == CUSTOM_RQ_SET_BUZZER){
		if (rq->wValue.bytes[0]>0) {
			//enable buzzer
			BUZZER_ENABLED = 1;
			BUZZER_COUNTER_REF = rq->wValue.bytes[0];
		} else {
			//disable buzzer
			BUZZER_ENABLED = 0;
		}
    }
    return 0;   /* default for not implemented requests: return no data back to host */
}

//Timer1 interrupt vector
ISR(TIMER1_COMPA_vect)
{
	if (TEST_MODE==0) return;
	if (timerCounter>0) {
		timerCounter--;
		return ;
	}
	//TIMSK |= (0 << OCIE1A); //stop timer
	stop();
	output_low(PORTD,PD6);
	TEST_MODE = 0;
	timerCounter=TEST_MODE_TC;
} 

ISR(TIMER0_COMPA_vect)
{
	if (BUZZER_ENABLED==0 || BUZZER_ON==0) return;
	if (BUZZER_COUNTER>0) {
		BUZZER_COUNTER--;
		return;
	}
	BUZZER_STATE=!BUZZER_STATE;
	if (BUZZER_STATE==1) {
		output_high(PORTB,PB4);
	} else {
		output_low(PORTB,PB4);
	}
	BUZZER_COUNTER = BUZZER_COUNTER_REF;	
}

//Start test mode when PCINT3 is pulled up
ISR (PCINT_vect)
{
	SWITCH_PRESSED=!SWITCH_PRESSED;
	if (TEST_MODE==1) return;
	if (SWITCH_PRESSED==1) test();
}

void start(void)
{
	output_high(PORTB,PB2);
	output_high(PORTB,PB1);
	BUZZER_ON = 1;
}

void stop(void)
{
	output_low(PORTB,PB2);
	output_low(PORTB,PB1);	
	output_low(PORTB,PB4);
	BUZZER_ON = 0;
}

void test(void)
{
	TEST_MODE = 1;
	start();
	output_high(PORTD,PD6);
	TIMSK |= (1 << OCIE1A);
}

usbMsgLen_t getStatus(uchar data[4]) 
{
	
	if (TEST_MODE) data[0] = 1;
	else if (bit_is_set(PORTB, PB2)) data[0] = 2; else data[0] = 3;
	
	data[1] = BUZZER_ENABLED;
	data[2] = BUZZER_COUNTER_REF;
	
	return 3;
}