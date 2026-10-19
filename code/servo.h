#ifndef SERVO_H
#define SERVO_H

void change_PWM_receivers(void);
typedef enum {MAGNETIZED, DEMAGNETIZED, STABLE, STEP} ServoState;
typedef enum {IRONY, CLEAN} PixelState;
ServoState servo_states[6][8];
uint8_t magnetized_count[7];
void group_1_servo_select(int);
void group_2_servo_select(int);
void group_3_servo_select(int);
void group_4_servo_select(int);
void group_5_servo_select(int);
void group_6_servo_select(int);
void group_7_servo_select(int);
void check_magnetized_count(void);
void servo_magnetize(int);
void servo_demagnetize(int);
ServoState get_servo_state(int);
void demagnetize_all(void);
void magnetize_all(void);


void set_demuxes(ServoState, bool*);
void pwm_select(ServoState);
void set_states(void);

#endif