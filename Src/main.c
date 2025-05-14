#include "stm32f4xx.h"
#include <ctype.h>

#define PPM_FRAME_LENGTH  22000  // 22 ms frame (45.45 Hz)
#define PPM_PULSE_WIDTH   9000   // PPM width for standby
#define PWM_FRAME_LENGTH  22000 // same as PPM
#define PWM_PULSE_WIDTH 9000 // PWM width for standby

void configure_ppm_timer3(void);
void configure_ppm_timer4(void);
void SystemClock_Config(void);
void USART2_Init(void);
void USART2_SendChar(char c);
void USART2_SendString(const char *str);
char USART2_ReceiveChar(void);


// Enable PPM signal on pin PC6 (on top right side of nucleo, the right column of morpho pins) using channel 1 of timer 3
void configure_ppm_timer3(void) {
    // Enable GPIO and Timer Clocks
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOCEN;
    RCC->APB1ENR |= RCC_APB1ENR_TIM3EN;

    // Configure PC6 (TIM3_CH1) as Alternate Function (AF2)
    GPIOC->MODER |= GPIO_MODER_MODER6_1;          // Alternate function mode
    GPIOC->AFR[0] &= ~(0xF << (6 * 4));          // Clear AF for PC6
    GPIOC->AFR[0] |= (2 << (6 * 4));             // AF2 (TIM3_CH1)

    // Configure TIM3 for PPM
    TIM3->PSC = 15;                              // 1 MHz timer (16 MHz / 16)
    TIM3->ARR = PPM_FRAME_LENGTH - 1;            // 22 ms frame (22000 µs)
    TIM3->CCR1 = PPM_PULSE_WIDTH;                // 1.5 ms pulse
    TIM3->CCMR1 |= TIM_CCMR1_OC1M_1 | TIM_CCMR1_OC1M_2 | TIM_CCMR1_OC1PE; // PWM1 mode + preload
    TIM3->CCER |= TIM_CCER_CC1E;                 // Enable CH1 output
    TIM3->CR1 |= TIM_CR1_ARPE | TIM_CR1_CEN;     // Enable auto-reload & start timer
}

// Enable PWM signal on pin PB6 (on middle right side of nucleo, the left column of morpho pins) using channel 1 of timer 4.
// Need to keep some distance between PWM signal generating pins to minimize emf interference.
void configure_pwm_timer4(void) {
    // Enable GPIO and Timer Clocks
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOBEN;
    RCC->APB1ENR |= RCC_APB1ENR_TIM4EN;

    // Configure PB6 (TIM4_CH1) as Alternate Function (AF2)
    GPIOB->MODER |= GPIO_MODER_MODER6_1;         // Alternate function mode
    GPIOB->AFR[0] &= ~(0xF << (6 * 4));          // Clear AF for PC6
    GPIOB->AFR[0] |= (2 << (6 * 4));             // AF2 (TIM3_CH1)

    // Configure Timer 4 for PWM
    TIM4->PSC = 15;                              // 1 MHz timer (16 MHz / 16)
    TIM4->ARR = PWM_FRAME_LENGTH - 1;            // 22 ms frame (22000 µs)
    TIM4->CCR1 = PWM_PULSE_WIDTH;                // Initialize Duty Cycle to 0
    TIM4->CCMR1 |= TIM_CCMR1_OC1M_1 | TIM_CCMR1_OC1M_2 | TIM_CCMR1_OC1PE; // PWM1 mode + preload
    TIM4->CCER |= TIM_CCER_CC1E;                 // Enable CH1 output
    TIM4->CR1 |= TIM_CR1_ARPE | TIM_CR1_CEN;     // Enable auto-reload & start timer
}


//Configuring UART for message transmission on STM32F411CET


