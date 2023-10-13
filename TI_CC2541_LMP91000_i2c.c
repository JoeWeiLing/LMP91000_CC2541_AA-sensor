//******************************************************************************
//  Description:  This file contains functions that allow the CC2541 device to
//  access the I2C interface of the LMP91000, by using the OSAL I2C HAL. 
//  It also includes some application-specific code for setting up
//  the LMP91000 and for operating it.
// 
//   Texas Instruments Inc.
//   October 2012
//******************************************************************************
// Change Log:
//******************************************************************************
// Version:  1.00
// Comments: Initial Release Version
//******************************************************************************

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



#include <string.h> // included for memset
#include "bcomdef.h"
#include "OSAL.h"
#include "OSAL_PwrMgr.h"

#include "OnBoard.h"
#include "hal_adc.h"
#include "hal_led.h"
#include "hal_key.h"
#include "hal_lcd.h"
#include "hal_i2c.h"

#include "TI_LMP91000.h"
#include "TI_CC2541_LMP91000_i2c.h"
#include "TI_LMP91000_register_settings.h"

#include "sensorBLEPeripheral.h" // Included to pick up some app-specific compile options


uint8 lmp91kOK=1;           // Set to 0 if there are problems communicating with the LMP91000
uint8 lmp91KinitComplete=0; // Set to 1, once init is complete


/**************************************************************************************************
 * @fn         LMP91000_I2CWriteReg
 *
 * @brief      Writes to an LMP91000 register
 *
 * @param[in]      unsigned char device_i2c_address - I2C bus address of the device
 * @param[in]      unsigned char reg_address - register address
 * @param[in]      unsigned char data  - the data to write
 *
 * @return      unsigned char (1 for success, 0 for fail)
 **************************************************************************************************
 */
void LMP91000_I2CWriteReg(unsigned char device_i2c_address, unsigned char reg_address , unsigned char data)
{
  uint8 write_buff[2];
  write_buff[0]=reg_address;
  write_buff[1]=data;
  HalI2CWrite(device_i2c_address,2, write_buff);
} // LMP91000_I2CWriteReg()

/**************************************************************************************************
 * @fn         LMP91000_I2CReadReg
 *
 * @brief      Reads from the specified LMP91000 register
 *
 * @param[in]      unsigned char device_i2c_address - I2C bus address of the device
 * @param[in]      unsigned char reg_address - register address

 *
 * @return     unsigned char - the value read
 **************************************************************************************************
 */
unsigned char LMP91000_I2CReadReg(unsigned char device_i2c_address, unsigned char reg_address)
{
  uint8 buff[2];
  
  // First, do a pointer-set transaction
  buff[0]=reg_address;
  HalI2CWrite(device_i2c_address,1, buff);
  
  // Now, do the read
  memset(buff, 0, sizeof(buff));
  HalI2CRead(device_i2c_address, 1, buff);
  return buff[0];
} // end LMP91000_I2CReadReg()

/**************************************************************************************************
 * @fn         LMP91000_I2CWriteAndConfirmReg
 *
 * @brief      Writes to an LMP91000 register, and confirmst that the value can be read back
 *
 * @param[in]      unsigned char device_i2c_address - I2C bus address of the device
 * @param[in]      unsigned char reg_address - register address
 * @param[in]      unsigned char data  - the data to write
 *
 * @return      unsigned char (1 for success, 0 for fail)
 **************************************************************************************************
 */
unsigned char LMP91000_I2CWriteAndConfirmReg(unsigned char device_i2c_address, unsigned char reg_address , unsigned char data)
{
   uint8 readval;
   LMP91000_I2CWriteReg(device_i2c_address, reg_address, data);
   readval = LMP91000_I2CReadReg(device_i2c_address, reg_address);
   if (readval == data)
   {
     return(1);
   }
   else
   {
     return(0);
   }
} // end LMP91000_I2CWriteAndConfirmReg()


/**************************************************************************************************
 * @fn          LMP91000_I2CSetMode
 *
 * @brief       Sets and verifies mode of the LMP91000
 *
 * @param[in]       unsigned char device_i2c_address - address of the device
 * @param[in]       unsigned char newmode
 *
 * @return      unsigned char (1 for success, 0 for fail
 **************************************************************************************************
 */
unsigned char LMP91000_I2CSetMode(unsigned char device_i2c_address, unsigned char newmode)
{
  uint8 retval;
  retval = 1;
  
  if (!LMP91000_I2CWriteAndConfirmReg(LMP91000_I2C_Address,TI_LMP91000_MODECN_REG, newmode))
  {
    retval = 0;
  }
  return(retval);
} // end LMP91000_I2CSetMode()


