/******************************************************************************/
/*                                                                            */
/*   All rights reserved. Distribution or duplication without previous        */
/*   written agreement of the owner prohibited.                               */
/*                                                                            */
/******************************************************************************/

/** \file ds1302.c
 *
 * \brief Source code file for DS1302
 *
 * Source code file for DS1302 manager
 *
 * <table border="0" cellspacing="0" cellpadding="0">
 * <tr> <td> Author:   </td> <td> C.Garcia   </td></tr>
 * <tr> <td> Date:     </td> <td> 27/02/2021             </td></tr>
 * </table>
 * \n
 * <table border="0" cellspacing="0" cellpadding="0">
 * <tr> <td> COMPONENT: </td> <td> DS1302  </td></tr>
 * <tr> <td> TARGET:    </td> <td> MCU        </td></tr>
 * </table>
 * \note
 *
 * \see
 */
/*https://github.com/STMicroelectronics-CentralLabs/ST_Drone_FCU_F401/blob/master/STM32%20FW%20Project/BLE%20Remocon%20Beta%20release%20301117/Drivers/BSP/STEVAL_FCU001_V1/steval_fcu001_v1.c*/
/******************************************************************************/
/*                Include common and project definition header                */
/******************************************************************************/
#include "string.h"
/******************************************************************************/
/*                      Include headers of the component                      */
/******************************************************************************/
#include "ds1302.h"

/******************************************************************************/
/*                            Include other headers                           */
/******************************************************************************/

/******************************************************************************/
/*                   Definition of local symbolic constants                   */
/******************************************************************************/
#define DS1302_SECONDS_MAX			(59U)
#define DS1302_MINUTES_MAX			(59U)
#define DS1302_WEEKDAY_MAX			(7U)
#define DS1302_YEAR_MAX				(2100U)
#define DS1302_HOUR_MAX				(23U)
#define DS1302_MONTHDAY_MAX			(31U)
#define DS1302_MONTH_MAX			(12U)
#define DS1302_BURST_MAX_BYTES		(8U)
#define DS1302_MILENIUM				(2000U)
#define DS1302_INSTRUCTION_CYCLES	(2U)
#define DS1302_12H					(12U)
#define DS1302_WEEK_DAYS_MAX		(8U)
#define DS1302_MONTHS_MAX			(13U)
#define DS1302_AMPM_MAX				(2U)
/******************************************************************************/
/*                  Definition of local function like macros                  */
/******************************************************************************/
//++++++++++++++++++++++++++++++++++++++++++ Conversion Macros ++++++++++++++++++++++++++++++++++++++++++//|
//  Macros to convert the bcd values of the registers to normal integer variables.  The code uses        //|
//  seperate variables for the high byte and the low byte of the bcd, so these macros handle both bytes  //|
//  seperately.
#define DS1302_BCD2BIN(h,l)   (((h)*10) + (l))
#define DS1302_BIN2BCD_H(x)   ((x)/10)
#define DS1302_BIN2BCD_L(x)   ((x)%10)

#define __SPI_DIRECTION_1LINE_TX(__HANDLE__) do{\
                                             CLEAR_BIT((__HANDLE__)->Instance->CR1, SPI_CR1_RXONLY | SPI_CR1_CPOL );\
                                             SET_BIT((__HANDLE__)->Instance->CR1, SPI_CR1_BIDIMODE | SPI_CR1_BIDIOE);\
                                             }while(0);

#define __SPI_DIRECTION_1LINE_RX(__HANDLE__) do {\
                                             CLEAR_BIT((__HANDLE__)->Instance->CR1, SPI_CR1_RXONLY | SPI_CR1_BIDIOE);\
                                             SET_BIT((__HANDLE__)->Instance->CR1, SPI_CR1_BIDIMODE | SPI_CR1_CPOL );\
                                             } while(0);
/******************************************************************************/
/*          Definition of local types (typedef, enum, struct, union)          */
/******************************************************************************/

/******************************************************************************/
/*                       Definition of local variables                        */
/******************************************************************************/

