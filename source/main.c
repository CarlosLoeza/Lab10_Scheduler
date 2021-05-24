
/*	Author: lab
` *  Partner(s) Name: 
 *	Lab Section:
 *	Assignment: Lab #  Exercise #
 *	Exercise Description: [optional - include for your own benefit]
 *
 *	I acknowledge all content contained herein, excluding template or example
 *	code, is my own original work.
 */

#include <avr/io.h>
#include <avr/interrupt.h>
#ifdef _SIMULATE_
#include "simAVRHeader.h"
#endif


// -- Task Scheduler --
 // Represents runnings process in our simple real time operating system
typedef struct task{
     signed char state;
     unsigned long int period;
     unsigned long int elapsedTime;
     int (*TickFct)(int);

 } task;


volatile unsigned char TimerFlag = 0; // TimerISR() sets this to 1. C programmer should clea$

// Internal variables for mapping AVR's ISR to our cleaner TimerISR model.
unsigned long _avr_timer_M = 1; // Start counter from here to 0. Default 1ms
unsigned long _avr_timer_cntcurr = 0; // Current internal clock of 1ms ticks

void TimerOn(){
    // AVR timer/counter controller register TCCR1
    TCCR1B = 0x0B;      // bit3 = 0: CTC mode(clear timer on compare
                         // bit2bit1bit0 = 011;
                         // 0000 1011 : 0x0B
                         // So 8 MHz clock or 8,000,000 / 64  = 125,000 ticks
                         // Thus TCNT1 register will count 125,000 ticks

    // AVR out compare register OCR1A
    OCR1A = 125;        // Timer interrupt will be generated when TCNT1==OCR1A
                         // We want a 1ms tick. .001s * 125,000 ticks = 125
                         // So when TCNT1 == 125 then that means 1ms has passed

    // AVR timer register interrupt mask register
    TIMSK1 = 0x02;      // bit1: OCIE1A -- enables compare match register

    // Initiliaze AVR counter
    TCNT1 = 0;

    _avr_timer_cntcurr = _avr_timer_M;

    // enable global interrupts
    SREG |= 0x80;       // 1000 0000
}

void TimerOff(){
    TCCR1B = 0x00;
}

void TimerISR(){
    TimerFlag = 1;
}

 // In our approach C program does not touch this ISR, only TimerISR()
ISR(TIMER1_COMPA_vect){
     // CPU automatically calls when TCNT1 == OCR1 (Every 1ms per TimerOn settings)
     _avr_timer_cntcurr--;       // count down to 0
     if (_avr_timer_cntcurr == 0){
         TimerISR();     // call the ISR that the user uses
         _avr_timer_cntcurr = _avr_timer_M;
     }
 }

// Set timer to tick every M ms.
void TimerSet(unsigned long M){
    _avr_timer_M = M;
    _avr_timer_cntcurr =  _avr_timer_M;
}



unsigned char GetBit(unsigned char x, unsigned char k) {
   return ((x & (0x01 << k)) != 0);
}

unsigned char GetKeypadKey(){
    PORTC = 0xEF;
    asm("nop");
    if(GetBit(PINC,0)==0) {return('1');}
    if(GetBit(PINC,1)==0) {return('4');}
    if(GetBit(PINC,2)==0) {return('7');}
    if(GetBit(PINC,3)==0) {return('*');}
 
    // check column 2 numbers
    PORTC = 0xDF;
    asm("nop");
    if(GetBit(PINC,0)==0) {return('2');}
    if(GetBit(PINC,1)==0) {return('5');}
    if(GetBit(PINC,2)==0) {return('8');}
    if(GetBit(PINC,3)==0) {return('0');}
    PORTC = 0xBF;
    asm("nop");
    if(GetBit(PINC,0)==0) {return('3');}
    if(GetBit(PINC,1)==0) {return('6');}
    if(GetBit(PINC,2)==0) {return('9');}
    if(GetBit(PINC,3)==0) {return('#');}
 
    PORTC = 0x7F;
    asm("nop");
    if(GetBit(PINC,0)==0) {return('A');}
    if(GetBit(PINC,1)==0) {return('B');}
    if(GetBit(PINC,2)==0) {return('C');}
    if(GetBit(PINC,3)==0) {return('D');}
    return ('\0');
}


