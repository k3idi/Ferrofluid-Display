/*
 * rvp_v4.c
 *
 * Created: 5/3/2026 12:16:16 PM
 * Author : Keidi
 */ 

#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdbool.h>
#include "servo.h"

#define PWM_period_ticks 40000
#define PWM_period_us 20000

uint16_t duty_cycle_magnetize;
uint16_t PWM_high_ticks_magnetize;
uint16_t duty_cycle_demagnetize;
uint16_t PWM_high_ticks_demagnetize;
uint16_t duty_cycle_step;
uint16_t PWM_high_ticks_step;

bool toggle_lipp = false;

void init_timer(void) {
	TIMSK3 = 0b00001110; // -- 0 - 1 1 1 0 : output compare A, B and C interrupt enabled, while overflow interrupt disabled : – – ICIE3 – OCIE3C OCIE3B OCIE3A TOIE3
	// WGM = 0100 : CTC (clear timer on compare match) mode
	TCCR3A = 0b00000000; // 01 01 01 00 COM3A1:COM3A0 COM3B1:COM3B0 COM3C1:COM3C0 WGM31:WGM30
	TCCR3B = 0b00001000; // 00 - 01 000 : the clock is stopped right now (000) ICNC3:ICES3 - WGM33:WGM32 CS32:CS31:CS30
	TCCR3C = 0b00000000; // nothing cool, just force output compare for all channels disabled or whatever
	TCNT3 = 0; // clear timer
	OCR3C = PWM_period_ticks; // controls PWM frequency
	OCR3A = PWM_high_ticks_magnetize; // controls the time that PWMa is high
	OCR3B = PWM_high_ticks_step; // controls the time that PWMb is high
	PORTC |= (1 << PC1) | (1 << PC5) | (1 << PC4); // set PWMa and PWMb high
	TCCR3B = 0b00000001; // start the timer
}

void init_pins(void) {
	DDRA = 0; DDRB = 0; DDRC = 0; DDRD = 0; DDRE = 0; DDRF = 0;
	PORTA = 0; PORTB = 0; PORTC = 0; PORTD = 0; PORTE = 0; PORTF = 0;
	DDRA = (1 << PA0) | (1 << PA1) | (1 << PA2) | (1 << PA3); // demux address controls to output
	// PWM_select_1 = PC6, PWM_select_2 = PC2, PWM_select_enable = PC3, PWMa (S1) = PC5, PWMb (S2) = PC1
	DDRC = (1 << PC5) | (1 << PC1) | (1 << PC4) | (1 << PC0) | (1 << PC6) | (1 << PC2) | (1 << PC3); // PWMa, PWMb and PWMc, DEMUX_5_A0 and PWM_select_1, PWM_select_2 and PWM_select_EN
	DDRD = (1 << PD2) | (1 << PD3) | (1 << PD4) | (1 << PD5) | (1 << PD6) | (1 << PD7); // demux address controls to output
	DDRE = (1 << PE0) | (1 << PE1); // demux address controls to output
	DDRF = (1 << PF0) | (1 << PF1) | (1 << PF2) | (1 << PF3) | (1 << PF4) | (1 << PF5) | (1 << PF6) | (1 << PF7); // demux address controls to output
	PORTC |= (1 << PC3); // enable MUX output (PC3), select PWMa (S1) by not setting any select pins high
}

void init_constants(void) {
	// from data sheet: 1-2 ms duty cycle
	duty_cycle_magnetize = 1000;
	float high_ticks_magnetize_float = (duty_cycle_magnetize * PWM_period_ticks) / PWM_period_us;
	PWM_high_ticks_magnetize = (uint16_t) high_ticks_magnetize_float;
	
	duty_cycle_step = 1500; 
	float high_ticks_step_float = (duty_cycle_step * PWM_period_ticks) / PWM_period_us;
	PWM_high_ticks_step = (uint16_t) high_ticks_step_float;
	
	duty_cycle_demagnetize = 2000; 
	float high_ticks_demagnetize_float = (duty_cycle_demagnetize * PWM_period_ticks) / PWM_period_us;
	PWM_high_ticks_demagnetize = (uint16_t) high_ticks_demagnetize_float;
}