/******************************************************************************/
/*                     Definition of local constant data                      */
/******************************************************************************/
/* Days of a week */
static char const *ds1302_days[DS1302_WEEK_DAYS_MAX] = {DS1302_UNKNOWN, DS1302_SUNDAY, DS1302_MONDAY, DS1302_TUESDAY, DS1302_WEDNESDAY, DS1302_THURSDAY, DS1302_FRIDAY, DS1302_SATURDAY};
/* Months of a year */
static char const *ds1302_months[DS1302_MONTHS_MAX] = {DS1302_UNKNOWN, DS1302_JAN, DS1302_FEB, DS1302_MAR, DS1302_APR, DS1302_MAY, DS1302_JUN, DS1302_JUL, DS1302_AUG, DS1302_SEP, DS1302_OCT, DS1302_NOV, DS1302_DIC};
/* AM/PM*/
static char const *ds1302_AMPM[DS1302_AMPM_MAX] = {DS1302_AM, DS1302_PM};

/******************************************************************************/
/*                      Definition of exported variables                      */
/******************************************************************************/

/******************************************************************************/
/*                    Definition of exported constant data                    */
/******************************************************************************/

/******************************************************************************/
/*                  Declaration of local function prototypes                  */
/******************************************************************************/
/**
 * @brief  This function reads multiple bytes on SPI 3-wire.
 * @param[in]  ds1302: The DS1302 object.
 * @param[in]  val: value.
 * @param[in]  nBytesToRead: number of bytes to read.
 * @param[in]  Timeout: number of expected clock cycles to read a byte.
 * @retval ds1302_errors_t
 */
static ds1302_errors_t ds1302_ReadNBytes(const ds1302_T *ds1302, uint8_t *val, uint16_t nBytesToRead, uint32_t Timeout);

/**
  * @brief  Receives a Byte from the SPI bus.
  * @param[in]  ds1302: Ds1302 object.
  * @param[in]  val: the received value though SPI port.
  * @retval ds1302_errors_t
  */
static ds1302_errors_t ds1302_Read1Byte(const ds1302_T *ds1302, uint8_t *val);

/**
  * @brief  Calculate a delay in ticks of system frequency.
  * @param[in]  ds1302: The ds1302 object.
  * @retval The delay calculated in ticks of system frequency.
  */
static uint32_t ds1302_getDelay(const ds1302_T *ds1302);

/******************************************************************************/
/*                       Definition of local functions                        */
/******************************************************************************/
static ds1302_errors_t ds1302_Read1Byte(const ds1302_T *ds1302, uint8_t *val)
{
	uint16_t delay = 0u;
	ds1302_errors_t errorcode = DS1302_OK;
	/* Disable the SPI and change the data line to input */
	/* Check if the SPI is already enabled */
	if ((ds1302->cfg.spi->Instance->CR1 & SPI_CR1_SPE) != SPI_CR1_SPE){
		/* Disable SPI peripheral */
		__HAL_SPI_DISABLE(ds1302->cfg.spi);
	}
	SPI_1LINE_RX(ds1302->cfg.spi);

	__disable_irq();
	__HAL_SPI_ENABLE(ds1302->cfg.spi);
	for(delay = 0; delay < ds1302->data.delayTicks; delay++){__DSB();}
	__HAL_SPI_DISABLE(ds1302->cfg.spi);
	__enable_irq();

	while ((ds1302->cfg.spi->Instance->SR & SPI_FLAG_RXNE) != SPI_FLAG_RXNE);
	/* read the received data */
	*val = *(__IO uint8_t *)ds1302->cfg.spi->Instance->DR;

	/* Wait for the BSY flag reset */
	while ((ds1302->cfg.spi->Instance->SR & SPI_FLAG_BSY) == SPI_FLAG_BSY);

	return errorcode;
}


