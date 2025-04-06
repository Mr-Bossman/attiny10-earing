#define F_CPU 8000000UL

#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>

#define ogseed 6203

struct table {
	unsigned D:3;
	unsigned P:3;
};

static const uint8_t message[] = {0b10000000, 0b10010101, 0b11000000};
static const struct table led_lut[] PROGMEM = {{0b101, 0b100}, {0b110, 0b010}, {0b011, 0b010},
					       {0b011, 0b001}, {0b101, 0b001}, {0b110, 0b100}};
static const uint8_t sinewave[] PROGMEM = {0, 0, 1, 3, 9, 19, 36, 65, 105, 154, 203, 240, 255,
					     240, 203, 154, 105, 65, 36, 19, 9, 3, 1, 0, 0};
static volatile uint8_t pattern = 0;
/*
import java.lang.Math;
public class Main
{
	public static void main(String[] args) {
		System.out.print("{");
		for (double x = 0; x < 2.2; x += 0.5) {
			System.out.print((int)(Math.pow(2, Math.sin(x * (Math.PI/5)) * 8) - 1));
			System.out.print(", ");
		}
		System.out.print((int)(Math.pow(2, Math.sin(2.4 * (Math.PI/5)) * 8) - 1));
		System.out.print("}");
	}
}
*/

static void brightness(uint8_t val, uint8_t led) {
	uint8_t top = 255;

	while (top--) {
		DDRB = 0;
		PORTB = 0;
		if (top < val){
			DDRB = led_lut[led].D;
			PORTB = led_lut[led].P;
		}
	}
	DDRB = 0;
	PORTB = 0;
}

static void fadedown(void) {
	for(uint8_t i = 0; i < 12; i++) {
		uint8_t n = 20;
		while (n--) {
			brightness(sinewave[0 + i], 0);
			brightness(sinewave[2 + i], 1);
			brightness(sinewave[4 + i], 2);
			brightness(sinewave[6 + i], 3);
			brightness(sinewave[8 + i], 4);
			brightness(sinewave[10 + i], 5);
		}
	}
}

static void fadeup(void) {
	for (uint8_t i = 0; i < 12; i++) {
		uint8_t n = 20;
		while (n--) {
			brightness(sinewave[0 + i], 5);
			brightness(sinewave[2 + i], 4);
			brightness(sinewave[4 + i], 3);
			brightness(sinewave[6 + i], 2);
			brightness(sinewave[8 + i], 1);
			brightness(sinewave[10 + i], 0);
		}
	}

}

static void set(uint8_t led) {
	DDRB = 0;
	PORTB = 0;
	DDRB = led_lut[led].D;
	PORTB = led_lut[led].P;
}

static void all(void) {
	for (uint8_t led = 0; led < 6; led++) {
		DDRB = 0;
		PORTB = 0;
		DDRB = led_lut[led].D;
		PORTB = led_lut[led].P;
	}
	DDRB = 0;
	PORTB = 0;
}

static void delay1(void) {
	__builtin_avr_delay_cycles(1024000);
}

static void all1(void) {
	for(uint16_t timeon = 4000; timeon > 0; timeon--)
		all();
}

static void morse(void) {
	for (uint8_t byte = 0; byte < sizeof(message); byte++) {
		uint8_t bits = message[byte];
		for (uint8_t bit = 0; bit < 4; bit++) {
			if ((bits & 3) == 0) {
				all1();
				delay1();
			} else if ((bits & 3) == 1) {
				all1();
				all1();
				all1();
				delay1();
			} else if ((bits & 3) == 2) {
				delay1();
				delay1();
			} else {
				delay1();
				delay1();
				delay1();
			}
			bits >>= 2;
		}
	}
}

static uint16_t randn(void) {
	static uint16_t x = 0, w = 0;

	x *= x;
	x += (w += ogseed);
	return x = (x>>8) | (x<<8);
}

static void loop(void) {
	for (uint8_t i = 0; i < 6; i++) {
		set(i);
		delay1();
	}
}

int main(void) {
	CCP = 0xD8;
	CLKPSR = 0;
	PUEB |= 0b1000;
	PCMSK |= 0b1000;
	PCICR = (1<<PCIE0);
	sei();

	while (1) {
		if (pattern == 0){
			loop();
		} else if (pattern == 1) {
			morse();
		} else if (pattern == 2) {
			uint16_t temp = randn();
			for (uint16_t i = temp; i > 0; i--)
				all();
			for (uint16_t i = temp; i > 0; i--)
				_delay_loop_1(50);
		} else if (pattern == 3) {
			fadedown();
			fadeup();
		} else if (pattern == 4) {
		} else {
			pattern = 0;
		}
	}
}

ISR(PCINT0_vect){
	if ((PINB & 0b1000) == 0)
		pattern++;
	delay1();
}
