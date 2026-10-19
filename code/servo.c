#include <avr/io.h>
#include <stdbool.h>
#include "servo.h"

ServoState servo_states[6][8] = {
	{DEMAGNETIZED, DEMAGNETIZED, DEMAGNETIZED, DEMAGNETIZED, DEMAGNETIZED, DEMAGNETIZED, DEMAGNETIZED, MAGNETIZED},
	{DEMAGNETIZED, DEMAGNETIZED, DEMAGNETIZED, DEMAGNETIZED, DEMAGNETIZED, DEMAGNETIZED, DEMAGNETIZED, MAGNETIZED},
	{DEMAGNETIZED, DEMAGNETIZED, DEMAGNETIZED, DEMAGNETIZED, DEMAGNETIZED, DEMAGNETIZED, DEMAGNETIZED, MAGNETIZED},
	{DEMAGNETIZED, DEMAGNETIZED, DEMAGNETIZED, DEMAGNETIZED, DEMAGNETIZED, DEMAGNETIZED, DEMAGNETIZED, MAGNETIZED},
	{DEMAGNETIZED, DEMAGNETIZED, DEMAGNETIZED, DEMAGNETIZED, DEMAGNETIZED, DEMAGNETIZED, DEMAGNETIZED, MAGNETIZED},
	{DEMAGNETIZED, DEMAGNETIZED, DEMAGNETIZED, DEMAGNETIZED, DEMAGNETIZED, DEMAGNETIZED, DEMAGNETIZED, MAGNETIZED}}; // kõik on mittemagneetunud (false)
		
int demux_to_display_mapping[6][8][2] = {
	{{1, 1}, {1, 0}, {1, 3}, {1, 2}, {1, 6}, {1, 4}, {1, 5}, {1, 7}},
	{{2, 1}, {2, 0}, {2, 3}, {2, 2}, {2, 5}, {2, 4}, {2, 7}, {2, 6}},
	{{3, 7}, {3, 6}, {3, 5}, {3, 4}, {3, 2}, {3, 3}, {3, 0}, {3, 1}},
	{{4, 1}, {4, 0}, {4, 3}, {4, 2}, {4, 5}, {4, 4}, {4, 7}, {4, 6}},
	{{5, 1}, {5, 0}, {5, 2}, {5, 3}, {5, 5}, {5, 4}, {5, 7}, {5, 6}},
	{{0, 1}, {0, 0}, {0, 3}, {0, 2}, {0, 5}, {0, 4}, {0, 7}, {0, 6}}};

int display_to_demux_mapping[6][8][2] = {
	{{5, 1}, {5, 0}, {5, 3}, {5, 2}, {5, 5}, {5, 4}, {5, 7}, {5, 6}},
	{{0, 1}, {0, 0}, {0, 3}, {0, 2}, {0, 5}, {0, 6}, {0, 4}, {0, 7}},
	{{1, 1}, {1, 0}, {1, 3}, {1, 2}, {1, 5}, {1, 4}, {1, 7}, {1, 6}},
	{{2, 6}, {2, 7}, {2, 4}, {2, 5}, {2, 3}, {2, 2}, {2, 1}, {2, 0}},
	{{3, 1}, {3, 0}, {3, 3}, {3, 2}, {3, 5}, {3, 4}, {3, 7}, {3, 6}},
	{{4, 1}, {4, 0}, {4, 2}, {4, 3}, {4, 4}, {4, 5}, {4, 7}, {4, 6}}}; // {x, y} - x = demux index, y = index in demux
	
bool invertebrates[6][8] = {
	{false, false, false, false, false, false, false, false},
	{false, false, true, false, false, false, false, false},
	{false, false, false, false, false, false, true, false},
	{false, false, true, false, false, false, false, false},
	{false, false, false, false, false, false, false, false},
	{false, false, false, false, false, false, false, false}};
		
ServoState current_PWM_select = DEMAGNETIZED;

void group_1_servo_select(int servo_nr) {
	if (servo_nr >= 4) {
		PORTF |= (1 << PF2);
		} else {
		PORTF &= ~(1 << PF2);
	}
	if (servo_nr == 2 || servo_nr == 3 || servo_nr == 4 || servo_nr == 7) {
		PORTF |= (1 << PF1);
		} else {
		PORTF &= ~(1 << PF1);
	}
	if (servo_nr % 2 == 1) {
		PORTF |= (1 << PF0);
		} else {
		PORTF &= ~(1 << PF0);
	}
}