static ds1302_errors_t ds1302_ReadNBytes(const ds1302_T *ds1302, uint8_t *val, uint16_t nBytesToRead, uint32_t Timeout)
{
	uint16_t delay = 0U;
	uint32_t tickstart;

	ds1302_errors_t errorcode = DS1302_OK;

	 /* Init tickstart for timeout management*/
	 tickstart = HAL_GetTick();

	/* Disable the SPI and change the data line to input */
	/* Check if the SPI is already enabled */
	if ((ds1302->cfg.spi->Instance->CR1 & SPI_CR1_SPE) != SPI_CR1_SPE){
		/* Disable SPI peripheral */
		__HAL_SPI_DISABLE(ds1302->cfg.spi);
	}
	SPI_1LINE_RX(ds1302->cfg.spi);

	/* Interrupts should be disabled during this operation */
	__disable_irq();
	__HAL_SPI_ENABLE(ds1302->cfg.spi);

	/* Transfer loop */
	while (nBytesToRead > 1U)
	{
		/* Check the RXNE flag */
		if (ds1302->cfg.spi->Instance->SR & SPI_FLAG_RXNE)
		{
			/* read the received data */
			*val = *(__IO uint8_t *) &ds1302->cfg.spi->Instance->DR;
			val += sizeof(uint8_t);
			nBytesToRead--;
		}
		else
		{
			/* Timeout management */
			if ((((HAL_GetTick() - tickstart) >=  Timeout) && (Timeout != HAL_MAX_DELAY)) || (Timeout == 0U))
			{
				errorcode = DS1302_TIMEOUT;
				goto error;
			}
		}
	}

	/* In master RX mode the clock is automatically generated on the SPI enable.
  	  So to guarantee the clock generation for only one data, the clock must be
  	  disabled after the first bit and before the latest bit of the last Byte received */
	/* __DSB instruction are inserted to guarantee that clock is Disabled in the right timeframe */

	for(delay=0; delay<ds1302->data.delayTicks; delay++){__DSB();}
	__HAL_SPI_DISABLE(ds1302->cfg.spi);

	__enable_irq();

	while ((ds1302->cfg.spi->Instance->SR & SPI_FLAG_RXNE) != SPI_FLAG_RXNE);
	/* read the received data */
	*val = *(__IO uint8_t *) &ds1302->cfg.spi->Instance->DR;
	while ((ds1302->cfg.spi->Instance->SR & SPI_FLAG_BSY) == SPI_FLAG_BSY);

error:
return errorcode;
}

static uint32_t ds1302_getDelay(const ds1302_T *ds1302){
	uint32_t HCLK_Frequency = 0U;
	uint32_t timeSpiTransferBitns = 0U;
	uint32_t execTimens = 0U;
	uint32_t spiBaudPrescaler = 0U;
	uint32_t result = 0U;

	if (ds1302 != NULL){
		switch (ds1302->cfg.spi->Init.BaudRatePrescaler){
		case SPI_BAUDRATEPRESCALER_2:
			spiBaudPrescaler = 2;
			break;
		case SPI_BAUDRATEPRESCALER_4:
			spiBaudPrescaler = 4;
			break;
		case SPI_BAUDRATEPRESCALER_8:
			spiBaudPrescaler = 8;
			break;
		case SPI_BAUDRATEPRESCALER_16:
			spiBaudPrescaler = 16;
			break;
		case SPI_BAUDRATEPRESCALER_32:
			spiBaudPrescaler = 32;
			break;
		case SPI_BAUDRATEPRESCALER_64:
			spiBaudPrescaler = 64;
			break;
		case SPI_BAUDRATEPRESCALER_128:
			spiBaudPrescaler = 128;
			break;
		case SPI_BAUDRATEPRESCALER_256:
			spiBaudPrescaler = 256;
			break;
		default:
			break;
		};

		HCLK_Frequency = HAL_RCC_GetHCLKFreq();
		timeSpiTransferBitns = HCLK_Frequency/spiBaudPrescaler;
		timeSpiTransferBitns = 1000000000000/timeSpiTransferBitns;
		execTimens = 1000000000000/HCLK_Frequency;
		result = (timeSpiTransferBitns/execTimens)/DS1302_INSTRUCTION_CYCLES;
	}

	return result;
}
/******************************************************************************/
/*                      Definition of exported functions                      */
/******************************************************************************/
ds1302_errors_t DS1302_Init(ds1302_T *ds1302, const ds1302_cfg_T *config){
	uint16_t i;
	ds1302_errors_t error = DS1302_OK;
	/* Write protect command (disable) */
	uint8_t ds1302_wp[2] = {DS1302_ENABLE, 0x00};
	/* Trickle charge command (disable) */
	uint8_t ds1302_trickle[2] = {DS1302_TRICKLE, 0x00};

	if((ds1302 != NULL) && (config != NULL)){

		ds1302->cfg.spi = config->spi;
		ds1302->cfg.RstPin.McuPort = config->RstPin.McuPort;
		ds1302->cfg.RstPin.Pinreset = config->RstPin.Pinreset;

		ds1302->data.delayTicks = ds1302_getDelay(ds1302);
		DS1302_Write(ds1302, ds1302_wp, 2);//
		for(i=0 ;i<20;i++){;}//delay
		DS1302_Write(ds1302, ds1302_trickle, 2);
		for(i=0 ;i<20;i++){;}//delay
	}else{
		error = DS1302_NOK;
	}
	return error;
}