unsigned char step;
unsigned char button;
unsigned char light;
enum Lock_States {Lock_Start, Lock_Reset, Lock_Held, Lock_Pound, Lock_One, Lock_Two, Lock_Three, Lock_Four, Lock_Unlock} Lock_State;

void Door_Lock(){
    button = GetKeypadKey();
    switch(Lock_State){
        case Lock_Start:
            Lock_State = Lock_Reset;
            break;
        case Lock_Reset:
            if(button == '#'){
                Lock_State = Lock_Held;
            } else if (button != '#'){
                Lock_State = Lock_Reset;
            }
            break;
            
        case Lock_Held:
            // if user releases button, select next state based on the last step.
            // step 0 = '#', step 1 = '1', step 2 = '2',..., step 4 = '4'
            if(button == '\0'){
                switch(step){
                    case 0:
                        Lock_State = Lock_Pound;
                        break;
                    case 1:
                        Lock_State = Lock_One;
                        break;
                    case 2:
                        Lock_State = Lock_Two;
                        break;
                    case 3:
                        Lock_State = Lock_Three;
                        break;
                    case 4:
                        Lock_State = Lock_Four;
                        break;
                    case 5:
                        Lock_State = Lock_Unlock;
                        break;
                }
            } else
                Lock_State = Lock_Held;
            break;
            
        case Lock_Pound:
            if(button == '1'){
                step = 1;
		Lock_State = Lock_Held;
	    }
            else if(button == '\0')
                Lock_State = Lock_Pound;
            else
                Lock_State = Lock_Reset;
            break;
            
        case Lock_One:
            if(button == '2'){
		step = 2;
                Lock_State = Lock_Held;
            }else if(button == '\0')
                Lock_State = Lock_One;
            else
                Lock_State = Lock_Reset;
            break;
            
        case Lock_Two:
            if(button == '3'){
                step = 3;
		Lock_State = Lock_Held;
            }else if(button == '\0')
                Lock_State = Lock_Two;
            else
                Lock_State = Lock_Reset;
            break;
            
            
        case Lock_Three:
            if(button == '4'){
                step = 4;
		Lock_State = Lock_Held;
            }else if(button == '\0')
                Lock_State = Lock_Three;
            else
                Lock_State = Lock_Reset;
            break;
            
        case Lock_Four:
            if(button == '5'){
		step = 5;
                Lock_State = Lock_Held;
            }else if(button == '\0')
                Lock_State = Lock_Four;
            else
                Lock_State = Lock_Reset;
            break;
            
        case Lock_Unlock:
            if(button != '\0')
                Lock_State = Lock_Reset;
            else
                Lock_State = Lock_Unlock;
            break;
        default:
            Lock_State = Lock_Reset;
	    break;
    }
        
    switch(Lock_State){
        case Lock_Reset:
            step =0;
            light = 0x10;
            break;
        case Lock_Pound:
	    light = 0x00;  
            break;
        case Lock_One:
            light = 0x01;
            break;
        case Lock_Two:
            light = 0x02;
            break;
        case Lock_Three:
            light = 0x04;
            break;
        case Lock_Four:
            light = 0x08;
            break;
        case Lock_Unlock:
            light = 0x1F;
            break;
    }
    PORTB = light;
    
}

 int main(void) {
     
     /* Insert DDR and PORT initializations */
     DDRB = 0xFF; PORTB = 0x00;
     DDRC = 0xF0; PORTC = 0x0F;

     Lock_State = Lock_Start;
     TimerSet(250);
     TimerOn();
     
      while(1){
	  Door_Lock();

      
          while(!TimerFlag);
          TimerFlag = 0;
      }
      
 }

