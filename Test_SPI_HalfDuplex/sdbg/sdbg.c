/******************************************************************************/
/*                                                                            */
/*   All rights reserved. Distribution or duplication without previous        */
/*   written agreement of the owner prohibited.                               */
/*                                                                            */
/******************************************************************************/

/** \file ds1302.c
 *
 * \brief Source code file for SDBG
 *
 * Source code file for SDBG (serial debug)
 *
 * <table border="0" cellspacing="0" cellpadding="0">
 * <tr> <td> Author:   </td> <td> C.Garcia   </td></tr>
 * <tr> <td> Date:     </td> <td> 27/02/2021             </td></tr>
 * </table>
 * \n
 * <table border="0" cellspacing="0" cellpadding="0">
 * <tr> <td> COMPONENT: </td> <td> SDBG  </td></tr>
 * <tr> <td> TARGET:    </td> <td> MCU        </td></tr>
 * </table>
 * \note
 *
 * \see
 */

/******************************************************************************/
/*                Include common and project definition header                */
/******************************************************************************/
#include "stm32f1xx_hal.h"
#include<stdio.h>
#include<stdarg.h>
#include<string.h>

/******************************************************************************/
/*                      Include headers of the component                      */
/******************************************************************************/

#include "sdbg.h"
/******************************************************************************/
/*                            Include other headers                           */
/******************************************************************************/

/******************************************************************************/
/*                   Definition of local symbolic constants                   */
/******************************************************************************/
#define UART_DECIMAL 	(10u)
#define UART_OCTAL 		(8u)
#define UART_HEX 		(16u)
/******************************************************************************/
/*                  Definition of local function like macros                  */
/******************************************************************************/

/******************************************************************************/
/*          Definition of local types (typedef, enum, struct, union)          */
/******************************************************************************/

/******************************************************************************/
/*                       Definition of local variables                        */
/******************************************************************************/
static char str_buffer[UART_BUFF_LEN] = {0u};

/******************************************************************************/
/*                     Definition of local constant data                      */
/******************************************************************************/

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
 * @brief  Convert integer numbers in to a string to be represented in the LCD.
 * @note
 * @param[in]  num    	The number to convert
 * @param[in]  base   	The base to convert the number.
 * 					use 10 as base parameter to convert the number in to a decimal value,
 * 					16 to convert it in hexadecimal value and 8 to convert the number in octal.
 * @return  none
 */
static char* sdbg_convert(unsigned int num, int base);       //Convert integer number into octal, hex, etc.

/**
  * @brief  Sends an amount of data in blocking mode or non blocking mode DMA mode.
  * @note   When UART parity is not enabled (PCE = 0), and Word Length is configured to 9 bits (M1-M0 = 01),
  *         the sent data is handled as a set of u16. In this case, Size must indicate the number
  *         of u16 provided through pData.
  * @param  huart Pointer to a UART_HandleTypeDef structure that contains
  *               the configuration information for the specified UART module.
  * @param  pData Pointer to data buffer (u8 or u16 data elements).
  * @param  Size  Amount of data elements (u8 or u16) to be sent
  * @param  Timeout Timeout duration (only when UART_USE_BLOCK_TRANSFER is used)
  * @retval HAL status
  */
static uint8_t sdbg_transmit(UART_HandleTypeDef *huart, uint8_t *pData, uint16_t Size, uint32_t timeout);

/******************************************************************************/
/*                       Definition of local functions                        */
/******************************************************************************/

static char *sdbg_convert(unsigned int num, int base)
{
    static char Representation[]= "0123456789ABCDEF";
    char *ptr;
    static char num_buffer[50];
    ptr = &num_buffer[49];
    *ptr = '\0';

    do
    {
        *--ptr = Representation[num%base];
        num /= base;
    }while(num != 0);

    return(ptr);
}

static uint8_t sdbg_transmit(UART_HandleTypeDef *huart, uint8_t *pData, uint16_t Size, uint32_t timeout)
{
	uint8_t ret = 1u;
#ifdef UART_USE_IT_TRANSFER
	(void)timeout;
	if (HAL_UART_Transmit_IT(huart,pData, size) != HAL_OK)
	{
		ret = 0u;
	}
#endif
#ifdef UART_USE_BLOCK_TRANSFER
	if(HAL_UART_Transmit(huart,pData, size, timeout) != HAL_OK)
	{
		ret = 0u;
	}
#endif
#ifdef UART_USE_DMA_TRANSFER
	(void)timeout;
	if (HAL_UART_Transmit_DMA(huart, pData, Size) != HAL_OK)
	{
		ret = 0u;
	}
#endif

	return ret;
}

/******************************************************************************/
/*                      Definition of exported functions                      */
/******************************************************************************/

/*
 * @brief Print function to be used like printf.
 *
 * @param[in]: LCD_t *lcd - The lcd object to use.
 * @param[in]: char* format - The data string to be printed on the LCD.
 * @return The number of printed items
 */