void group_2_servo_select(int servo_nr) {
	if (servo_nr >= 4) {
		PORTF |= (1 << PF5);
		} else {
		PORTF &= ~(1 << PF5);
	}
	if (servo_nr == 2 || servo_nr == 3 || servo_nr == 6 || servo_nr == 7) {
		PORTF |= (1 << PF4);
		} else {
		PORTF &= ~(1 << PF4);
	}
	if (servo_nr % 2 == 1) {
		PORTF |= (1 << PF3);
		} else {
		PORTF &= ~(1 << PF3);
	}
}

void group_3_servo_select(int servo_nr) {
	if (servo_nr <= 3) {
		PORTA |= (1 << PA0);
		} else {
		PORTA &= ~(1 << PA0);
	}
	if (servo_nr == 5 || servo_nr == 4 || servo_nr == 1 || servo_nr == 0) {
		PORTF |= (1 << PF7);
		} else {
		PORTF &= ~(1 << PF7);
	}
	if (servo_nr % 2 == 0) {
		PORTF |= (1 << PF6);
		} else {
		PORTF &= ~(1 << PF6);
	}
}

void group_4_servo_select(int servo_nr) {
	if (servo_nr >= 4) {
		PORTA |= (1 << PA3);
		} else {
		PORTA &= ~(1 << PA3);
	}
	if (servo_nr == 2 || servo_nr == 3 || servo_nr == 6 || servo_nr == 7) {
		PORTA |= (1 << PA2);
		} else {
		PORTA &= ~(1 << PA2);
	}
	if (servo_nr % 2 == 1) {
		PORTA |= (1 << PA1);
		} else {
		PORTA &= ~(1 << PA1);
	}
}

void group_5_servo_select(int servo_nr) {
	if (servo_nr >= 4) {
		PORTE |= (1 << PE0);
		} else {
		PORTE &= ~((1 << PE0) | (1 << PE2)); // PE2 is HWB and if we wrote a 1 to there then PE2 internal pull up would be activated, and we may not want that
	}
	if (servo_nr == 2 || servo_nr == 3 || servo_nr == 6 || servo_nr == 7) {
		PORTE |= (1 << PE1);
		} else {
		PORTE &= ~((1 << PE1) | (1 << PE2)); // PE2 is HWB and if we wrote a 1 to there then PE2 internal pull up would be activated, and we may not want that
	}
	if (servo_nr % 2 == 1) { // A0 = PC0, A1 = PE1, A2 = PE0
		PORTC |= (1 << PC0);
		} else {
		PORTC &= ~(1 << PC0);
	}
}

void group_6_servo_select(int servo_nr) {
	if (servo_nr >= 4) {
		PORTD |= (1 << PD5);
		} else {
		PORTD &= ~(1 << PD5);
	}
	if (servo_nr == 2 || servo_nr == 3 || servo_nr == 6 || servo_nr == 7) {
		PORTD |= (1 << PD6);
		} else {
		PORTD &= ~(1 << PD6);
	}
	if (servo_nr % 2 == 1) {
		PORTD |= (1 << PD7);
		} else {
		PORTD &= ~(1 << PD7);
	}
}

void group_7_servo_select(int servo_nr) {
	if (servo_nr >= 4) {
		PORTD |= (1 << PD2);
	} else {
		PORTD &= ~(1 << PD2);
	}
	if (servo_nr == 2 || servo_nr == 3 || servo_nr == 6 || servo_nr == 7) {
		PORTD |= (1 << PD3);
	} else {
		PORTD &= ~(1 << PD3);
	}
	if (servo_nr % 2 == 1) {
		PORTD |= (1 << PD4);
	} else {
		PORTD &= ~(1 << PD4);
	}
}

void (*group_servo_select[7])(int) = {group_1_servo_select, group_2_servo_select, group_3_servo_select, group_4_servo_select,
group_5_servo_select, group_6_servo_select, group_7_servo_select};




ServoState get_servo_state(int servo_nr) {
	servo_nr -= 1;
	return servo_states[(int)((servo_nr-(servo_nr%8))/8)][(int)((servo_nr)%8)];
}

void demagnetize_all(void) {
	for (int i = 0; i < 7; i++) {
		for (int j = 0; j < 8; j++) {
			servo_states[i][j] = DEMAGNETIZED;
		}
	}
}

