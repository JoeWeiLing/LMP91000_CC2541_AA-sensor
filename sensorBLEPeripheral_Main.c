/**************************************************************************************************
  Filename:       sensorBLEBroadcaster_Main.c
  Revised:        $Date: 2012-10-19 $
  Revision:       $Revision:$

  Description:    This file contains the main and callback functions for
                  the sensor BLE Broadcaster sample application.

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
  PROVIDED “AS IS" WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESS OR IMPLIED,
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

/**************************************************************************************************
 *                                           Includes
 **************************************************************************************************/
/* Hal Drivers */

#include "hal_types.h"
#include "hal_key.h"
#include "hal_timer.h"
#include "hal_drivers.h"
#include "hal_led.h"
#include "hal_adc.h"
#include "hal_i2c.h"

/* OSAL */
#include "OSAL.h"
#include "OSAL_Tasks.h"
#include "OSAL_PwrMgr.h"
#include "osal_snv.h"
#include "OnBoard.h"

#include "sensorBLEPeripheral.h"

#include "TI_LMP91000.h"
#include "TI_CC2541_LMP91000_i2c.h"
#include "TI_LMP91000_register_settings.h"


/*********************************************************************
 * CONSTANTS
 */

// How often to collect sensor data periodic event
#define DEFAULT_DATA_COLLECT_PERIOD           1000

/**************************************************************************************************
 * EXTERN GLOBAL VARIABLES
 **************************************************************************************************/
extern uint8 DataReadyFlag;
    

/**************************************************************************************************
 * FUNCTIONS
 **************************************************************************************************/

/* This callback is triggered when a key is pressed */
void MSA_Main_KeyCallback(uint8 keys, uint8 state);

/**************************************************************************************************
 * @fn          main
 *
 * @brief       Start of application.
 *
 * @param       none
 *
 * @return      none
 **************************************************************************************************
 */
int main(void)
{
  /* Initialize hardware */
  HAL_BOARD_INIT();

  // Initialize board I/O
  InitBoard( OB_COLD );

  /* Initialze the HAL driver */
  HalDriverInit();

  /* Initialize NV system */
  osal_snv_init();
  
  /* Initialize the operating system */
  osal_init_system();

  /* Enable interrupts */
  HAL_ENABLE_INTERRUPTS();

  // Final board initialization
  InitBoard( OB_READY );

  #if defined ( POWER_SAVING )
    osal_pwrmgr_device( PWRMGR_BATTERY );
  #endif
    
  /* Start OSAL */
  osal_start_system(); // No Return from here

  return 0;
} // end main()

/**************************************************************************************************
                                           CALL-BACKS
**************************************************************************************************/


/*************************************************************************************************
**************************************************************************************************/
// Set COMPUTE_91K_TEMP_WITH_TABLE to 1 to use a table lookup (scan) to convert
// from millivolts to degrees C.  If COMUTE_91K_TEMP_WITH_TABLE is 0, we'll 
// do a fit to a pre-computed curve
#define COMPUTE_91K_TEMP_WITH_TABLE 0

#if COMPUTE_91K_TEMP_WITH_TABLE
int tempTable[] =
{
 1875,  /* 1875 mv at -40 C */
 1867, 
 1860, 
 1852, 
 1844, 
 1836, 
 1828, 
 1821, 
 1813, 
 1805, 
 1797, 
 1789, 
 1782, 
 1774, 
 1766, 
 1758, 
 1750, 
 1742, 
 1734, 
 1727, 
 1719, 
 1711, 
 1703, 
 1695, 
 1687, 
 1679, 
 1671, 
 1663, 
 1656, 
 1648, 
 1640, 
 1632, 
 1624, 
 1616, 
 1608, 
 1600, 
 1592, 
 1584, 
 1576, 
 1568, 
 1560, /* 0 */
 1552, 
 1544, 
 1536, 
 1528, 
 1520, 
 1512, 
 1504, 
 1496, 
 1488, 
 1480, 
 1472, 
 1464, 
 1456, 
 1448, 
 1440, 
 1432, 
 1424, 
 1415, 
 1407, 
 1399, 
 1391, 
 1383, 
 1375, 
 1367, 
 1359, 
 1351, 
 1342, 
 1334, 
 1326, 
 1318, 
 1310, 
 1302, 
 1293, 
 1285, 
 1277, 
 1269, 
 1261, 
 1253, 
 1244, 
 1236, 
 1228, 
 1220, 
 1212, 
 1203, 
 1195, 
 1187, 
 1179, 
 1170, 
 1162, 
 1154, 
 1146, 
 1137, 
 1129, 
 1121, 
 1112, 
 1104, 
 1096, 
 1087, 
 1079, 
 1071, 
 1063, 
 1054, 
 1046, 
 1038, 
 1029, 
 1021, 
 1012, 
 1004, 
 996, 
 987, 
 979, 
 971, 
 962, 
 954, 
 945, 
 937, 
 929, 
 920, 
 912, 
 903, 
 895, 
 886, 
 878, 
 870, 
 861, /* 85 */
};
#endif