uint16_t SDBG_print(UART_HandleTypeDef *huart, char* format,...)
{
	char *traverse = format;
	uint16_t strLenght = 0u; 	//index of the string recorrido
	uint16_t strNumLenght = 0u;	//longitud del numero
	uint16_t accumulatedNumber = 0u;
	uint16_t strStart = 0u;
	uint16_t counter = 0u;
	uint8_t go_out = 0u;
	uint16_t dPerCount = 0u;   	//number of d found after % symbol
	uint16_t sPerCount = 0u;   	//number of s found after % symbol
	uint16_t oPerCount = 0u;   	//number of o found after % symbol
	uint16_t cPerCount = 0u;   	// number of c found after % symbol
	uint16_t xPerCount = 0u;   	// number of x found after % symbol
	uint16_t percCount = 0u;  	// number of % found
	uint16_t discardItemsCount = 0u; // the sum of d,s,o,c,x,perc count variables;
	uint16_t messageSize = 0u;
	int32_t i;
	char *s;
	//Module 1: Initializing Myprintf's arguments
	va_list arg;
	memset(str_buffer,'\0',sizeof(str_buffer));
	va_start(arg, format);

	for(; *traverse != '\0'; traverse++,counter++)
	{
		while( (*traverse != '%') && (*traverse != '\0'))
		{
			traverse++;
			strLenght++;
			counter++;
		}
		//Only print if any char detected before "%" special character
		if(strLenght > 0u)
		{
			if((counter - discardItemsCount + accumulatedNumber)< UART_BUFF_LEN)
			{
				if(percCount == 0){
					(void*)strncat(str_buffer, format+strStart, strLenght);
				}else{
					(void*)strncat(str_buffer, format+strStart+2, strLenght);
				}

				strLenght = 0;
				strStart = counter;
			}
		}
		if((*traverse == '%')){
			percCount++;
			traverse++;
			counter++;
		}

		//Module 2: Fetching and executing arguments
		switch(*traverse)
		{
		case 'c' :
			i = va_arg(arg,int);		//Fetch char argument
			accumulatedNumber++;
			cPerCount++;
			if((counter + accumulatedNumber - dPerCount - oPerCount - cPerCount - xPerCount - percCount) < UART_BUFF_LEN)
			{
				(void*)strncat(str_buffer, (char*)&i, 1u);
			}
			break;

		case 'd' :
			i = va_arg(arg,int);         //Fetch Decimal/Integer argument
			dPerCount++;
			if(i<0)
			{
				accumulatedNumber++;
				i = -i;
				if(((counter + accumulatedNumber + dPerCount - sPerCount - oPerCount - cPerCount - xPerCount - percCount)) < UART_BUFF_LEN)
				{
					(void*)strncat(str_buffer, (char *)"-", 1u);
				}
			}
			strNumLenght = strlen(sdbg_convert(i,10));
			accumulatedNumber += strNumLenght;
			if(((counter + accumulatedNumber - dPerCount - sPerCount - oPerCount - cPerCount - xPerCount - percCount)) < UART_BUFF_LEN)
			{
				(void*)strncat(str_buffer, (char*)sdbg_convert(i,10), strNumLenght);
			}
			break;

		case 'o':
			i = va_arg(arg,unsigned int); //Fetch Octal representation
			oPerCount++;
			strNumLenght = strlen(sdbg_convert(i, 8));
			accumulatedNumber += strNumLenght;
			if(((counter+accumulatedNumber - dPerCount - sPerCount - oPerCount - cPerCount - xPerCount - percCount)) < UART_BUFF_LEN)
			{
				(void*)strncat(str_buffer, (char *)sdbg_convert(i, 8), strNumLenght);
			}
			break;

		case 's':
			s = va_arg(arg,char *);       //Fetch string
			sPerCount++;
			strNumLenght = strlen(s);
			accumulatedNumber += strNumLenght;
			if(((counter + accumulatedNumber - dPerCount - sPerCount - oPerCount - cPerCount - xPerCount - percCount)) < UART_BUFF_LEN)
			{
				(void*)strncat(str_buffer, (char *)s, strNumLenght);
			}
			break;

		case 'x':
			i = va_arg(arg,unsigned int); //Fetch Hexadecimal representation
			xPerCount++;
			strNumLenght = strlen(sdbg_convert(i, 16));
			accumulatedNumber += strNumLenght;
			if(((counter + accumulatedNumber - dPerCount - sPerCount - oPerCount - cPerCount - xPerCount - percCount)) < UART_BUFF_LEN)
			{
				(void*)strncat(str_buffer, (char *)sdbg_convert(i, 16), strNumLenght);
			}
			break;
		default:
			go_out = 1u;
			break;
		}
		discardItemsCount = dPerCount + sPerCount + oPerCount + cPerCount + xPerCount + percCount;
		if(go_out){break;}
	}
	messageSize = counter - discardItemsCount + accumulatedNumber;
	if(messageSize < UART_BUFF_LEN)
	{
		sdbg_transmit(huart, (uint8_t *)str_buffer, messageSize, UART_TIMEOUT);
	}

	//Module 3: Closing argument list to necessary clean-up
	va_end(arg);
	return messageSize;
}
