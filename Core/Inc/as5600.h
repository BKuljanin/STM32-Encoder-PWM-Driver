#ifndef INC_AS5600_H_
#define INC_AS5600_H_

extern volatile uint32_t pulse_width;
extern volatile uint32_t pulse_period;

extern float angle;

void as5600_pwm_to_angle(void);

#endif /* INC_AS5600_H_ */