void SystemClock_Config(void) {
    // We will configure the main system clock to 100 MHz using the STM32's internal HSI oscillator

    FLASH->ACR |= FLASH_ACR_LATENCY_3WS; /* STM32 flash memory has certain access time, at higher clock speeds, cpu might read instructions faster
     	 	 	 	 	 	 	 	 	 	than flash can respond. For 100 Mhz, 3 wait states are required per datasheet*/

    /* RCC is the Reset and Clock Control peripheral, RCC->CR is the Reset Clock Control Register which is used for clock management.
    i.e. which oscillator to use: HSI (16 MHz High Speed Internal oscillator) or HSE (High Speed External oscillator).
    Also controls ready flags for HSI/HSE and PLL (Phase Locked Loop) */

    RCC->CR |= RCC_CR_HSION; // Enable the high speed 16 Mhz oscillator (HSI) on the STM32F411VET
    while(!(RCC->CR & RCC_CR_HSIRDY)); // Wait until oscillator stabilizes: HSIRDY is status flag bit indicating HSI is stable

    RCC->PLLCFGR = (16 << RCC_PLLCFGR_PLLM_Pos) |  // PLLM = 16 (PLLM is divider so divide 16 MHz HSI clock by 16 to get 1 MHz)
                   (200 << RCC_PLLCFGR_PLLN_Pos) | // PLLN = 200 (PLLN is multiplier, i.e. 1 * 200 = 200 Mhz, PLLN must be between 192 and 432 when using USB)
                   (0 << RCC_PLLCFGR_PLLP_Pos);    // PLLP = 2 (PLLP is divider, 0 in register = divide by 2 to get 100 Mhz
    // Hence, PLL output = (HSI / PLLM) * PLLN / PLLP = (16/16)*200/2 = 100 MHz

    RCC->CR |= RCC_CR_PLLON; // Set this bit to enable PLL (Phase Locked Loop)
    while(!(RCC->CR & RCC_CR_PLLRDY)); // Wait for PLL Lock

    RCC->CFGR |= RCC_CFGR_SW_PLL; // Switches to PLL (from HSI) as system clock source
    while((RCC->CFGR & RCC_CFGR_SWS_Msk) != RCC_CFGR_SWS_PLL); // Confirms that system clock source is switched to PLL
    														   // CFGR is clock configuration register
        													   // CFGR_SWS_Msk is bitmask that (SWS[1:0] bits)
        													   // CFGR_SWS_PLL is value of PLL which represents the current clock source
    														   // Loops until SWS[1:0] bits matches SWS_PLL, confirming that PLL is system clock

    RCC->CFGR |= RCC_CFGR_PPRE1_DIV2 | RCC_CFGR_PPRE2_DIV1;    // Sets APB1 prescaler to /2 (50 MHz), APB2 to /1 (100 MHz)
}

void USART2_Init(void) {
    // Enable peripheral clocks
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN; // Enable GPIOA clock (which enables PA2 and PA3 which is used by USART2
    RCC->APB1ENR |= RCC_APB1ENR_USART2EN; // Enable USART2 clock

    // Configure GPIO pins
    GPIOA->MODER &= ~(GPIO_MODER_MODER2 | GPIO_MODER_MODER3); // Clear mode bits associated with PA2 and PA3,
    														  // MODER2 and MODER3 are bitmasks that target those bits
    GPIOA->MODER |= (2 << GPIO_MODER_MODER2_Pos) |  // Set the starting bit position for PA2 to 2 -> PA2 Alternate function mode
                    (2 << GPIO_MODER_MODER3_Pos);    // Same as above for PA3

    // Configures alternate function of of PA2 (TX) and PA3 (RX) to AF7 which maps to USART2.
    GPIOA->AFR[0] &= ~(0xFF << 8); // AFR[0] controls alternate function for pins PA0 to PA7
    							   // Each pin uses 4 bits in the AFR register to specify its alternate function
    							   // PA2 uses bits 8-11 and PA3 uses bits 12-15, hence, 0XFF creates a mask targeting bits 8-15
    							   // &=~(...) clears these bits

    // USART2 maps to the AF7 bit (i.e. 7 -> 0b0111) on the Alternate Function Register
    // Refer to datasheet for numbering but it is simply numeric (i.e. AF1 -> 0b0001, AF2 -> 0b0011, etc)
    GPIOA->AFR[0] |= (7 << 8) |    // Sets start bit of PA2 in AFR register to AF7 (USART2_TX)
                     (7 << 12);    // Sets start bit of PA3 in AFR register to AF7 (USART2_RX)


    // Configure USART2 baud rate using BRR register and enable
    USART2->BRR = 0x1B2; // 50 MHz / (16 * 115200) ≈ 27.1267 → 0x1B2
    USART2->CR1 = USART_CR1_TE |   // Transmitter enable
                  USART_CR1_RE |   // Receiver enable
                  USART_CR1_UE;    // USART enable
}

