#include "as5600.h"

float angle;

void as5600_pwm_to_angle(void)
{
	angle = ((float)pulse_width / (float)pulse_period) * 360.0f;
}
