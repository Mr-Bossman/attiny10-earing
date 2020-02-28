#define F_CPU 8000000UL
#include <avr/io.h>
#include <util/delay_basic.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#define ogseed 6203

struct table{
	unsigned D:3;
	unsigned P:3;
}table;

#define msglng 3
const uint8_t message[msglng] = {0b10000000,0b10010101,0b11000000};
const struct table t[] PROGMEM = {{0b101,0b001},{0b110,0b100},{0b011,0b010},{0b110,0b010},{0b101,0b100},{0b011,0b001}};
const uint8_t vals[26] PROGMEM = {0,0,1,3,9,19,36,65,105,154,203,240,255,240,203,154,105,65,36,19,9,3,1,0,0};
volatile uint8_t padern = 0;
/*
import java.lang.Math;
public class Main
{
	public static void main(String[] args) {
	    System.out.print("{");
	    for(double x = 0;x < 2.2;x+= 0.5){
	        System.out.print((int)(Math.pow(2,Math.sin((x)*(Math.PI/5))*8)-1) +",");
	    }
	    System.out.print((int)(Math.pow(2,Math.sin((2.4)*(Math.PI/5))*8)-1));
	    System.out.print("}");
	}
}
*/

void brightness(uint8_t val, uint8_t led){
	register uint8_t top = 255;
	while(top--) {
		DDRB = 0;
		PORTB = 0;
		if(top < val){
			DDRB = t[led].D;
			PORTB = t[led].P;
		}
	}
	DDRB = 0;
	PORTB = 0;
}
void fadedown(){
	for(int i = 0; i <12;i++){
		uint8_t n = 20;
		while(n--){
			brightness(vals[0+i],0);
			brightness(vals[2+i],1);
			brightness(vals[4+i],2);
			brightness(vals[6+i],3);
			brightness(vals[8+i],4);
			brightness(vals[10+i],5);
		}
	}
}
void fadeup(){
	for(int i = 0; i < 12 ;i++){
		uint8_t n = 20;
		while(n--){
			brightness(vals[0+i],5);
			brightness(vals[2+i],4);
			brightness(vals[4+i],3);
			brightness(vals[6+i],2);
			brightness(vals[8+i],1);
			brightness(vals[10+i],0);
		}
	}

}
void set(uint8_t led){
	DDRB = 0;
	PORTB = 0;
	DDRB = t[led].D;
	PORTB = t[led].P;
}
void all(){
	for(uint8_t led = 0; led < 6;led++){
		DDRB = 0;
		PORTB = 0;
		DDRB = t[led].D;
		PORTB = t[led].P;
	}
	DDRB = 0;
	PORTB = 0;
}
void morse(const uint8_t * bytes){
	for(uint8_t byte = 0; byte <msglng;byte++){
		uint8_t bits = bytes[byte];
		for(uint8_t bit = 0; bit < 4;bit++){
		
			if((bits&3) == 0){
				for(uint16_t timeon = 4000;timeon>0;timeon--)all();
				for(uint16_t timeoff = 4000;timeoff>0;timeoff--)_delay_loop_1(0);
			}else if((bits&3) == 1){
				for(uint16_t timeon = 24000;timeon>0;timeon--)all();
				for(uint16_t timeoff = 4000;timeoff>0;timeoff--)_delay_loop_1(0);
			} else if((bits&3) == 2){
				for(uint16_t timeoff = 8000;timeoff>0;timeoff--)_delay_loop_1(0);
			} else {
				for(uint16_t timeoff = 24000;timeoff>0;timeoff--)_delay_loop_1(0);
			}
			bits >>= 2;
		}
	}
}
uint16_t x = 0, w = 0;
inline static uint16_t randn() {
	x *= x;
	x += (w += ogseed);
	return x = (x>>8) | (x<<8);
}
int main(void)
{
	CCP = 0xD8;
	CLKPSR = 0;	
	PUEB |= 0b1000;
	PCMSK |= 0b1000;
	PCICR = (1<<PCIE0);
	sei();
	while (1)
	{
		if(padern == 0){
			fadedown();
			fadeup();
		}else if (padern == 1){
			morse(message);
		} else if (padern == 2){
			for(int i = randn();i > 0;i--)all();
			for(int i = x;i > 0;i--)_delay_loop_1(50);
		}else if (padern == 3);
		 else{
			padern = 0;
		}
	}
}
ISR(PCINT0_vect){
	if((PINB&0b1000) == 0)padern++;
	for(uint16_t debounce = 4000;debounce>0;debounce--)_delay_loop_1(0);
}