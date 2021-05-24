/*	Author: lab
 *  Partner(s) Name: 
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



unsigned char led0_output = 0x00;
unsigned char pause = 1;

enum pauseButtoSM_State {pauseButton_wait, pauseButton_press, pauseButton_release};

 // Monitors buttons connected to PA0
 // When button is pressed, shared variable "pause" is toggled
 int pauseButtonSMTick(int state){
     // Local Variables
     
     unsigned char press = GetKeypadKey();

     switch(state){
     case pauseButton_wait:
         state = press != '\0' ? pauseButton_press: pauseButton_wait; break;
     case pauseButton_press:
         state = pauseButton_release; break;
     case pauseButton_release:
         state = press == '\0' ? pauseButton_wait: pauseButton_press; break;
     default:
         state = pauseButton_wait; break;
     }

     switch(state){
     case pauseButton_wait: 
	pause = 1;
	break;
     case pauseButton_press:
         pause = 0;
         break;
     case pauseButton_release: break;
     }
     return state;

 }


enum toggleLED0_States {toggleLED0_wait, toggleLED0_blink};

int toggleLED0SMTick(int state){
    switch(state){
        case toggleLED0_wait: state = !pause? toggleLED0_blink: toggleLED0_wait; break;
        case toggleLED0_blink: state = pause? toggleLED0_wait: toggleLED0_wait; break;
        default: state = toggleLED0_wait; break;
    }

    switch(state){
        case toggleLED0_wait: 
	    led0_output = 0x00;
            break;

        case toggleLED0_blink:
            led0_output = 0x01;
        break;
    }
    return state;
}


enum display_States{ display_display};

int displayTickSM(int state){
    unsigned char output;

    switch(state){
        case display_display: state = display_display; break;
        default: state = display_display; break;
    }

    switch(state){
        case display_display:
            output = led0_output;
        break;
    }
    PORTB = output;
    return state;
 }


unsigned long int findGCD(unsigned long int a, unsigned long int b){
    unsigned long int c;
    while(1){
    c = a%b;
    if(c==0) {return b;}
        a=b;
    b=c;
    }
    return 0;
 }

 int main(void) {
     
     /* Insert DDR and PORT initializations */
     DDRB = 0xFF; PORTB = 0x00;
     DDRC = 0xF0; PORTC = 0x0F;

     static task task1, task2, task3;
     task *tasks[] = {&task1, &task2, &task3};
     const unsigned short numTasks = sizeof(tasks)/sizeof(task*);
     const char start = -1;
     
     task1.state = start; // init state
     task1.period = 25;
     task1.elapsedTime = task1.period;
     task1.TickFct = &pauseButtonSMTick;
     // Task2 { toggleLED0SM}
     task2.state = start; // init state
     task2.period = 500;
     task2.elapsedTime = task2.period;
     task2.TickFct = &toggleLED0SMTick;
     // Task3 { toggleLED1SM}
     task3.state = start; // init state
     task3.period = 10;
     task3.elapsedTime = task3.period;
     task3.TickFct = &displayTickSM;
     
     
     unsigned short i;
     unsigned long GCD = tasks[0]->period;
     for(i = 1; i<numTasks; i++){ GCD = findGCD(GCD, tasks[i]->period);}

     TimerSet(GCD);
     TimerOn();
     
      while(1){

          for(i =0; i<numTasks; i++){
              if(tasks[i]->elapsedTime == tasks[i]->period){
                  tasks[i]->state = tasks[i]->TickFct(tasks[i]->state);
                  tasks[i]->elapsedTime = 0;
              }
              tasks[i]->elapsedTime += GCD;
      }
      while(!TimerFlag);
      TimerFlag = 0;
      }
      return 0;
 }