void magnetize_all(void) {
	for (int i = 0; i < 7; i++) {
		for (int j = 0; j < 8; j++) {
			servo_states[i][j] = MAGNETIZED;
		}
	}
}


ServoState servo_positions[6][8] = {
			{STABLE, STABLE, STABLE, STABLE, STABLE, STABLE, STABLE, STABLE}, // vasakpoolseim on kõige alumine servo
			{STABLE, STABLE, STABLE, STABLE, STABLE, STABLE, STABLE, STABLE}, // veeru nullinda indeksi servo on displei vaataja poolelt vasakpoolseim
			{STABLE, STABLE, STABLE, STABLE, STABLE, STABLE, STABLE, STABLE},
			{STABLE, STABLE, STABLE, STABLE, STABLE, STABLE, STABLE, STABLE},
			{STABLE, STABLE, STABLE, STABLE, STABLE, STABLE, STABLE, STABLE},
			{STABLE, STABLE, STABLE, STABLE, STABLE, STABLE, STABLE, STABLE}};

/*
PixelState servo_dreams[6][8] = {
			{CLEAN, CLEAN, IRONY, CLEAN, CLEAN, CLEAN, CLEAN, CLEAN},
			{CLEAN, CLEAN, CLEAN, IRONY, CLEAN, CLEAN, CLEAN, CLEAN},
			{CLEAN, CLEAN, CLEAN, CLEAN, IRONY, CLEAN, CLEAN, CLEAN},
			{CLEAN, CLEAN, CLEAN, CLEAN, CLEAN, IRONY, CLEAN, CLEAN},
			{CLEAN, CLEAN, CLEAN, CLEAN, CLEAN, CLEAN, IRONY, CLEAN},
			{CLEAN, CLEAN, CLEAN, CLEAN, CLEAN, CLEAN, CLEAN, IRONY}};
				*/

/*	
PixelState servo_dreams[6][8] = {
			{CLEAN, CLEAN, CLEAN, CLEAN, CLEAN, CLEAN, CLEAN, IRONY},
			{CLEAN, CLEAN, CLEAN, CLEAN, CLEAN, CLEAN, IRONY, CLEAN},
			{CLEAN, CLEAN, CLEAN, CLEAN, CLEAN, IRONY, CLEAN, CLEAN},
			{CLEAN, CLEAN, CLEAN, CLEAN, IRONY, CLEAN, CLEAN, CLEAN},
			{CLEAN, CLEAN, CLEAN, IRONY, CLEAN, CLEAN, CLEAN, CLEAN},
			{CLEAN, CLEAN, IRONY, CLEAN, CLEAN, CLEAN, CLEAN, CLEAN}};
*/

PixelState servo_dreams[6][8] = {
	{CLEAN, CLEAN, CLEAN, CLEAN, IRONY, CLEAN, CLEAN, IRONY},
	{CLEAN, CLEAN, CLEAN, IRONY, CLEAN, CLEAN, IRONY, CLEAN},
	{CLEAN, CLEAN, IRONY, CLEAN, CLEAN, IRONY, CLEAN, CLEAN},
	{CLEAN, IRONY, CLEAN, CLEAN, IRONY, CLEAN, CLEAN, CLEAN},
	{IRONY, CLEAN, CLEAN, IRONY, CLEAN, CLEAN, CLEAN, CLEAN},
	{CLEAN, CLEAN, IRONY, CLEAN, CLEAN, CLEAN, CLEAN, CLEAN}};

PixelState pixel_states[6][8] = {
			{CLEAN, CLEAN, CLEAN, CLEAN, CLEAN, CLEAN, CLEAN, CLEAN},
			{CLEAN, CLEAN, CLEAN, CLEAN, CLEAN, CLEAN, CLEAN, CLEAN},
			{CLEAN, CLEAN, CLEAN, CLEAN, CLEAN, CLEAN, CLEAN, CLEAN},
			{CLEAN, CLEAN, CLEAN, CLEAN, CLEAN, CLEAN, CLEAN, CLEAN},
			{CLEAN, CLEAN, CLEAN, CLEAN, CLEAN, CLEAN, CLEAN, CLEAN},
			{CLEAN, CLEAN, CLEAN, CLEAN, CLEAN, CLEAN, CLEAN, CLEAN}};

int throwaway_servos[7] = {1, 1, 6, 1, 1, 1, 0}; //  ?  ?  ?  7.  8.  1.  