/**************************************************************************************************
 * @fn          LMP91000_I2CConfigureForO2Sensor
 *
 * @brief       Configures LMP91000 for O2 Sensor measurement.
 *
* @param[in]   unsigned char tia_gain - LMP91000 TIA Gain
 *                                       TIA_GAIN_EXT_RESIST, TIA_GAIN_2_75_OHM, TIA_GAIN_3_5_OHM,
 *                                       TIA_GAIN_7_OHM, TIA_GAIN_14_OHM, TIA_GAIN_35_OHM,
 *                                       TIA_GAIN_120_OHM, or TIA_GAIN_350_OHM 
 * @param[in]   unsigned char r_load - LMP91000 Resistive Load
 *                                       R_LOAD_10_OHM, R_LOAD_30_OHM, R_LOAD_50_OHM, or R_LOAD_100_OHM 
 * @param[in]   unsigned char ref_source - LMP91000 Reference Source
 *                                       REF_SOURCE_INTERNALor REF_SOURCE_EXTERNAL
 * @param[in]   unsigned char internal_zero - LMP91000 Internal Zero (VREF Divider)
 *                                       INT_Z_SEL_20_PERCENT, INT_Z_SEL_50_PERCENT, 
 *                                       INT_Z_SEL_67_PERCENT, or INT_Z_SEL_BYPASS
 *
 * @return      unsigned char (1 for success, 0 for fail
 **************************************************************************************************
 */
unsigned char LMP91000_I2CConfigureForO2Sensor(unsigned char tia_gain, unsigned char r_load, 
                                               unsigned char ref_source, unsigned char internal_zero)
{
  
  // Set for gain resistor and load
  if (!LMP91000_I2CWriteAndConfirmReg(LMP91000_I2C_Address, TI_LMP91000_TIACN_REG, (tia_gain | r_load)))
  {
     lmp91kOK=0;
  }
     
  // Set internal/external ref, internal zero setting, 0 bias level, and negative bias sign
  if (!LMP91000_I2CWriteAndConfirmReg(LMP91000_I2C_Address, TI_LMP91000_REFCN_REG, 
                                      (REF_SOURCE_EXTERNAL | INT_Z_SEL_50_PERCENT | BIAS_24_PERCENT | BIAS_SIGN_POSITIVE)))
  {
    lmp91kOK=0;
  }

  if (lmp91kOK!=0)
  {
    return (1);
  }
  else
  {
    return(0);
  }
} // end LMP91000_I2CConfigureForO2Sensor()


/**************************************************************************************************
 * @fn          LMP91000_I2CSwitchVoutToO2Sensor
 *
 * @brief       Configures LMP91000 to output O2 Sensor measurement on Vout.
 *
 * @param[in]   unsigned char op_mode - LMP91000 Operational Mode
 *                                       OP_MODE_DEEP_SLEEP, OP_MODE_2_LEAD, OP_MODE_STANDBY,
 *                                       OP_MODE_3_LEAD, OP_MODE_TEMP_MEAS_TIA_OFF, or OP_MODE_TEMP_MEAS_TIA_ON 
 *
 * @return      unsigned char (1 for success, 0 for fail
 **************************************************************************************************
 */
unsigned char LMP91000_I2CSwitchVoutToO2Sensor(unsigned char op_mode)
{
   // Configure LMP9100 to 3-wire mode, and to output value from o2 sensor
  if (!LMP91000_I2CSetMode(LMP91000_I2C_Address, op_mode))
  {
    lmp91kOK = 0;
  }

  if (lmp91kOK!=0)
  {
     return (1);
  }
  else
  {
    return(0);
  }
} // end LMP91000_I2CSwitchVoutToO2Sensor()


/**************************************************************************************************
 * @fn          LMP91000_I2CSwitchVoutToTempSensor
 *
 * @brief       Configures LMP91000 to output Temp Sensor measurement on Vout.
 *              As a side-effect, puts the O2 Sensor output on C2
 *
 * @param       none
 *
 * @return      unsigned char (1 for success, 0 for fail
 **************************************************************************************************
 */
unsigned char LMP91000_I2CSwitchVoutToTempSensor(void)
{
  if (!LMP91000_I2CSetMode(LMP91000_I2C_Address, TI_LMP91000_TIATEMP_MEASURE_MODE))
  {
    lmp91kOK = 0;
  }

  if (lmp91kOK!=0)
  {
    return (1);
  }
  else
  {
    return(0);
  } 
} // end LMP91000_I2CSwitchVoutToTempSensor()



