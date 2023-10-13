//----------------------------------------------------------------------------
//  Description:  This file contains definitions specific to the LMP91000.
//  All the LMP91000 register addresses as well as some common masks for these registers 
//  are defined. 
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
  Copyright 2011 Texas Instruments Incorporated. All rights reserved.

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


#ifndef HEADER_FILE_TI_LMP91000_H

#define HEADER_FILE_TI_LMP91000_H

/************************************************************
* TI LMP91000 REGISTER SET ADDRESSES
************************************************************/

#define LMP91000_I2C_Address                           (0x48)                  // Device Address

#define TI_LMP91000_STATUS_REG                         (0x00)                  /* Read only status register */
#define TI_LMP91000_LOCK_REG                           (0x01)                  /* Protection Register */
#define TI_LMP91000_TIACN_REG                          (0x10)                  /* TIA Control Register */
#define TI_LMP91000_REFCN_REG                          (0x11)                  /* Reference Control Register*/
#define TI_LMP91000_MODECN_REG                         (0x12)                  /* Mode Control Register */

#define TI_LMP91000_WRITE_LOCK                         (0x01)
#define TI_LMP91000_WRITE_UNLOCK                       (0x00)
#define TI_LMP91000_READY                              (0x01)
#define TI_LMP91000_NOT_READY                          (0x00)


// TIACN - TIA Control Register (address 0x10)
#define TIA_GAIN_SHIFT                    2
#define TIA_GAIN_EXT_RESIST               (0 << TIA_GAIN_SHIFT)
#define TIA_GAIN_2_75K_OHM                (1 << TIA_GAIN_SHIFT)
#define TIA_GAIN_3_5K_OHM                 (2 << TIA_GAIN_SHIFT)
#define TIA_GAIN_7K_OHM                   (3 << TIA_GAIN_SHIFT)
#define TIA_GAIN_14K_OHM                  (4 << TIA_GAIN_SHIFT)
#define TIA_GAIN_35K_OHM                  (5 << TIA_GAIN_SHIFT)
#define TIA_GAIN_120K_OHM                 (6 << TIA_GAIN_SHIFT)
#define TIA_GAIN_350K_OHM                 (7 << TIA_GAIN_SHIFT)

#define R_LOAD_SHIFT                      0
#define R_LOAD_10_OHM                     (0 << R_LOAD_SHIFT)
#define R_LOAD_30_OHM                     (1 << R_LOAD_SHIFT)
#define R_LOAD_50_OHM                     (2 << R_LOAD_SHIFT)
#define R_LOAD_100_OHM                    (3 << R_LOAD_SHIFT)

// REFCN - Reference Control Register (address 0x11)
#define INT_Z_SHIFT                       5
#define INT_Z_SEL_20_PERCENT              (0 << INT_Z_SHIFT)
#define INT_Z_SEL_50_PERCENT              (1 << INT_Z_SHIFT)
#define INT_Z_SEL_67_PERCENT              (2 << INT_Z_SHIFT)
#define INT_Z_SEL_BYPASS                  (3 << INT_Z_SHIFT)

#define REF_SOURCE_SHIFT                  7
#define REF_SOURCE_INTERNAL               (0 << REF_SOURCE_SHIFT)
#define REF_SOURCE_EXTERNAL               (1 << REF_SOURCE_SHIFT)

#define BIAS_SIGN_SHIFT                   4
#define BIAS_SIGN_NEGATIVE                (0 << BIAS_SIGN_SHIFT)
#define BIAS_SIGN_POSITIVE                (1 << BIAS_SIGN_SHIFT)

#define BIAS_SHIFT                        0
#define BIAS_0_PERCENT                    (0 << BIAS_SHIFT)
#define BIAS_1_PERCENT                    (1 << BIAS_SHIFT)
#define BIAS_2_PERCENT                    (2 << BIAS_SHIFT)
#define BIAS_4_PERCENT                    (3 << BIAS_SHIFT)
#define BIAS_6_PERCENT                    (4 << BIAS_SHIFT)
#define BIAS_8_PERCENT                    (5 << BIAS_SHIFT)
#define BIAS_10_PERCENT                   (6 << BIAS_SHIFT)
#define BIAS_12_PERCENT                   (7 << BIAS_SHIFT)
#define BIAS_14_PERCENT                   (8 << BIAS_SHIFT)
#define BIAS_16_PERCENT                   (9 << BIAS_SHIFT)
#define BIAS_18_PERCENT                   (10 << BIAS_SHIFT)
#define BIAS_20_PERCENT                   (11 << BIAS_SHIFT)
#define BIAS_22_PERCENT                   (12 << BIAS_SHIFT)
#define BIAS_24_PERCENT                   (13 << BIAS_SHIFT)

// OP_MODE Settings
#define OP_MODE_SHIFT                     0
#define OP_MODE_DEEP_SLEEP                (0 << OP_MODE_SHIFT)
#define OP_MODE_2_LEAD                    (1 << OP_MODE_SHIFT)
#define OP_MODE_STANDBY                   (2 << OP_MODE_SHIFT)
#define OP_MODE_3_LEAD                    (3 << OP_MODE_SHIFT)
#define OP_MODE_TEMP_MEAS_TIA_OFF         (6 << OP_MODE_SHIFT)
#define OP_MODE_TEMP_MEAS_TIA_ON          (7 << OP_MODE_SHIFT)

#define FET_SHORT_SHIFT                   7
#define FET_SHORT_ENABLED                 (1 << FET_SHORT_SHIFT)
#define FET_SHORT_DISABLED                (0 << FET_SHORT_SHIFT)

#endif                                                        // HEADER_FILE_TI_LMP91000_H