volatile uint32_t cycle_count = 0;
volatile uint32_t cycle_count_2 = 0;
uint8_t PWM_counter = 40;
// potential problem with COMPA and COMPB interrupts: the interrupt triggers always when the TCNT value is higher than OCR3A or OCR3B. this shouldn't
// happen, because in the datasheet it is written on page 124 that a match is found when they are equal, not equal or higher, but if this is wrong
// then we just have to set the OCR3A and OCR3B values higher than OCR3C - OCR3A and OCR3C - OCR3B in their respective interrupts, and then set them 
// to the correct values again in the COMPC interrupt.
ISR (TIMER3_COMPA_vect) { 
	// set PWMa signal low
	PORTC &= ~(1 << PC5); // set PWMa signal low
}
bool stepping = true;
ISR (TIMER3_COMPB_vect) {
	// set PWMb or PWMc signal low
	if(stepping){
		PORTC &= ~(1 << PC4); // set PWMc signal low
		stepping = false;
		OCR3B = PWM_high_ticks_demagnetize;
	}
	else{
		PORTC &= ~(1 << PC1); // set PWMb signal low
	}
}

ISR (TIMER3_COMPC_vect) {
	// controls the frequency of PWMk: sets timer value to 0 and sets PWMa and PWMb high
	TCCR3B = 0b00000000; // stop the timer
	TCNT3 = 0; // clear timer
	TIFR3 = 0; // clear all flags
	TCCR3B = 0b00000001; // start the timer
	PORTC |= (1 << PC1) | (1 << PC5) | (1 << PC4); // set PWMb and PWMa high
	OCR3B = PWM_high_ticks_step;
	stepping = true;
	cycle_count += 1;
	cycle_count_2 += 1;
}

ServoState olek = DEMAGNETIZED;
uint8_t oleku_jrk = 0;
int main(void) {	
	MCUCR |= (1 << JTD); 
	MCUCR |= (1 << JTD);
	init_constants();
	init_pins();
	init_timer();
	DDRA = 0xFF;
	sei();

	demagnetize_all();
	cycle_count = 0;
	
    while (1) 
    {
		// initial setup for each pass
		oleku_jrk++;
		pwm_select(STABLE);
		PORTA ^= (1 << PA4); // is anything even happening check
		
		// change servo states
		/*
		if (cycle_count_2 >= change_positions_count) { 
			cycle_count_2 = 0;
			for (int i = 0; i < 6; i++) {
				for (int j = 0; j < 8; j++) {
					if (j == magnetized_nr || j+1 == magnetized_nr) {
						servo_states[column_mapping[i][j][0]][column_mapping[i][j][1]] = MAGNETIZED;	
					} else {
						servo_states[column_mapping[i][j][0]][column_mapping[i][j][1]] = DEMAGNETIZED;
					}
				}
			}
			magnetized_nr++;
			if (magnetized_nr >= 9) {
				magnetized_nr = -1;
			}
		}
		*/
		
		// servo control logic
		bool leiti_olek = false;
		set_demuxes(olek, &leiti_olek);
		if(leiti_olek){
			/*
			pwm_select(STEP);
			cycle_count = 0;
			while(cycle_count < PWM_counter);*/
			pwm_select(olek);
			cycle_count = 0;
			while(cycle_count < PWM_counter);
			pwm_select(STABLE);
		}
		
		if(oleku_jrk >= 8){
			set_states();
			//magnetize_all;
			olek = (olek == MAGNETIZED) ? DEMAGNETIZED : MAGNETIZED;
			oleku_jrk = 0;
		}
		
			
	
    }
}







