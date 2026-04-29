#include "as5600.h"

as5600 encoder;

float lp_filter(float signal, float LP_cutoff, float delta_t)
{
	float alpha = 2 * PI * delta_t * LP_cutoff / (1 + 2 * PI * delta_t * LP_cutoff);
	static float filtered_signal;

	filtered_signal = alpha * signal + (1-alpha) * filtered_signal;

	return filtered_signal;
}

void as5600_pwm_to_angle(void)
{
	encoder.angle_previous = encoder.angle;
	encoder.angle = ((float)pulse_width / (float)pulse_period) * 360.0f;
}

void as5600_calculate_speed(void)
{
	// Raw angular speed calculation, current - previous angle
	float angular_speed_raw = (encoder.angle - encoder.angle_previous) / ((float)pulse_period / 1e6f);	// Converting pulse width from us to s to get [deg/s]

	// Applying LP filter to raw speed in order to filter amplified noise from encoder (amplified by derivation)
	encoder.angular_speed = lp_filter(angular_speed_raw, SPEED_LP_CUTOFF, (float)pulse_period / 1e6f);
}
