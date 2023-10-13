//----------------------------------------------------------------------------
//  Description:  This file contains the initialization values for the 
//  LMP91000 registers.
//
//  MSP430/LMP91000 Interface Code Library v1.0
//
//   Vishy Natarajan
//   Texas Instruments Inc.
//   December 2011
//   Modified October 2012

//------------------------------------------------------------------------------
// Change Log:
//------------------------------------------------------------------------------
// Version:  1.00
// Comments: Initial Release Version
//------------------------------------------------------------------------------

/*
  Copyright 2012 Texas Instruments Incorporated. All rights reserved.

  IMPORTANT: Your use of this Software is limited to those specific rights
  granted under the terms of a software license agreement between the user
  who downloaded the software, his/her employer (which must be your employer)
  and Texas Instruments Incorporated (the "License").  You may not use this
  Software unless you agree to abide by the terms of the License. The License
  limits your use, and you acknowledge, that the Software may not be modified,
  copied or distributed unless embedded on a Texas Instruments microcontroller
  or used solely and exclusively in conjunction with a Texas Instruments radio
  frequency transceiver, which is integrated into your product.  Other than for
  the foregoing purpose, you may not use, reproduce, copy, prepare derivative
  works of, modify, distribute, perform, display or sell this Software and/or
  its documentation for any purpose.

  YOU FURTHER ACKNOWLEDGE AND AGREE THAT THE SOFTWARE AND DOCUMENTATION ARE
  PROVIDED “AS IS” WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESS OR IMPLIED,
  INCLUDING WITHOUT LIMITATION, ANY WARRANTY OF MERCHANTABILITY, TITLE,
  NON-INFRINGEMENT AND FITNESS FOR A PARTICULAR PURPOSE. IN NO EVENT SHALL
  TEXAS INSTRUMENTS OR ITS LICENSORS BE LIABLE OR OBLIGATED UNDER CONTRACT,
  NEGLIGENCE, STRICT LIABILITY, CONTRIBUTION, BREACH OF WARRANTY, OR OTHER
  LEGAL EQUITABLE THEORY ANY DIRECT OR INDIRECT DAMAGES OR EXPENSES
  INCLUDING BUT NOT LIMITED TO ANY INCIDENTAL, SPECIAL, INDIRECT, PUNITIVE
  OR CONSEQUENTIAL DAMAGES, LOST PROFITS OR LOST DATA, COST OF PROCUREMENT
  OF SUBSTITUTE GOODS, TECHNOLOGY, SERVICES, OR ANY CLAIMS BY THIRD PARTIES
  (INCLUDING BUT NOT LIMITED TO ANY DEFENSE THEREOF), OR OTHER SIMILAR COSTS.

  Should you have any questions regarding your right to use this Software,
  contact Texas Instruments Incorporated at www.TI.com.
**************************************************************************************************/



#ifndef HEADER_FILE_TI_LMP91000_REGISTER_SETTINGS_H

#define HEADER_FILE_TI_LMP91000_REGISTER_SETTINGS_H

/************************************************************
* TI LMP91000 REGISTER SET INITIALIZATION VALUES
************************************************************/

#define TI_LMP91000_TIACN_REG_VALUE                    (0x1C)                  /* default */
#define TI_LMP91000_REFCN_REG_VALUE                    (0xBD)                  /* default */
#define TI_LMP91000_MODECN_REG_VALUE                   (0x03)                  /* 3lead  */
#define TI_LMP91000_TIATEMP_MEASURE_MODE               (0x07)                  /* Temperature Measurement Mode (TIA ON) */
#define TI_LMP91000_STANDBY_MODE                       (0x02)                  /* Standby Mode */
#define TI_LMP91000_STANDBY_FET_EN_MODE                (0x82)                  /* Standby & FET EN Mode */
#define TI_LMP91000_DEEP_SLEEP_FET_EN_MODE             (0x80)                  /* Deep Sleep & FET EN Mode */

#define TIACN_NEW_VALUE 0x1C                                                   // Rtia = 350kohm, Rload = 10ohm
#define REFCN_NEW_VALUE 0xBD                                                   // External Ref, 0.6 V

#endif                                                                         // HEADER_FILE_TI_LMP91000_REGISTER_SETTINGS_H