/**************************************************************************************************
 * @fn          LMP91000_I2CInitialSetup
 *
 * @brief       Performs initial config of the LMP91000
 *
 * @param[in]   unsigned char tia_gain - LMP91000 TIA Gain
 *                                       TIA_GAIN_EXT_RESIST, TIA_GAIN_2_75_OHM, TIA_GAIN_3_5_OHM,
 *                                       TIA_GAIN_7_OHM, TIA_GAIN_14_OHM, TIA_GAIN_35_OHM,
 *                                       TIA_GAIN_120_OHM, or TIA_GAIN_350_OHM 
 * @param[in]   unsigned char r_load - LMP91000 Resistive Load
 *                                       R_LOAD_10_OHM, R_LOAD_30_OHM, R_LOAD_50_OHM, or R_LOAD_100_OHM 
 * @param[in]   unsigned char ref_source - LMP91000 Reference Source
 *                                       REF_SOURCE_INTERNALor REF_SOURCE_EXTERNAL
 * @param[in]   unsigned char internal_zero - LMP91000 Internal Zero (VREF Divider)
 *                                       INT_Z_SEL_20_PERCENT, INT_Z_SEL_50_PERCENT, 
 *                                       INT_Z_SEL_67_PERCENT, or INT_Z_SEL_BYPASS
 * @param[in]   unsigned char op_mode - LMP91000 Operational Mode
 *                                       OP_MODE_DEEP_SLEEP, OP_MODE_2_LEAD, OP_MODE_STANDBY,
 *                                       OP_MODE_3_LEAD, OP_MODE_TEMP_MEAS_TIA_OFF, or OP_MODE_TEMP_MEAS_TIA_ON 
 *
 * @return      unsigned char (1 for success, 0 for fail
 **************************************************************************************************
 */
unsigned char LMP91000_I2CInitialSetup(unsigned char tia_gain, unsigned char r_load, 
                                       unsigned char ref_source, unsigned char internal_zero,
                                       unsigned char op_mode)
{
  int count;
  int status;
  uint8 read_val[2];
  
  // Wait until LMP91000 is ready, or until timeout occurs
   count=0;
   status = TI_LMP91000_NOT_READY;
   
   while ((status == TI_LMP91000_NOT_READY) && (count<20000) )
   {
    status = LMP91000_I2CReadReg(LMP91000_I2C_Address, TI_LMP91000_STATUS_REG);               
    count++;
   }
   
  if (status == TI_LMP91000_NOT_READY)
  {
     lmp91kOK = 0;    // Indicate error, but continue anyway, to try to drive past intermittent connections
  }
 
  LMP91000_I2CWriteReg(LMP91000_I2C_Address, TI_LMP91000_LOCK_REG, TI_LMP91000_WRITE_UNLOCK);   // Unlock the registers for write
 
  // The following two register writes were from the original code, just to verify that
  // we can write to and read from the part.  The final configuration is set further below
  LMP91000_I2CWriteReg(LMP91000_I2C_Address, TI_LMP91000_TIACN_REG, TIACN_NEW_VALUE);           // Modify TIA control register 
  LMP91000_I2CWriteReg(LMP91000_I2C_Address, TI_LMP91000_REFCN_REG, REFCN_NEW_VALUE);           // Modify REF control register
  read_val[0] = LMP91000_I2CReadReg(LMP91000_I2C_Address, TI_LMP91000_TIACN_REG);               // Read to confirm register is modified
  read_val[1] = LMP91000_I2CReadReg(LMP91000_I2C_Address, TI_LMP91000_REFCN_REG);               // Read to confirm register is modified  
  
  if ( (read_val[0] != TIACN_NEW_VALUE) || (read_val[1] != REFCN_NEW_VALUE) )                   // Test values took effect
  {
     lmp91kOK = 0;    // otherwise error
  }
  
  
  // Here, we make the settings that are as we desire for operation for our board.

  // First, configure the board for the correct load and gain (and for now, output 
  // the O2 sensor on Vout).
  //LMP91000_I2CConfigureForO2Sensor(tia_gain, r_load, ref_source, internal_zero);
  LMP91000_I2CWriteReg(LMP91000_I2C_Address, TI_LMP91000_TIACN_REG, TIACN_NEW_VALUE);           // Modify TIA control register 
  LMP91000_I2CWriteReg(LMP91000_I2C_Address, TI_LMP91000_REFCN_REG, REFCN_NEW_VALUE);           // Modify REF control register
  LMP91000_I2CSwitchVoutToO2Sensor(op_mode);

   // Configure LMP9100 to output temp on vout, and o2 on C2
   LMP91000_I2CSwitchVoutToTempSensor();
 
   
  #if USE_SEPARATE_TEMP_AD_CHANNEL==1
    LMP91000_I2CWriteReg(LMP91000_I2C_Address, TI_LMP91000_LOCK_REG, TI_LMP91000_WRITE_LOCK);          // lock the registers 
  #else
    // In muxed mode, we won't lock the LMP91000, as we will continue to re-configure it every 100 msec or so, and we'd like
    // to do that as quickly as possible.
  #endif
  
  // save I2C registers so that they're preserved when OSAL sleeps
  HalI2CEnterSleep(); 
  
   // Indicate to the data update task that the LMP91000 is available for use
   // The data update task -may- alter the LMP91000 if we're in multiplexed mode.
   lmp91KinitComplete=1;
  
  if (lmp91kOK!=0)
  {
    return (1);
  }
  else
  {
    return(0);
  }
} // end LMP91000_I2CInitialSetup()

