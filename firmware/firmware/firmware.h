#define F_CPU 12000000UL

#include <avr/io.h>
#include <avr/wdt.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <util/delay.h>

#include "usbdrv/usbdrv.h"

usbMsgLen_t usbFunctionSetup(uchar data[8]);

#define CUSTOM_RQ_SET_TEST 0
#define CUSTOM_RQ_SET_START 1
#define CUSTOM_RQ_SET_STOP 2
#define CUSTOM_RQ_GET_STATUS 3
#define CUSTOM_RQ_SET_BUZZER 4

#define output_low(port,pin) port &= ~(1<<pin)
#define output_high(port,pin) port |= (1<<pin)
#define set_input(portdir,pin) portdir &= ~(1<<pin)
#define set_output(portdir,pin) portdir |= (1<<pin)

const int TEST_MODE_TC = 10;
volatile int timerCounter = 0;

volatile int TEST_MODE = 0;
volatile int SWITCH_PRESSED = 0;

uchar BUZZER_ENABLED = 0; //General flag to disable buzzer
uchar BUZZER_ON = 0; //Flag to indicate that the buzzer is on
volatile uchar BUZZER_STATE = 0;
volatile uchar BUZZER_COUNTER_REF = 10;//Indicate frequency
volatile uchar BUZZER_COUNTER = 0;

void start(void);
void stop(void);
void test(void);
usbMsgLen_t getStatus(uchar data[4]);
