/******************************************************************************/
/*                                                                            */
/*   All rights reserved. Distribution or duplication without previous        */
/*   written agreement of the owner prohibited.                               */
/*                                                                            */
/******************************************************************************/

/** \file sdbg.h
 *
 * \brief Header file for SDBG component
 *
 * Header file for SDBG manager
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

#ifndef SDBG_H_
#define SDBG_H_

/******************************************************************************/
/*                         Project Includes                                   */
/******************************************************************************/

/******************************************************************************/
/*                 Definition of exported symbolic constants                  */
/******************************************************************************/

/******************************************************************************/
/*                Definition of exported function like macros                 */
/******************************************************************************/

/******************************************************************************/
/*         Definition of exported types (typedef, enum, struct, union)        */
/******************************************************************************/

/******************************************************************************/
/*                    Declaration of exported variables                       */
/******************************************************************************/

/******************************************************************************/
/*                  Declaration of exported constant data                     */
/******************************************************************************/
//options

#define UART_USE_DMA_TRANSFER
//#define UART_USE_IT_TRANSFER
//#define UART_USE_BLOCK_TRANSFER
#define UART_BUFF_LEN	(50U)
#define UART_TIMEOUT	(0U)/* Only used when block transfer is defined */
/******************************************************************************/
/*               Declaration of exported function prototypes                  */
/******************************************************************************/

extern uint16_t SDBG_print(UART_HandleTypeDef *huart , char* format,...);

#endif /* SW_COMPONENT */
