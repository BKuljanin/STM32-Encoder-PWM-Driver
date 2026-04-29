#ifndef INC_TIMER_H_
#define INC_TIMER_H_

void tim3_pa6_1mhz_init(void);
#define SR_CC1IF (1U<<1)
#define CCER_CC1P (1U<<1)
#define CCER_CC1NP (1U<<3)

#endif /* INC_TIMER_H_ */
