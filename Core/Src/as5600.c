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

	// Remove the 128 tick bias so 0 degrees maps to 0 and 360 maps to 360
	// AS5600 PWM cycle: [128 always LOW][4095 angle][128 always HIGH] = 4351 total
	// Even at 0 degrees the duty cycle is ~3%, never fully LOW.
	// Even at 360 degrees the duty cycle is ~97%, never fully HIGH.
	// This way we can always tell the sensor is alive and outputting.
	float duty = (float)pulse_width / (float)period;
	float raw = duty * AS5600_DCL_TOTAL - AS5600_DCL_PADDING;

	if (raw < 0.0f) raw = 0.0f;
	if (raw > AS5600_DCL_ANGLE) raw = AS5600_DCL_ANGLE;

	float angle = (raw / AS5600_DCL_ANGLE) * 360.0f;

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
