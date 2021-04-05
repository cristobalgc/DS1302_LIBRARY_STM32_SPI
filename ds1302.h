/******************************************************************************/
/*                                                                            */
/*   All rights reserved. Distribution or duplication without previous        */
/*   written agreement of the owner prohibited.                               */
/*                                                                            */
/******************************************************************************/

/** \file ds1302.h
 *
 * \brief Header file for DS1302 component
 *
 * Header file for DS1302 manager
 *
 * <table border="0" cellspacing="0" cellpadding="0">
 * <tr> <td> Author:   </td> <td> C.Garcia   </td></tr>
 * <tr> <td> Date:     </td> <td> 04/09/2018             </td></tr>
 * </table>
 * \n
 * <table border="0" cellspacing="0" cellpadding="0">
 * <tr> <td> COMPONENT: </td> <td> DS1302    </td></tr>
 * <tr> <td> SCOPE:     </td> <td> Public      </td></tr>
 * <tr> <td> TARGET:    </td> <td> MCU        </td></tr>
 * </table>
 * \note
 *
 * \see
 */

#ifndef DS_1302_H_
#define DS_1302_H_

/******************************************************************************/
/*                         Project Includes                                   */
/******************************************************************************/
#include "stm32f1xx_hal.h"
/******************************************************************************/
/*                 Definition of exported symbolic constants                  */
/******************************************************************************/
#define SPANISH_LANGUAGE
#define DS1302_24H_FORMAT	(0U)
#define DS1302_12H_FORMAT	(1U)

#define DS1302_UNKNOWN	"Unknown"
#define DS1302_AM "AM"
#define DS1302_PM "PM"
#define DS1302_EMPTY "  "

#ifdef ENGLISH_LANGUAJE
/* Days of a Week */
#define DS1302_MONDAY "Mon"
#define DS1302_TUESDAY "Tue"
#define DS1302_WEDNESDAY "Wed"
#define DS1302_THURSDAY "Thu"
#define DS1302_FRIDAY "Fri"
#define DS1302_SATURDAY "Sat"
#define DS1302_SUNDAY "Sun"
/* Months of a year */
#define DS1302_JAN	"Jan"
#define DS1302_FEB "Feb"
#define DS1302_MAR "Mar"
#define DS1302_APR "Apr"
#define DS1302_MAY "May"
#define DS1302_JUN "Jun"
#define DS1302_JUL "Jul"
#define DS1302_AUG "Aug"
#define DS1302_SEP "Sep"
#define DS1302_OCT "Oct"
#define DS1302_NOV "Nov"
#define DS1302_DIC "Dec"
#endif

#ifdef SPANISH_LANGUAGE
/* Dias de la semana */
#define DS1302_MONDAY "Lun"
#define DS1302_TUESDAY "Mar"
#define DS1302_WEDNESDAY "Mie"
#define DS1302_THURSDAY "Jue"
#define DS1302_FRIDAY "Vie"
#define DS1302_SATURDAY "Sab"
#define DS1302_SUNDAY "Dom"
/* Meses del año */
#define DS1302_JAN	"Ene"
#define DS1302_FEB "Feb"
#define DS1302_MAR "Mar"
#define DS1302_APR "Abr"
#define DS1302_MAY "May"
#define DS1302_JUN "Jun"
#define DS1302_JUL "Jul"
#define DS1302_AUG "Ago"
#define DS1302_SEP "Sep"
#define DS1302_OCT "Oct"
#define DS1302_NOV "Nov"
#define DS1302_DIC "Dic"
#endif

//+++++++++++++++++++++++++++++++++++++++++ Set Register Names ++++++++++++++++++++++++++++++++++++++++++//
//  Since the highest bit is always '1', the registers start at 0x80.  If the register is read, the      //|
//  lowest bit should be '1'.                                                                            //|

