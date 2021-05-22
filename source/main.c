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
#ifdef _SIMULATE_
#include "simAVRHeader.h"
#endif

unsigned char led0_output = 0x00;
unsigned char led1_output= 0x00;
unsigned char pause = 0;


enum toggleLED0_States {toggleLED0_wait, toggleLED0_blink};

int toggleLED0SMTick(int state){
    switch(state){
	case toggleLED0_wait: state = !pause? toggleLED0_blink: toggleLED0_wait; break;
	case toggleLED0_blink: state = pause? toggleLED0_wait: toggleLED0_wait; break;
	default: state = toggleLED0_wait; break;
    }

    switch(state){
	case toggleLED0_wait: break;
	case toggleLED0_blink:
	    led0_output = (led0_output == 0x00) ? 0x01 : 0x00;
	    break;
    }
    return state;
}

enum toggleLED1_States {toggleLED1_wait, toggleLED1_blink};

int toggleLED1SMTick(int state){
    switch(state){
        case toggleLED1_wait: state = !pause? toggleLED1_blink: toggleLED1_wait; break;
        case toggleLED1_blink: state = pause? toggleLED1_wait: toggleLED1_wait; break;
        default: state = toggleLED1_wait; break;
    }

    switch(state){
        case toggleLED1_wait: break;
        case toggleLED1_blink:
            led0_output = (led1_output == 0x00) ? 0x01 : 0x00;
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
	    output = led0_output | led1_output << 1;
	    break;
    }
    PORTB = output;
    return state;
}


enum pauseButtoSM_State {pauseButton_wait, pauseButton_press, pauseButton_release};

// Monitors buttons connected to PA0
// When button is pressed, shared variable "pause" is toggled
int pauseButtonSMTick(int state){
    // Local Variables
    unsigned char press = ~PINA & 0x01;
   
    switch(state){
	case pauseButton_wait:
	    state = press == 0x01? pauseButton_press: pauseButton_wait; break;
	case pauseButton_press:
	    state = pauseButton_release; break;
	case pauseButton_release:
	    state = press == 0x00? pauseButton_wait: pauseButton_press; break;
	default:
	    state = pauseButton_wait; break;
    }

    switch(state){
	case pauseButton_wait: break;
	case pauseButton_press:
	    pause = (pause == 0) ? 1:0;
	    break;
	case pauseButton_release: break;
    }
    return state;

}

// -- Task Scheduler --
// Represents runnings process in our simple real time operating system
typedef struct task{
    signed char state;
    unsigned long int period;
    unsigned long int elapsedTime;
    int (*TickFct)(int);

} task;

unsigned char GetBit(unsigned char x, unsigned char k) {
   return ((x & (0x01 << k)) != 0);
}

unsigned char GetKeypadKey(){
    PORTC = 0xEF;
    asm("nop");
    if(GetBit(PINC,0)==0) {return('1');}
    if(GetBit(PINC,0)==0) {return('4');} 
    if(GetBit(PINC,0)==0) {return('7');} 
    if(GetBit(PINC,0)==0) {return('*');} 
 
    // check column 2 numbers
    PORTC = 0xDF;
    asm("nop"); 
    if(GetBit(PINC,0)==0) {return('2');}
    if(GetBit(PINC,0)==0) {return('5');} 
    if(GetBit(PINC,0)==0) {return('8');} 
    if(GetBit(PINC,0)==0) {return('0');}

    PORTC = 0xBF;
    asm("nop"); 
    if(GetBit(PINC,0)==0) {return('3');}
    if(GetBit(PINC,0)==0) {return('6');} 
    if(GetBit(PINC,0)==0) {return('9');} 
    if(GetBit(PINC,0)==0) {return('#');} 
 
    PORTC = 0x7F;
    asm("nop"); 
    if(GetBit(PINC,0)==0) {return('A');}
    if(GetBit(PINC,0)==0) {return('B');} 
    if(GetBit(PINC,0)==0) {return('C');} 
    if(GetBit(PINC,0)==0) {return('D');}

    return ('\0'); 


}


int main(void) {
    /* Insert DDR and PORT initializations */
    unsigned char x;
    DDRAB = 0x00; PORTA = 0xFF;
    DDRB = 0xFF; PORTB = 0x00;
  
    /* Insert your solution below */
    while (1) {
	x = GetKeypadKey();
	switch(x){
	    case '\0': PORTB = 0x1F; break;
	    case '1': PORTB = 0x01; break;
	    case '2': PORTB = 0x02; break;
	    case '3': PORTB = 0x03; break;
	    case '4': PORTB = 0x04; break;
            case '5': PORTB = 0x05; break;
            case '6': PORTB = 0x06; break;
	    case '7': PORTB = 0x07; break;
            case '8': PORTB = 0x08; break;
            case '9': PORTB = 0x09; break;
	    case 'A': PORTB = 0x0A; break;
            case 'B': PORTB = 0x0B; break;
            case 'C': PORTB = 0x0C; break;
            case 'D': PORTB = 0x0D; break;
            case '*': PORTB = 0x0E; break;
            case '0': PORTB = 0x00; break;
	    case '#': PORTB = 0x0F; break;
            default: PORTB = 0x1B; break;
	}	
    }
}