ds1302_errors_t DS1302_Write(const ds1302_T *ds1302, const uint8_t *data, uint8_t size){
	ds1302_errors_t error;
	HAL_GPIO_WritePin(ds1302->cfg.RstPin.McuPort, ds1302->cfg.RstPin.Pinreset, GPIO_PIN_SET);
	error = HAL_SPI_Transmit(ds1302->cfg.spi,data, size, DS1302_TIMEOUT_MAX);
	HAL_GPIO_WritePin(ds1302->cfg.RstPin.McuPort, ds1302->cfg.RstPin.Pinreset, GPIO_PIN_RESET);
	return error;
}

ds1302_errors_t DS1302_Read(ds1302_T *ds1302, uint8_t RegisterAddr, uint8_t *ptr, uint8_t nbytes){
	ds1302_errors_t error;

	RegisterAddr|= 0x01U;// last bit should be set for read

	HAL_GPIO_WritePin(ds1302->cfg.RstPin.McuPort, ds1302->cfg.RstPin.Pinreset, GPIO_PIN_SET);
	HAL_SPI_Transmit(ds1302->cfg.spi,&RegisterAddr, 1, DS1302_TIMEOUT_MAX);
	/* Check if we need to read one byte or more */
	if(nbytes > 1U){
		error = ds1302_ReadNBytes(ds1302, ptr, nbytes, DS1302_TIMEOUT_MAX);
	}else{
		error = ds1302_Read1Byte(ds1302, ptr);
	}
	HAL_GPIO_WritePin(ds1302->cfg.RstPin.McuPort, ds1302->cfg.RstPin.Pinreset, GPIO_PIN_RESET);
	/* Change the data line to output and enable the SPI */
	SPI_1LINE_TX(ds1302->cfg.spi);
	__HAL_SPI_ENABLE(ds1302->cfg.spi);
	return error;
}