void USART2_SendChar(char ch) {
    while(!(USART2->SR & USART_SR_TXE)); // Wait for transmit buffer empty
    USART2->DR = ch; // Set first free value of transmit buffer to input character ch
}

void USART2_SendString(const char *str) {
    while(*str) {
        USART2_SendChar(*str++); // Loop through the characters of the string calling SendChar for each character
    }
}

char USART2_ReceiveChar(void) {
    while(!(USART2->SR & USART_SR_RXNE)); // Wait for transmit and receive buffer to both be empty
    return toupper(USART2->DR);
}

//Main function

int main(void) {

    SystemClock_Config();
    USART2_Init();
    configure_ppm_timer3();
    configure_pwm_timer4();

    // Send startup message
    USART2_SendString("STM32F411 USART2 Ready\r\n");
    USART2_SendString("Right Motor (PPM) Ready!\r\n");
    USART2_SendString("Left Motor (PWM) Ready!\r\n");


    while(1) {
        char command = USART2_ReceiveChar();
		while(command == 'W'){
			USART2_SendString(" Moving Forward!\r\n");
			TIM3->CCR1 = 10100;
			TIM4->CCR1 = 10100;
			command = USART2_ReceiveChar();
			}
		while(command == 'S'){
			USART2_SendString(" Moving Reverse!\r\n");
			TIM3->CCR1 = 8500;
			TIM4->CCR1 = 8500;
			command = USART2_ReceiveChar();
		}
		while(command == 'A'){
			USART2_SendString(" Moving Left!\r\n");
			TIM3->CCR1 = 8600;
			TIM4->CCR1 = 10000;
			command = USART2_ReceiveChar();
		}
		while(command == 'D'){
			USART2_SendString(" Moving Right!\r\n");
			TIM3->CCR1 = 10000;
			TIM4->CCR1 = 8600;
			command = USART2_ReceiveChar();
		}
		while(command == 'Q'){
			USART2_SendString(" Moving Forward Left!\r\n");
			TIM3->CCR1 = 10100;
			TIM4->CCR1 = 8700;
			command = USART2_ReceiveChar();
		}
		while(command == 'E'){
			USART2_SendString(" Moving Forward Right!\r\n");
			TIM3->CCR1 = 8700;
			TIM4->CCR1 = 10100;
			command = USART2_ReceiveChar();
		}
		while(command == 'Z'){
			USART2_SendString(" Moving Reverse Left!\r\n");
			TIM3->CCR1 = 8500;
			TIM4->CCR1 = 9900;
			command = USART2_ReceiveChar();
		}
		while(command == 'C'){
			USART2_SendString(" Moving Reverse Right!\r\n");
			TIM3->CCR1 = 9900;
			TIM4->CCR1 = 8500;
			command = USART2_ReceiveChar();
		}
		while(command == '['){
			USART2_SendString(" Speeding up!!!!\r\n");
			if(TIM3->CCR1 < 10500){
				TIM3->CCR1 += 20;
				TIM4->CCR1 = TIM3->CCR1;
			} else{
				TIM3->CCR1 = 10500;
				TIM4->CCR1 = 10500;
			}
			command = USART2_ReceiveChar();
		}
		while(command == ']'){
			USART2_SendString(" Slowing Down!!!!\r\n");
			if(TIM3->CCR1 > 8200){
				TIM3->CCR1 -= 20;
				TIM4->CCR1 = TIM3->CCR1;
			} else{
				TIM3->CCR1 = 8200;
				TIM4->CCR1 = 8200;
			}
			command = USART2_ReceiveChar();
		}
		if(command == ' '){
			TIM4->CCR1 = 9200;
			TIM3->CCR1 = 9200;
		}
    }

} //End of main