/**************************************************************************************************
 * @fn          convertTemp
 *
 * @brief       Convert tempval to tenths of degrees C, based on the table in the datasheet for the LMP91000
 *
 * @param       int16 original value
 *
 * @return      int16 converted value
 **************************************************************************************************
 */
int16 convertTemp(int16 tempval)
{
  int16 newtemp;
  int16 millivolts;
#if COMPUTE_91K_TEMP_WITH_TABLE
  int index;
#else
  // No extra variables needed for the curve-fit
#endif
  
  // Clamp at 0, shouldn't be negative
  if (tempval <0) 
  {
    tempval = 0; 
  }
  
  // Convert to millivolts, assuming 12 bit conversion (2047 max code)
  // and a 3.0 volt external reference
  millivolts = (int16) ((tempval * 3000.0) / 2047.0);

#if COMPUTE_91K_TEMP_WITH_TABLE  
  // The table is -close- to linear, but not quite, so just walk it to find 
  // the range of values of interest.
  index = 0; // -40C
  while ((index<40+85) && (tempTable[index]>millivolts) )
  {
    index++;
  }
  // At this point, index is indicating the table element that is
  // greater than the voltage reading (unless index==0)
  // If we ran past the end of the table (85 degrees), index
  // will be pointing to the last element of the table (close enough).
  if (index==0)
  {
    // fudge it, and bump the index by 1
    index++;
  }
  
  newtemp = (index-41)*10;
#else
  // Instead of the table lookup above, here's a good answer
  // TinDegreesCelsius = millivolts * (millivolts * A + B) + C
  // where
  // A = -5.23385 x 10-6
  // B = -0.108906
  // C = 182.635
#define C_val (182.635)
#define B_val (-0.108906)
#define A_val (-5.23385E-6)
  newtemp = (int16) (10.0 * ( ((float) millivolts) * ( ( ( (float) millivolts) * A_val) + (B_val)) + C_val));
#endif  
  return(newtemp);
} // end convertTemp()
       




static uint8 DataUpdate_TaskID;   // Task ID for internal task/event processing


/**************************************************************************************************
 * @fn          DataUpdateTask_Init
 *
 * @brief       Set up task and variables for the data update task
 *
 * @param       uint8 task_id
 *
 * @return      none
 **************************************************************************************************
 */
void DataUpdateTask_Init( uint8 task_id )
{
  DataUpdate_TaskID = task_id;
  
  //                                          event_id   ms
  osal_start_reload_timer (DataUpdate_TaskID,    0x1,   DEFAULT_DATA_COLLECT_PERIOD);
} // end DataUpdateTask_Init()

static int16 oldtempval;   // old value read from temp sensor
static int16 oldsensorval; // old value read from O2 sensor
static int8 cyclecount;    // used for operating the LED

/**************************************************************************************************
 * @fn          DataUpdate_ProcessEvent
 *
 * @brief       Update values from A/D sensor and diags
 *
 * @param       uint8 task_id
 * @param       uint16 events - event mask
 *
 * @return      uint16 - 0 (for now)
 **************************************************************************************************
 */
