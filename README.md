# DS1302_LIBRARY_STM32_SPI
Library to control the DS1302 Real Time Clock using STMCUBE HAL with SPI in half duplex mode 

### Referenced links
- This library was based on the datasheet documentation.

### Steps to use the library
- Connect your DS1302 to your mcu, i used the blue pill.
- Configure your MCU using Stm32cubeMx, to run:
	- One channel of SPI in HALF duplex mode and another
	- One USART channel to print using a serial console the information reported by the DS1302 peripheral.
	- One timer that should be triggered each 0.5 seconds, the timer interrupt shall have the higest priority.
- Download the library and include it in your source code project.
- Try to inlude this example in your own main.c file to test it.

### Sample of code

	#include "../ds1302/ds1302.h"
	#include "../sdbg/sdbg.h"

	#define HOUR_FORMAT	(0) // 0 = 0-24h; 1= 0-12h
	#define HOUR 		(17U)
	#define MINUTES 	(25U)
	#define SECONDS 	(00U)
	#define AM_PM		(1U)
	#define WEEKDAY 	(1U)
	#define MONTHDAY 	(26U)
	#define MONTH 		(9U)
	#define YEAR 		(2021U)
	
	//generated with stm32cubeMX
	SPI_HandleTypeDef hspi1;
	TIM_HandleTypeDef htim3;
	UART_HandleTypeDef huart1;
	
	static uint32_t encoderPos = 0;
	static uint8_t flag_spi=0;
	static ds1302_T rtc;

	static ds1302_cfg_T ds1302_config = {
		&hspi1,
		{GPIOA, GPIO_PIN_4}
	};

	void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
	{
		//check if the interrupt comes from TIM3
		if (htim->Instance == TIM3){
			HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_13);
			encoderPos=(encoderPos+1)%60;
			SDBG_print(&huart1,"%d : %d : %d",rtc.data.dateandtime.hours, rtc.data.dateandtime.minutes, rtc.data.dateandtime.seconds);
			flag_spi = 1u;
		}
	}
	
	int main(void)
	{
		DS1302_Init(&rtc, &ds1302_config);
		DS1302_setTime(&rtc, HOUR_FORMAT, HOUR, MINUTES, SECONDS, AM_PM, WEEKDAY, MONTHDAY, MONTH, YEAR);
		while (1)
		{
			if(flag_spi){
				DS1302_updateDateTime(&rtc);
				flag_spi=0;
			}
		}
	}

### End