#define DS1302_SECONDS           (0x80U)
#define DS1302_MINUTES           (0x82U)
#define DS1302_HOURS             (0x84U)
#define DS1302_DATE              (0x86U)
#define DS1302_MONTH             (0x88U)
#define DS1302_DAY               (0x8AU)
#define DS1302_YEAR              (0x8CU)
#define DS1302_CLOCK_BURST       (0xBEU)
#define DS1302_CLOCK_BURST_WRITE (0xBEU)
#define DS1302_CLOCK_BURST_READ  (0xBFU)
#define DS1302_RAMSTART          (0xC0U)
#define DS1302_RAMEND            (0xFCU)
#define DS1302_RAM_BURST         (0xFEU)
#define DS1302_RAM_BURST_WRITE   (0xFEU)
#define DS1302_RAM_BURST_READ    (0xFFU)

#define DS1302_ENABLE            (0x8EU)
#define DS1302_TRICKLE           (0x90U)
#define DS1302_TIMEOUT_MAX       (100U)
/******************************************************************************/
/*                Definition of exported function like macros                 */
/******************************************************************************/

/******************************************************************************/
/*         Definition of exported types (typedef, enum, struct, union)        */
/******************************************************************************/
typedef enum ds1302_errors_e{
	DS1302_NOK,		/**< DS1302 ERROR */
	DS1302_OK,		/**< DS1302 OK */
	DS1302_TIMEOUT	/**< DS1302 timeout */
} ds1302_errors_t;

typedef struct {
	union{
		uint8_t reg;
		struct{
			uint8_t Seconds:4;             /**< 4-bits to hold low decimal digits 0-9 */
			uint8_t Seconds10:3;           /**< 3-bits to hold high decimal digit 0-5 */
			uint8_t CH:1;                  /**< 1-bit to hold CH = Clock Halt */
		}b;
	}seconds;

	union{
		uint8_t reg;
		struct{
			uint8_t Minutes:4;          /**< 4-bits to hold low decimal digits 0-9 */
			uint8_t Minutes10:3;        /**< 3-bits to hold high decimal digit 0-5 */
			uint8_t reserved1:1;		/**< Reserved */
		}b;
	}Minutes;

	union{

		union{
			uint8_t reg;
			struct  {                                             // 24-hour section
				uint8_t Hour:4;          /**< 4-bits to hold low decimal digits 0-9 */
				uint8_t Hour10:2;        /**< 2-bits to hold high decimal digits 0-2 */
				uint8_t reserved2:1;     /**< Reserved */
				uint8_t hour_12_24:1;    /**< 1-bit to set 0 for 24 hour format  */
			} b;
		}h24;

		union{
			uint8_t reg;
			struct  {                                             // 12 hour section
				uint8_t Hour:4;         /**< 4-bits to hold low decimal digits 0-9 */
				uint8_t Hour10:1;       /**< 2-bits to hold high decimal digits 0-2 */
				uint8_t AM_PM:1;        /**< 1-bit to set AM or PM, 0 = AM, 1 = PM */
				uint8_t reserved2:1;    /**< Reserved */
				uint8_t hour_12_24:1;   /**< 1-bit to set 1 for 12 hour format  */
			} b;
		}h12;
	}Hour;

	union{
		uint8_t reg;
		struct{
			uint8_t MonthDay:4;        	/**< 4-bits to hold low decimal digits 0-9 */
			uint8_t MonthDay10:2;      	/**< 2-bits to hold high decimal digits 0-3 */
			uint8_t reserved3:2;		/**< Reserved */
		}b;
	}MonthDay;

	union{
		uint8_t reg;
		struct{
			uint8_t Month:4;        /**< 4-bits to hold low decimal digits 0-9 */
			uint8_t Month10:1;      /**< 1-bits to hold high decimal digits 0-5 */
			uint8_t reserved4:3;	/**< Reserved */
		}b;
	}Month;

	union{
		uint8_t reg;
		struct{
			uint8_t Day:3;          /**< 3-bits to hold decimal digit 1-7 */
			uint8_t reserved5:5;	/**< Reserved */
		}b;
	}Weekday;
	union{
		uint8_t reg;
		struct{
			uint8_t Year:4;       /**< 4-bits to hold high decimal digit 20 */
			uint8_t Year10:4;     /**< 4-bits to hold high decimal digit 14 */
		}b;
	}Year;

	union{
		uint8_t reg;
		struct{
			uint8_t reserved6:7;  /**< Reserved */
			uint8_t WP:1;         /**< WP = Write Protect */
		}b;
	}WriteProct;

}rtc_T;

