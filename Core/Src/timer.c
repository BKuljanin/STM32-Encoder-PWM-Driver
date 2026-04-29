#include "stm32f4xx.h"

#define GPIOAEN (1U<<0)
#define AFR6_TIM (1U<<25)
#define TIM3EN (1U<<1)
#define CCMR1_IN_CC1S (1U<<0)
#define CCMR1_IN_CC2S (1U<<9)
#define CCER_CC1E (1U<<0)
#define CCER_CC2E (1U<<4)
#define CCER_CC2P (1U<<6)
#define CR1_CEN	(1U<<0)
#define DIER_CC1IE (1U<<1)

void tim3_pa6_1mhz_init(void)
{

	// Enable clock access to GPIOA
	RCC->AHB1ENR |= GPIOAEN; // Datasheet p16 GPIOA is connected to AHB1

	GPIOA->MODER &=~ (1U<<12); // Reference manual p186 10-alternate function
	GPIOA->MODER |= (1U<<13);

	// Set PA6 alternate function as timer 3 channel 1
	GPIOA->AFR[0] |= AFR6_TIM; // Reference manual p190 low AFRL (low register since its PA6), AF02. Set bit 25 to 1

	// Enable clock access to tim3
	RCC->APB1ENR |= TIM3EN; // Reference manual p146 tim3 bit 1

	// Set prescaler
	TIM3->PSC = 84 - 1; // -1 because of counting from zero, 84 000 000 / 84 = 1 000 000 (1MHz)

	// Set CH1 to input mode
	TIM3->CCMR1 = CCMR1_IN_CC1S; // Reference manual p497, 498

	// Set CH2 to listen on same input as CH1
	TIM3->CCMR1 |= CCMR1_IN_CC2S;	// Reference manual p495

	// Initialize CH1 to capture at rising edge, default is rising edge
	TIM3->CCER |= CCER_CC1E; // Reference manual p499 capture/compare enable register, p501 CC1E see configuration for input, bit 0 capture enable

	// Initialize CH2 to capture at falling edge (to capture pulse width)
	TIM3->CCER |= CCER_CC2P | CCER_CC2E; // Reference manual p499, setting the polarity to falling edge

	// Enable TIM3
	TIM3->CR1 = CR1_CEN;

	// Enable TIM interrupt
	TIM3->DIER |= DIER_CC1IE; // Reference manual p491

	// Enable TIN interrupt in NVIC
	NVIC_EnableIRQ(TIM3_IRQn);
}