uint16 DataUpdate_ProcessEvent(uint8 task_id, uint16 events)
{
    #if ADV_DEBUG_MESSAGE_FORMAT==1
    // variables that are needed if we're running the advanced debugging message format
    int vdd_div_3;
    int vdd;
    int16 tempvalCC2541;
    int spare;
    #endif
    
    int tempval;
    int16 tempval2;
    int16 tempval3;
    int16 tempval4;
    int16 tempavg;
    uint16 timeval;
    uint16 sensorval;
#if USE_SEPARATE_TEMP_AD_CHANNEL==0
    uint8 lmp_configured;
    uint8 lmp_configure_tries;
#endif

#ifdef FAKE_SENS_DATA

#ifdef O2_SENSOR
    uint16 max_fake = 5000;
    uint16 min_fake = -5000;
    static int16  fake_adj = 1;
    static uint16 fakesensorval = 600;
#endif
#ifdef CO_SENSOR
    uint16 max_fake = 2047;
    uint16 min_fake = 0;
    static int16  fake_adj = 1;
    static uint16 fakesensorval = 380;
#endif

    
#endif    
    if (events & 1)
    {

      timeval = (uint16) (osal_GetSystemClock() & 0xffff);
      cyclecount++;
      if (cyclecount>9)
      {
        cyclecount=0;
        // Also, set P1_0 (the LED) as an output, and drive high
        P1DIR = P1DIR | 0x01; 
        P1 = P1 | 0x01;
      }
      
      
       /* Enable channel */
       ADCCFG |= 0x80;
       P0DIR = 0x7C; // force P0.0, P0.1, and P0.7 to be inputs
       APCFG = 0x83;
       HalAdcSetReference (0x80); // use AIN7 for ref (0x80 would be AVDD5, 0x00 would be internal ref

#if USE_SEPARATE_TEMP_AD_CHANNEL==0
       // Configure LMP9100 to output temperature
       if (lmp91KinitComplete)
       {
         // Because the processor may have been sleeping, re-configure I2C intf. before 
         // performing the I2C transaction
         HalI2CExitSleep();
         lmp_configure_tries=0;
         lmp_configured=0;
       
         // We can't hang out in this loop forever, but we can retry a couple
         // of times, to get over any instability on the I2C bus
         while ((lmp_configure_tries<4) && (!lmp_configured))
         {
            // Set this flag so that we presume that communication is working.
           // The flag will get cleared (quickly) as a side-effect in the
           // communication routines if we fail again.
           lmp91kOK=1; 

           lmp_configured=LMP91000_I2CSwitchVoutToTempSensor();
           
           lmp_configure_tries++;
         } // end while 
         
          // Because the processor may go into sleep mode, save the state of the I2C
          // interface before continuing.
          HalI2CEnterSleep();
       } // end if (lmp91KinitComplete)
#endif       
       
       
       // Read temperature from LMP9100
       if (!lmp91kOK)
       {
         // If we're not communicating, then the LMP91000 may not be in the right
         // state, so just use a previous value.
         tempval = oldtempval;
       }
       else
       {
         tempval =  HalAdcRead(HAL_ADC_CHANNEL_0, HAL_ADC_RESOLUTION_12);
         tempval2 =  HalAdcRead(HAL_ADC_CHANNEL_0, HAL_ADC_RESOLUTION_12);
         tempval3 =  HalAdcRead(HAL_ADC_CHANNEL_0, HAL_ADC_RESOLUTION_12);
         tempval4 =  HalAdcRead(HAL_ADC_CHANNEL_0, HAL_ADC_RESOLUTION_12);
         // Get a bit of noise out of the temperature measurment by averaging 4
         // samples.
         // By the way, we expect nominal values to be around 1100 or so
         // at room temperature, so we can fairly safely add 4 16-bit values
         // together without thinking too much about overflow.
         // If the nominal values were larger, we'd promote to a larger
         // data type before averaging.
         tempavg = (tempval+tempval2+tempval3+tempval4)/4;
         oldtempval=tempavg;
       }
       
       
       // Convert temp to tenths of degrees C, based on the table in
       // the datasheet for the LMP91000, and assuming a 2.5v ref
       
       tempval = convertTemp(tempavg);

       // Now, get ready to measure the oxygen sensor
       HalAdcSetReference (0x80); // use AIN7 for ref 
       
#if USE_SEPARATE_TEMP_AD_CHANNEL==0
       
       if (lmp91KinitComplete)
       {
         // Because the processor may have been sleeping, re-configure I2C intf. before 
         // performing the I2C transaction(s)
         HalI2CExitSleep();
         
         lmp_configure_tries=0;
         lmp_configured=0;
         // we can't hang out in this loop forever, but we can retry a couple
         // of times, to get over any instability on the I2C
         while ((lmp_configure_tries<4) && (!lmp_configured))
         {
           // Set this flag so that we presume that communication is working.
           // The flag will get cleared (quickly) as a side-effect in the
           // communication routines if we fail again.
           lmp91kOK=1; 
           
           // Likely redundant, but doesn't hurt
           LMP91000_I2CConfigureForO2Sensor( SensTIAGain , SensRLoad, SensRefVoltageSource, SensIntZSel);
           LMP91000_I2CWriteReg(LMP91000_I2C_Address, TI_LMP91000_LOCK_REG, TI_LMP91000_WRITE_UNLOCK);   // Unlock the registers for write
           LMP91000_I2CWriteReg(LMP91000_I2C_Address, TI_LMP91000_TIACN_REG, TIACN_NEW_VALUE);           // Modify TIA control register 
           LMP91000_I2CWriteReg(LMP91000_I2C_Address, TI_LMP91000_REFCN_REG, REFCN_NEW_VALUE);           // Modify REF control register
           LMP91000_I2CWriteAndConfirmReg(LMP91000_I2C_Address, TI_LMP91000_REFCN_REG, 
                                      (REF_SOURCE_EXTERNAL | INT_Z_SEL_50_PERCENT | BIAS_24_PERCENT | BIAS_SIGN_POSITIVE));
           if (lmp91kOK)
           {
             lmp_configured = LMP91000_I2CSwitchVoutToO2Sensor(SensModeOp);
           }
 
           lmp_configure_tries++;
         } // end while
         
         // Because the processor may go into sleep mode, save the state of the I2C
         // interface before continuing.
         HalI2CEnterSleep();
       } // end  if (lmp91KinitComplete)

#endif
       
       if (!lmp91kOK)
       {
         // If we're not communicating, then the LMP91000 may not be in the right
         // state, so just use a previous value.
         LMP91000_I2CWriteAndConfirmReg(LMP91000_I2C_Address, TI_LMP91000_REFCN_REG, 
                                      (REF_SOURCE_EXTERNAL | INT_Z_SEL_50_PERCENT | BIAS_24_PERCENT | BIAS_SIGN_POSITIVE));
         sensorval = oldsensorval;
       }
       else
       {
#if    USE_SEPARATE_TEMP_AD_CHANNEL==1
       sensorval =  HalAdcRead(HAL_ADC_CHANNEL_1,HAL_ADC_RESOLUTION_12);
#else
       sensorval =  HalAdcRead(HAL_ADC_CHANNEL_0,HAL_ADC_RESOLUTION_12);
#endif
         oldsensorval = sensorval;
       }
       
       // Depending on compile options, build the message, or gather
       // additional diagnostic info and then build the debug mode message
#ifdef FAKE_SENS_DATA
fakesensorval += fake_adj;
if ((fakesensorval >= max_fake) || (fakesensorval <= min_fake))
  fake_adj = -1 * fake_adj;
sensorval = fakesensorval;
#endif
     
       #if ADV_DEBUG_MESSAGE_FORMAT==0
         updateSensorData ((uint16)timeval, (uint16)tempval, (uint16)sensorval);
         DataReadyFlag = 1;
       #else
         // Gather additional interesting diagnostic info before updating structure
         spare = HalAdcRead(0x01, HAL_ADC_RESOLUTION_12); // Spare A/D chan - use for battery measurement later
  
         // Turn on the test mode to enable temperature measurement 
         // from the CC2544's internal temp sensor
         ATEST=1; // ATEST.ATEST_CTRL=0x01;
         TR0=1;   //  TR0.ADCTM=1;
        
         HalAdcSetReference(0); // use internal ref voltage (1.15 V)
         //HalAdcSetReference ( 0x80); // use AVDD5 pin for ref
         
         // CC2541 Internal temp sensor is A/D input 14 (0x0e)
         tempvalCC2541 = HalAdcRead(0x0E, HAL_ADC_RESOLUTION_12); 

         /* Turn off test modes */
         TR0=0; //        TR0.ADCTM=0;
         ATEST=0;

        // The analog temperature sensor in the CC2544 should give back a value 
        // of 1480 at 25 degrees C and VDD=3, 
        // and will change by a value of 4.5 per degree C
        //     
        // So, to get temperature in 0.1 degrees C units, consider the 
        // following formula:
        //  
        tempval2 = tempvalCC2541-1480;
        tempvalCC2541 = (int16) (250.0 + (tempval2/4.5));
      
        HalAdcSetReference(0); // use internal ref voltage (1.15 V)
          
        // Pick up VDD divided by 3
        vdd_div_3 = HalAdcRead(0x0F, HAL_ADC_RESOLUTION_12); // VDD/3
        // Convert to millivolts (and get rid of the divide by 3, since we're doing math
        // vdd = (int) (1.15*vdd_div_3*3.0*1000.0 / 2047.0); // convert to millivolts
        // vdd = (int) (vdd_div_3 * 1.6853932584269662921348314606742); // more precisely
        vdd = (int) (vdd_div_3 * 1.6853); // close enough
        
        // Pick up the spare A/D input
        HalAdcSetReference (0x80); // use AIN7 for ref
        spare = HalAdcRead(0x01, HAL_ADC_RESOLUTION_12); 

        updateSensorData ((uint16)timeval, (uint16)tempval, (uint16)sensorval, (uint16)tempvalCC2541, (uint16)vdd, (uint16)spare);
        DataReadyFlag = 1;
        
        #endif
        
        // Also, set P1_0 (the LED) as an output, and drive low  
        P1DIR = P1DIR | 0x01; 
        P1 = P1&0xFE;

        
        return (events ^ 1);;
    }
    
    return (0);
} // end DataUpdate_ProcessEvent()
