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

	uint32_t period = pulse_period;
	if (period < 900) period = 900;

	float angle = ((float)pulse_width / (float)period) * 360.0f;

	if (angle > 360.0f) angle = 360.0f;
	if (angle < 0.0f) angle = 0.0f;

	encoder.angle = angle;
}

void as5600_calculate_speed(void)
{
	uint32_t period = pulse_period;
	if (period < 900) period = 900;

	// Wrap aware delta, if prev ~360 and current ~0 (or vice versa) compute actual small movement
	float delta_angle = encoder.angle - encoder.angle_previous;
	if (delta_angle > 180.0f) delta_angle -= 360.0f;
	if (delta_angle < -180.0f) delta_angle += 360.0f;

	float angular_speed_raw = delta_angle / ((float)period / 1e6f);	// Converting pulse width from us to s to get [deg/s]

	// Applying LP filter to raw speed in order to filter amplified noise from encoder (amplified by derivation)
	encoder.angular_speed = lp_filter(angular_speed_raw, SPEED_LP_CUTOFF, (float)period / 1e6f);
}