typedef struct ds1302_gpio_cfg_s
{
	  GPIO_TypeDef *McuPort; /**< The selected Port for the CE signal, it can be GPIOA, GPIOB...*/
	  uint16_t Pinreset; /**< The GPIO pin into the port for the CE signal, it can be GPIO_PIN_4, GPIO_PIN_5...*/
}ds1302_gpio_cfg_t;


typedef struct ds1302_cfg_s{
	SPI_HandleTypeDef *spi;
	ds1302_gpio_cfg_t RstPin;
}ds1302_cfg_T;

typedef struct ds1302_datatime_s{
	uint8_t seconds;
	uint8_t hours;
	uint8_t minutes;
	uint8_t monthday;
	const char * month;
	const char *weekday;
	const char *amPm;
	uint16_t year;
}ds1302_datatime_t;

typedef struct ds1302_data_s{
	uint32_t delayTicks;
	rtc_T received;
	rtc_T send;
	ds1302_datatime_t dateandtime;
}ds1302_data_T;

typedef struct ds1302_s{
	ds1302_cfg_T cfg;
	ds1302_data_T data;
}ds1302_T;
/******************************************************************************/
/*                    Declaration of exported variables                       */
/******************************************************************************/

/******************************************************************************/
/*                  Declaration of exported constant data                     */
/******************************************************************************/

/******************************************************************************/
/*               Declaration of exported function prototypes                  */
/******************************************************************************/

/**
  * @brief  Initialize the DS1302 Driver.
  * @param[in]  ds1302: The Ds1302 object.
  * @param[in]  config: The configuration to be applied into the selected DS1302 object.
  * @retval   ds1302_errors_t
  *
 */
extern ds1302_errors_t DS1302_Init(ds1302_T *ds1302, const ds1302_cfg_T *config);

/**
  * @brief  Write a valid command into the driver .
  * @param[in]  ds1302: The DS1302 object.
  * @param[in]  data: Data to be written
  * @param[in]  size: Data size.
  * @retval   ds1302_errors_t
  *
 */
extern ds1302_errors_t DS1302_Write(const ds1302_T *ds1302, const uint8_t *data, uint8_t size);

/**
  * @brief  Set the date-time into the DS1302 chip.
  * @param[in]  ds1302: The DS1302 object.
  * @param[in]  hformat: Hour format. Set 0 for 24H format. Set 1 for 12h format.
  * @param[in]  hours: Number of hours to set into the driver.
  * @param[in]  minutes: Number of minutes to set into the driver.
  * @param[in]  seconds: Number of seconds to set into the driver.
  * @param[in]  ampm: In case of 12h format : 0 = AM, 1 = PM.
  * @param[in]  dayofweek : Day of week to set into the driver.
  * @param[in]  dayofmonth: Day of month to set into the driver.
  * @param[in]  month: Month to set into the driver.
  * @param[in]  year: Year to set into the driver.
  * @retval   ds1302_errors_t
 */
extern ds1302_errors_t DS1302_setTime(ds1302_T *ds1302, const uint8_t hformat, const uint8_t hours, const uint8_t minutes,
		const uint8_t seconds, const uint8_t ampm , const uint8_t dayofweek, const uint8_t dayofmonth, const uint8_t month,
		const int year);
/**
  * @brief  Read from a register
  * @param[in]  ds1302: The DS1302 object.
  * @param[in]  RegisterAddr: specifies the ds1302 register to be written.
  * @param[in]  ptr: Data to be written
  * @param[in]  nbytes: Number of bytes to be written in to the driver
  * @retval   None
 */
extern ds1302_errors_t DS1302_Read(ds1302_T *ds1302, uint8_t RegisterAddr, uint8_t *ptr, uint8_t nbytes);
/**
  * @brief  Update the date and time of the DS1302 driver.
  * @param[in]  ds1302: The DS1302 object.
  * @retval   ds1302_errors_t
 */
extern ds1302_errors_t DS1302_updateDateTime(ds1302_T *ds1302);

#endif /* SW_COMPONENT */
