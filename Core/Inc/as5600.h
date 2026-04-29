#ifndef INC_AS5600_H_
#define INC_AS5600_H_

#define PI 3.14159265f

#define SPEED_LP_CUTOFF 150 // [Hz]

extern volatile uint32_t pulse_width;
extern volatile uint32_t pulse_period;

void as5600_pwm_to_angle(void);
void as5600_calculate_speed(void);

typedef struct {
    float angle;
    float angle_previous;
    float angular_speed;
} as5600;

extern as5600 encoder;

#endif /* INC_AS5600_H_ */