ds1302_errors_t DS1302_setTime(ds1302_T *ds1302, const uint8_t hformat, const uint8_t hours, const uint8_t minutes,
		const uint8_t seconds, const uint8_t ampm , const uint8_t dayofweek, const uint8_t dayofmonth, const uint8_t month,
		const int year){
	uint8_t arraysend[9];
	ds1302_errors_t error;
	arraysend[0] = DS1302_CLOCK_BURST_WRITE;
	memset ((char *)&ds1302->data.send, 0, sizeof(rtc_T));
	if((seconds <= DS1302_SECONDS_MAX) && (minutes <= DS1302_MINUTES_MAX) &&
			(hours <= DS1302_HOUR_MAX) && (dayofweek <= DS1302_WEEKDAY_MAX) &&
			(dayofmonth <= DS1302_MONTHDAY_MAX) && (month <= DS1302_MONTH_MAX ) &&
			(year <= DS1302_YEAR_MAX)){

		ds1302->data.send.seconds.b.Seconds10 = DS1302_BIN2BCD_H( seconds);
		ds1302->data.send.seconds.b.Seconds = DS1302_BIN2BCD_L( seconds);
		ds1302->data.send.seconds.b.CH = 0;  // 1 for Clock Halt, 0 to run

		ds1302->data.send.Minutes.b.Minutes10 = DS1302_BIN2BCD_H(minutes);
		ds1302->data.send.Minutes.b.Minutes = DS1302_BIN2BCD_L(minutes);

		if((hformat) && (hours <= DS1302_12H)){
			ds1302->data.send.Hour.h12.b.Hour10 = DS1302_BIN2BCD_H(hours);
			ds1302->data.send.Hour.h12.b.Hour = DS1302_BIN2BCD_L(hours);
			ds1302->data.send.Hour.h12.b.AM_PM = ampm;
			ds1302->data.send.Hour.h12.b.hour_12_24 = DS1302_12H_FORMAT;
		} else {
			ds1302->data.send.Hour.h24.b.Hour10 = DS1302_BIN2BCD_H(hours);
			ds1302->data.send.Hour.h24.b.Hour = DS1302_BIN2BCD_L(hours);
			ds1302->data.send.Hour.h24.b.hour_12_24 = DS1302_24H_FORMAT; // 0 for 24 hour format
		}

		ds1302->data.send.MonthDay.b.MonthDay10 = DS1302_BIN2BCD_H(dayofmonth);
		ds1302->data.send.MonthDay.b.MonthDay = DS1302_BIN2BCD_L(dayofmonth);

		ds1302->data.send.Month.b.Month10 = DS1302_BIN2BCD_H(month);
		ds1302->data.send.Month.b.Month = DS1302_BIN2BCD_L(month);

		ds1302->data.send.Weekday.b.Day = dayofweek;

		ds1302->data.send.Year.b.Year10 = DS1302_BIN2BCD_H(year-DS1302_MILENIUM);
		ds1302->data.send.Year.b.Year = DS1302_BIN2BCD_L(year-DS1302_MILENIUM);

		ds1302->data.send.WriteProct.b.WP = 0U;
	}
	memcpy(&arraysend[1], &ds1302->data.send, DS1302_BURST_MAX_BYTES);
	error = DS1302_Write(ds1302, arraysend, 9);
	return error;
}

ds1302_errors_t DS1302_updateDateTime(ds1302_T *ds1302) {
	ds1302_errors_t error = DS1302_OK;
	uint8_t month;
	if(ds1302 != NULL){
		error = DS1302_Read(ds1302, DS1302_CLOCK_BURST_READ, (uint8_t*)&ds1302->data.received, DS1302_BURST_MAX_BYTES);

		month = DS1302_BCD2BIN(ds1302->data.received.Month.b.Month10, ds1302->data.received.Month.b.Month);

		if(!ds1302->data.received.Hour.h24.b.hour_12_24){
			ds1302->data.dateandtime.hours = DS1302_BCD2BIN(ds1302->data.received.Hour.h24.b.Hour10, ds1302->data.received.Hour.h24.b.Hour);
			ds1302->data.dateandtime.amPm = DS1302_EMPTY;
		}else{
			ds1302->data.dateandtime.hours = DS1302_BCD2BIN(ds1302->data.received.Hour.h12.b.Hour10, ds1302->data.received.Hour.h12.b.Hour);
			ds1302->data.dateandtime.amPm = ds1302_AMPM[ds1302->data.received.Hour.h12.b.AM_PM];
		}

		ds1302->data.dateandtime.minutes = DS1302_BCD2BIN(ds1302->data.received.Minutes.b.Minutes10, ds1302->data.received.Minutes.b.Minutes);
		ds1302->data.dateandtime.seconds = DS1302_BCD2BIN(ds1302->data.received.seconds.b.Seconds10, ds1302->data.received.seconds.b.Seconds);
		ds1302->data.dateandtime.weekday = ds1302_days[ds1302->data.received.Weekday.b.Day];
		ds1302->data.dateandtime.month = ds1302_months[month];
		ds1302->data.dateandtime.monthday =  DS1302_BCD2BIN(ds1302->data.received.MonthDay.b.MonthDay10, ds1302->data.received.MonthDay.b.MonthDay);
		ds1302->data.dateandtime.year = DS1302_BCD2BIN(ds1302->data.received.Year.b.Year10, ds1302->data.received.Year.b.Year) + DS1302_MILENIUM;
	} else {
		error = DS1302_NOK;
	}

	return error;
}
