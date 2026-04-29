#ifndef INC_TIMER_H_
#define INC_TIMER_H_

void tim3_pa6_1mhz_init(void);
#define SR_CC1IF (1U<<1)
#define CCER_CC1P (1U<<1)
#define CCER_CC1NP (1U<<3)

extern volatile uint8_t measurement_ready;
extern volatile uint32_t pulse_width;
extern volatile uint32_t pulse_period;
extern volatile uint32_t rising_previous;

#endif /* INC_TIMER_H_ */