void set_demuxes(ServoState olek, bool* leiti_olek){
	for(int i = 0; i < 6; i++){
		bool leitud = false;
		
		 
		for(int j = 0; j<8; j++){
			ServoState tegelik_olek = olek;
			if(invertebrates[demux_to_display_mapping[i][j][0]][demux_to_display_mapping[i][j][1]]){
				tegelik_olek = (olek == MAGNETIZED) ? DEMAGNETIZED : MAGNETIZED;
			}
			
			
			if (servo_states[i][j] == tegelik_olek){
				group_servo_select[i](j);
				servo_states[i][j] = STABLE;
				//servo_positions[demux_to_display_mapping[i][j][0]][demux_to_display_mapping[i][j][1]] = tegelik_olek;
				//if (olek == MAGNETIZED || (olek == MAGNETIZED && ))
				pixel_states[demux_to_display_mapping[i][j][0]][demux_to_display_mapping[i][j][1]] = (tegelik_olek == MAGNETIZED) ? IRONY : CLEAN;
				leitud = true;
				*leiti_olek = true;
				break;
			}
		}
		
		if(!leitud){ // enne throwaway vaatab, kasäkki mõni on õiges olekus
			PixelState pixel_olek = (olek == MAGNETIZED) ? IRONY : CLEAN;
			for (int j=0; j < 8; j++){
				PixelState tegelik_olek = pixel_olek;
				if(invertebrates[demux_to_display_mapping[i][j][0]][demux_to_display_mapping[i][j][1]]){
					tegelik_olek = (pixel_olek == IRONY) ? CLEAN : IRONY;
				}
				if (pixel_states[demux_to_display_mapping[i][j][0]][demux_to_display_mapping[i][j][1]] == tegelik_olek){
					group_servo_select[i](j);
					leitud = true;
					break;
				}
			}
		}
		if(!leitud){
			group_servo_select[i](throwaway_servos[i]); // siia peab minema default igast demuxist
		}
	}
}

void pwm_select(ServoState olek){
	if (olek == MAGNETIZED){
		PORTC &= ~((1 << PC6) | (1 << PC2));
	}
	else if (olek == DEMAGNETIZED){
		PORTC |= (1 << PC6);
		PORTC &= ~(1 << PC2);
	}
	else if (olek == STABLE){
		PORTC |= ((1 << PC6) | (1 << PC2));
	} else if (olek == STEP) {
		PORTC |= (1 << PC2);
		PORTC &= ~(1 << PC6);
	}
}






void set_states(void){ 
	for (int i = 0; i < 6; i++) {
		for (int j = 7; j >= 0; j--) { // alustame kõige ülemisest servost
			PixelState dream = servo_dreams[i][j];
			PixelState reality = pixel_states[i][j];
			if (dream == reality) {
				continue; // the servo has reached its dream
			}
			
			else if (dream == IRONY && reality == CLEAN) {
				if(j==0){
					servo_states[display_to_demux_mapping[i][j][0]][display_to_demux_mapping[i][j][1]] = MAGNETIZED;	
					continue;
				}
				
				bool found_iron = false;
				for (int k = j; k >= 0; k--) {
					if (pixel_states[i][k] == IRONY) {
						found_iron = true;
						servo_states[display_to_demux_mapping[i][k][0]][display_to_demux_mapping[i][k][1]] = STABLE; // see juurde, sest vb muidu kuskil see satub valeks

						if (k != 0 && pixel_states[i][k-1] == IRONY) {
							servo_states[display_to_demux_mapping[i][k-1][0]][display_to_demux_mapping[i][k-1][1]] = DEMAGNETIZED;								
						}
						
						servo_states[display_to_demux_mapping[i][k+1][0]][display_to_demux_mapping[i][k+1][1]] = MAGNETIZED;
						
						break;
					}
				}
				if (!found_iron) {
					servo_states[display_to_demux_mapping[i][0][0]][display_to_demux_mapping[i][0][1]] = MAGNETIZED;
					
					//pixel_states[i][0] = IRONY;
	
				}
				break;
			}
			
			else if (dream == CLEAN && reality == IRONY) {
				servo_states[display_to_demux_mapping[i][j][0]][display_to_demux_mapping[i][j][1]] = DEMAGNETIZED;
				
				//pixel_states[i][j] = CLEAN;
			}
		}
	}
}
