/**************************************************************************************************
  Filename:       sensorBLEBroadcaster.c
  Revised:        $Date: 2012 $
  Revision:       $Revision: $

  Description:    This file contains the Sensor BLE Broadcaster sample application 
                  for use with the CC2540 Bluetooth Low Energy Protocol Stack.

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

/*********************************************************************
 * INCLUDES
 */

#include "bcomdef.h"
#include "OSAL.h"
#include "OSAL_PwrMgr.h"

#include "OnBoard.h"
#include "hal_adc.h"
#include "hal_led.h"
#include "hal_key.h"
#include "hal_lcd.h"
#include "hal_i2c.h"

#include "hci.h"
#include "gap.h"

#include "linkdb.h"
#include "gatt.h"
#include "gapgattserver.h"
#include "gattservapp.h"
#include "sensorservice.h"
#include "devinfoservice.h"
#include "battservice.h"
#include "peripheral.h"
#include "gapbondmgr.h"
#include "sensorBLEPeripheral.h"

#include "TI_LMP91000.h"
#include "TI_CC2541_LMP91000_i2c.h"
#include "TI_LMP91000_register_settings.h"

#if defined O2_SENSOR
#include "O2_Sensor_Settings.h"
#elif defined CO_SENSOR  
#include "CO_Sensor_Settings.h"
#endif

/*********************************************************************
 * MACROS
 */

/*********************************************************************
 * CONSTANTS
 */

//// What is the advertising interval when device is discoverable (units of 625us, 160=100ms)
//#define DEFAULT_ADVERTISING_INTERVAL          160

// Company Identifier: Texas Instruments Inc. (13)
//#define TI_COMPANY_ID                         0x000D

// Fast advertising interval in 625us units
#define DEFAULT_FAST_ADV_INTERVAL             32

// Duration of fast advertising duration in ms
#define DEFAULT_FAST_ADV_DURATION             30000

// Slow advertising interval in 625us units
#define DEFAULT_SLOW_ADV_INTERVAL             1704     // 1065 ms

// Duration of slow advertising duration in ms (set to 0 for continuous advertising)
#define DEFAULT_SLOW_ADV_DURATION             0

// How often to perform sensor data send periodic event in ms - use 10ms intervals as that is the tick size
#define DEFAULT_SENS_PERIOD                   1000

// Whether to enable automatic parameter update request when a connection is formed
#define DEFAULT_ENABLE_UPDATE_REQUEST         FALSE

// Minimum connection interval (units of 1.25ms) if automatic parameter update request is enabled
#define DEFAULT_DESIRED_MIN_CONN_INTERVAL     32

// Maximum connection interval (units of 1.25ms) if automatic parameter update request is enabled
#define DEFAULT_DESIRED_MAX_CONN_INTERVAL     64

// Slave latency to use if automatic parameter update request is enabled
#define DEFAULT_DESIRED_SLAVE_LATENCY         0

// Supervision timeout value (units of 10ms) if automatic parameter update request is enabled
#define DEFAULT_DESIRED_CONN_TIMEOUT          500

// Battery level is critical when it is less than this %
#define DEFAULT_BATT_CRITICAL_LEVEL           6 

// Battery measurement period in ms
#define DEFAULT_BATT_PERIOD                   15000


/*********************************************************************
 * TYPEDEFS
 */

/*********************************************************************
 * GLOBAL VARIABLES
 */
uint8 DataReadyFlag = 0;

/*********************************************************************
 * EXTERNAL VARIABLES
 */

/*********************************************************************
 * EXTERNAL FUNCTIONS
 */

/*********************************************************************
 * LOCAL VARIABLES
 */
static uint8 sensBLEPeripheral_TaskID;   // Task ID for internal task/event processing

static gaprole_States_t gapProfileState = GAPROLE_INIT;

// GAP - SCAN data (max size = 31 bytes)
static uint8 scanData[] =
{
  // Complete name
  0x0B,   // length of this data
  GAP_ADTYPE_LOCAL_NAME_COMPLETE,
  0x4A,   // 'J'
  0x6F,   // 'o'
  0x65,   // 'e'
  0x57,   // 'W'
  0x65,   // 'e'
  0x69,   // 'i'
  0x4C,   // 'L'
  0x69,   // 'i'
  0x6E,   // 'n'
  0x67,   // 'g'

  // TX Power Level
  0x02,    // Length of this data
  GAP_ADTYPE_POWER_LEVEL,  
  0x00,   // 0 dBm
};


static uint8 advertData[] = 
{ 
  // flags
  0x02,
  GAP_ADTYPE_FLAGS,
  GAP_ADTYPE_FLAGS_GENERAL | GAP_ADTYPE_FLAGS_BREDR_NOT_SUPPORTED,
  // service UUIDs
  0x05,
  GAP_ADTYPE_16BIT_MORE,
  LO_UINT16(SENS_SERV_UUID),
  HI_UINT16(SENS_SERV_UUID),
  LO_UINT16(BATT_SERVICE_UUID),
  HI_UINT16(BATT_SERVICE_UUID)
};

// Device name attribute value
static uint8 attDeviceName[GAP_DEVICE_NAME_LEN] = "O2 Wireless Sensor - Peripheral";

// GAP connection handle
static uint16 gapConnHandle;

// O2 Sensor measurement value stored in this structure
static attHandleValueNoti_t sensMeas;

uint8 errCnt=0;

// Advertising user-cancelled state
static bool sensAdvCancelled = FALSE;

static bool sens_running = FALSE;



/*********************************************************************
 * LOCAL FUNCTIONS
 */
static void sensBLEPeripheral_ProcessOSALMsg( osal_event_hdr_t *pMsg );
static void sensGapStateCB( gaprole_States_t newState );
static void sensPeriodicTask( void );
static void sensBattPeriodicTask( void );
static void sens_HandleKeys( uint8 shift, uint8 keys );
static void sensMeasNotify(void);
static void sensCB(uint8 event);
static void sensBattCB(uint8 event);


/*********************************************************************
 * PROFILE CALLBACKS
 */

// GAP Role Callbacks
static gapRolesCBs_t sensPeripheralCB =
{
  sensGapStateCB,                 // Profile State Change Callbacks
  NULL                            // When a valid RSSI is read from controller
};

// Bond Manager Callbacks
static const gapBondCBs_t sensBondCB =
{
  NULL,                   // Passcode callback
  NULL                    // Pairing state callback
};

/*********************************************************************
 * PUBLIC FUNCTIONS
 */

/*********************************************************************
 * @fn      sensBLEPeripheral_Init
 *
 * @brief   Initialization function for the sensor BLE Peripheral App
 *          Task. This is called during initialization and should contain
 *          any application specific initialization (ie. hardware
 *          initialization/setup, table initialization, power up
 *          notificaiton ... ).
 *
 * @param   task_id - the ID assigned by OSAL.  This ID should be
 *                    used to send messages and set timers.
 *
 * @return  none
 */
void sensBLEPeripheral_Init( uint8 task_id )
{
  uint32 value32;
  uint16 value16;
  uint8  value8;
  uint8  modeOp;
  uint8  tiaGain;
  uint8  rLoad;
  uint8  refVoltageSource;
  uint8  intZSel;
  
  sensBLEPeripheral_TaskID = task_id;

  // Setup the GAP Peripheral Role Profile
  {
    // For the CC2540DK-MINI keyfob, device doesn't start advertising until button is pressed
    uint8 initial_advertising_enable = TRUE;

    // By setting this to zero, the device will go into the waiting state after
    // being discoverable for 30.72 second, and will not being advertising again
    // until the enabler is set back to TRUE
    uint16 gapRole_AdvertOffTime = 0;
      
    uint8 enable_update_request = DEFAULT_ENABLE_UPDATE_REQUEST;
    uint16 desired_min_interval = DEFAULT_DESIRED_MIN_CONN_INTERVAL;
    uint16 desired_max_interval = DEFAULT_DESIRED_MAX_CONN_INTERVAL;
    uint16 desired_slave_latency = DEFAULT_DESIRED_SLAVE_LATENCY;
    uint16 desired_conn_timeout = DEFAULT_DESIRED_CONN_TIMEOUT;

    // Set the GAP Role Parameters
    GAPRole_SetParameter( GAPROLE_ADVERT_ENABLED, sizeof( uint8 ), &initial_advertising_enable );
    GAPRole_SetParameter( GAPROLE_ADVERT_OFF_TIME, sizeof( uint16 ), &gapRole_AdvertOffTime );
    
    GAPRole_SetParameter( GAPROLE_SCAN_RSP_DATA, sizeof ( scanData ), scanData );
    GAPRole_SetParameter( GAPROLE_ADVERT_DATA, sizeof( advertData ), advertData );
    
    GAPRole_SetParameter( GAPROLE_PARAM_UPDATE_ENABLE, sizeof( uint8 ), &enable_update_request );
    GAPRole_SetParameter( GAPROLE_MIN_CONN_INTERVAL, sizeof( uint16 ), &desired_min_interval );
    GAPRole_SetParameter( GAPROLE_MAX_CONN_INTERVAL, sizeof( uint16 ), &desired_max_interval );
    GAPRole_SetParameter( GAPROLE_SLAVE_LATENCY, sizeof( uint16 ), &desired_slave_latency );
    GAPRole_SetParameter( GAPROLE_TIMEOUT_MULTIPLIER, sizeof( uint16 ), &desired_conn_timeout );
  }
  
  // Set the GAP Characteristics
  GGS_SetParameter( GGS_DEVICE_NAME_ATT, GAP_DEVICE_NAME_LEN, attDeviceName );

  // Setup the GAP Bond Manager
  {
    uint32 passkey = 0; // passkey "000000"
    uint8 pairMode = GAPBOND_PAIRING_MODE_WAIT_FOR_REQ;
    uint8 mitm = FALSE;
    uint8 ioCap = GAPBOND_IO_CAP_DISPLAY_ONLY;
    uint8 bonding = TRUE;
    GAPBondMgr_SetParameter( GAPBOND_DEFAULT_PASSCODE, sizeof ( uint32 ), &passkey );
    GAPBondMgr_SetParameter( GAPBOND_PAIRING_MODE, sizeof ( uint8 ), &pairMode );
    GAPBondMgr_SetParameter( GAPBOND_MITM_PROTECTION, sizeof ( uint8 ), &mitm );
    GAPBondMgr_SetParameter( GAPBOND_IO_CAPABILITIES, sizeof ( uint8 ), &ioCap );
    GAPBondMgr_SetParameter( GAPBOND_BONDING_ENABLED, sizeof ( uint8 ), &bonding );
  }  

  // Setup the Sensor Characteristic Values
  {

    value32 = SENS_DENOM_0;    
    sens_SetParameter( SENS_DENOM_0_PARAM, sizeof ( uint32 ), (void *)&value32 );

    value32 = SENS_NUMER_0;    
    sens_SetParameter( SENS_NUM_0_PARAM, sizeof ( uint32 ), (void *)&value32 );
    
    value32 = SENS_SUB_0;    
    sens_SetParameter( SENS_SUB_X_0_PARAM, sizeof ( uint32 ), (void *)&value32 );

    value32 = SENS_DENOM_1;    
    sens_SetParameter( SENS_DENOM_1_PARAM, sizeof ( uint32 ), (void *)&value32 );

    value16 = SENS_Y_DISPLAY_MIN;    
    sens_SetParameter( SENS_Y_AXIS_DISPLAY_MIN_PARAM, sizeof ( uint16 ), (void *)&value16 );

    value16 = SENS_Y_DISPLAY_MAX;    
    sens_SetParameter( SENS_Y_AXIS_DISPLAY_MAX_PARAM, sizeof ( uint16 ), (void *)&value16 );

    value16 = SENS_GRAPH_TOP_MID_BOUNDARY;    
    sens_SetParameter( SENS_COLOR_BAND_TOP_MID_LEVEL_PARAM, sizeof ( uint16 ), (void *)&value16 );

    value16 = SENS_GRAPH_MID_LOW_BOUNDARY;    
    sens_SetParameter( SENS_COLOR_BAND_MID_LOW_LEVEL_PARAM, sizeof ( uint16 ), (void *)&value16 );

    value32 = SENS_GRAPH_TOP_COLOR;    
    sens_SetParameter( SENS_COLOR_BAND_TOP_LEVEL_COLOR_PARAM, sizeof ( uint32 ), (void *)&value32 );

    value32 = SENS_GRAPH_MID_COLOR;    
    sens_SetParameter( SENS_COLOR_BAND_MID_LEVEL_COLOR_PARAM, sizeof ( uint32 ), (void *)&value32 );

    value32 = SENS_GRAPH_LOW_COLOR;    
    sens_SetParameter( SENS_COLOR_BAND_LOW_LEVEL_COLOR_PARAM, sizeof ( uint32 ), (void *)&value32 );

    value32 = SENS_CALIB;    
    sens_SetParameter( SENS_CALIBIRATION_VALUE_PARAM, sizeof ( uint32 ), (void *)&value32 );
    
    value8 = SENS_TYPE;
    sens_SetParameter( SENS_TYPE_PARAM, sizeof ( uint8 ), (void *)&value8 );
    
    value8 = SENS_DISPLAY_CURRENT_VALUE;
    sens_SetParameter( SENS_TYPE_DISPLAY_CURRENT_VALUE_PARAM, sizeof ( uint8 ), (void *)&value8 );

    value8 = SENS_DISPLAY_LOG_SCALE;
    sens_SetParameter( SENS_GRAPH_LOG_SCALE_PARAM, sizeof ( uint8 ), (void *)&value8 );

    value8 = (FET_SHORT_DISABLED | SENS_OPERATIONAL_MODE);
    sens_SetParameter( SENS_MODE_OP_PARAM, sizeof ( uint8 ), (void *)&value8 );

    value8 = SENS_FEEDBACK_GAIN;
    sens_SetParameter( SENS_TIA_GAIN_PARAM, sizeof ( uint8 ), (void *)&value8 );
    
    value8 = SENS_RLOAD;
    sens_SetParameter( SENS_R_LOAD_PARAM, sizeof ( uint8 ), (void *)&value8 );
    
    value8 = SENS_INT_Z_REF_DIVIDER;
    sens_SetParameter( SENS_INT_Z_SEL_PARAM, sizeof ( uint8 ), (void *)&value8 );
    
    value8 = SENS_REF_SOURCE;
    sens_SetParameter( SENS_REF_VOLTAGE_SOURCE_PARAM, sizeof ( uint8 ), (void *)&value8 );
  }

  // Setup Battery Characteristic Values
  {
    uint8 critical = DEFAULT_BATT_CRITICAL_LEVEL;
    Batt_SetParameter( BATT_PARAM_CRITICAL_LEVEL, sizeof (uint8 ), &critical );
  }
  
  // Initialize GATT attributes
  GGS_AddService( GATT_ALL_SERVICES );         // GAP
  GATTServApp_AddService( GATT_ALL_SERVICES ); // GATT attributes
  sens_AddService( GATT_ALL_SERVICES );
  DevInfo_AddService( );
  Batt_AddService( );
  
  // Register for Sensor service callback
  sens_Register( sensCB );
  
  // Register for Battery service callback;
  Batt_Register ( sensBattCB );

#if defined( CC2540_MINIDK )
 
  // Register for all key events - This app will handle all key events
  RegisterForKeys( sensBLEPeripheral_TaskID );
  
  // makes sure LEDs are off
  HalLedSet( (HAL_LED_1 | HAL_LED_2), HAL_LED_MODE_OFF );
  
  // For keyfob board set GPIO pins into a power-optimized state
  // Note that there is still some leakage current from the buzzer,
  // accelerometer, LEDs, and buttons on the PCB.
  
  // You may want to optimize this code for the sensor demo board
  
  P0SEL = 0; // Configure Port 0 as GPIO // actually gets overrideen by the APCFG reg
  P0SEL = 1;
  P1SEL = 0; // Configure Port 1 as GPIO
  P2SEL = 0; // Configure Port 2 as GPIO

  // turn on analog test for temp sensor
  ATEST.ATEST_CTRL=0x01;

  P0DIR = 0xFC; // Port 0 pins P0.0 and P0.1 as input (buttons),
                // all others (P0.2-P0.7) as output
  P1DIR = 0xFF; // All port 1 pins (P1.0-P1.7) as output
  P2DIR = 0x1F; // All port 1 pins (P2.0-P2.4) as output
  
  P0 = 0x03; // All pins on port 0 to low except for P0.0 and P0.1 (buttons)
  P1 = 0;   // All pins on port 1 to low
  P2 = 0;   // All pins on port 2 to low  
  
#endif // #if defined( CC2540_MINIDK )
  
  // Setup a delayed profile startup
  osal_set_event( sensBLEPeripheral_TaskID, START_DEVICE_EVT );
  
  /* Init I2C portion of the HAL */
  HalI2CInit(i2cClock_33KHZ); // Was i2cClock_123KHZ, using slower rate for stability (for now)
   
  // Set the MENB pin (P1_1) as an output, and drive low
  P1DIR = P1DIR | 0x02; 
  P1 = P1&0xFD;
  
  // Also, set P1_0 (the LED) as an output, and drive low  
  P1DIR = P1DIR | 0x01; 
  P1 = P1&0xFE;
   
  // Set up the LMP91000
  get_SensHardwareSettings (&modeOp, &tiaGain, &rLoad, &refVoltageSource, &intZSel);
  LMP91000_I2CInitialSetup(tiaGain , rLoad, refVoltageSource, intZSel, (FET_SHORT_DISABLED | modeOp));
  LMP91000_I2CWriteReg(LMP91000_I2C_Address, TI_LMP91000_LOCK_REG, TI_LMP91000_WRITE_UNLOCK);   // Unlock the registers for write
 
            // The following two register writes were from the original code, just to verify that
            // we can write to and read from the part.  The final configuration is set further below
            LMP91000_I2CWriteReg(LMP91000_I2C_Address, TI_LMP91000_TIACN_REG, TIACN_NEW_VALUE);           // Modify TIA control register 
            LMP91000_I2CWriteReg(LMP91000_I2C_Address, TI_LMP91000_REFCN_REG, REFCN_NEW_VALUE);           // Modify REF control register
            LMP91000_I2CWriteAndConfirmReg(LMP91000_I2C_Address, TI_LMP91000_REFCN_REG, 
                                      (REF_SOURCE_EXTERNAL | INT_Z_SEL_50_PERCENT | BIAS_24_PERCENT | BIAS_SIGN_POSITIVE)); 

} // end sensBLEPeripheral_Init()


/*********************************************************************
 * @fn      sensBLEPeripheral_ProcessEvent
 *
 * @brief   Sensor BLE Broadcaster Application Task event processor. This
 *          function is called to process all events for the task. Events
 *          include timers, messages and any other user defined events.
 *
 * @param   task_id  - The OSAL assigned task ID.
 * @param   events - events to process.  This is a bit map and can
 *                   contain more than one event.
 *
 * @return  events not processed
 */
uint16 sensBLEPeripheral_ProcessEvent( uint8 task_id, uint16 events )
{
  
  VOID task_id; // OSAL required parameter that isn't used in this function
  
  if ( events & SYS_EVENT_MSG )
  {
    uint8 *pMsg;

    if ( (pMsg = osal_msg_receive( sensBLEPeripheral_TaskID )) != NULL )
    {
        
      sensBLEPeripheral_ProcessOSALMsg( (osal_event_hdr_t *)pMsg );

      // Release the OSAL message
      VOID osal_msg_deallocate( pMsg );
    }

    // return unprocessed events
    return (events ^ SYS_EVENT_MSG);
  }

  if ( events & START_DEVICE_EVT )
  {
    // Start the Device
    VOID GAPRole_StartDevice( &sensPeripheralCB );

    // Register with bond manager after starting device
    GAPBondMgr_Register( (gapBondCBs_t *) &sensBondCB );
    
    return ( events ^ START_DEVICE_EVT );
  }

  if ( events & SENS_PERIODIC_EVT )
  {
    // Restart timer
    if (DEFAULT_SENS_PERIOD)
    {
      osal_start_timerEx( sensBLEPeripheral_TaskID, SENS_PERIODIC_EVT, DEFAULT_SENS_PERIOD );
    }

    if (DataReadyFlag)
    {
    
      // Perform periodic O2 Sensor task
      sensPeriodicTask();
    }
    
    return (events ^ SENS_PERIODIC_EVT);
  }  

  if ( events & BATT_PERIODIC_EVT )
  {
    // Perform periodic battery task
    sensBattPeriodicTask();
    
    return (events ^ BATT_PERIODIC_EVT);
  }  
  
  // Discard unknown events
  return 0;
} // end sensBLEPeripheral_ProcessEvent()

/*********************************************************************
 * @fn      sensBLEPeripheral_ProcessOSALMsg
 *
 * @brief   Process an incoming task message.
 *
 * @param   pMsg - message to process
 *
 * @return  none
 */
static void sensBLEPeripheral_ProcessOSALMsg( osal_event_hdr_t *pMsg )
{
  switch ( pMsg->event )
  {
  
    case KEY_CHANGE:
      sens_HandleKeys( ((keyChange_t *)pMsg)->state, ((keyChange_t *)pMsg)->keys );
      break;
      
    default:
      // do nothing
      break;
  }
}


/*********************************************************************
 * @fn      heartRate_HandleKeys
 *
 * @brief   Handles all key events for this device.
 *
 * @param   shift - true if in shift/alt.
 * @param   keys - bit field for key events. Valid entries:
 *                 HAL_KEY_SW_2
 *                 HAL_KEY_SW_1
 *
 * @return  none
 */
static void sens_HandleKeys( uint8 shift, uint8 keys )
{
  if ( keys & HAL_KEY_SW_1 )
  {
    // Left Key Button - Start/Stop Data Notification (also can be started from server)
    if (sens_running)
    {
      sens_running = FALSE;
      osal_stop_timerEx( sensBLEPeripheral_TaskID, SENS_PERIODIC_EVT);
    }
    else if (gapProfileState == GAPROLE_CONNECTED)
    {
      osal_start_timerEx( sensBLEPeripheral_TaskID, SENS_PERIODIC_EVT, DEFAULT_SENS_PERIOD );
      sens_running = TRUE;
    } 
  }
  
  if ( keys & HAL_KEY_SW_2 )
  {
    // if not in a connection, toggle advertising on and off
    if( gapProfileState != GAPROLE_CONNECTED )
    {
      uint8 status;
      
      // Set fast advertising interval for user-initiated connections
      GAP_SetParamValue( TGAP_GEN_DISC_ADV_INT_MIN, DEFAULT_FAST_ADV_INTERVAL );
      GAP_SetParamValue( TGAP_GEN_DISC_ADV_INT_MAX, DEFAULT_FAST_ADV_INTERVAL );
      GAP_SetParamValue( TGAP_GEN_DISC_ADV_MIN, DEFAULT_FAST_ADV_DURATION );

      // toggle GAP advertisement status
      GAPRole_GetParameter( GAPROLE_ADVERT_ENABLED, &status );
      status = !status;
      GAPRole_SetParameter( GAPROLE_ADVERT_ENABLED, sizeof( uint8 ), &status );   
      
      // Set state variable
      if (status == FALSE)
      {
        sensAdvCancelled = TRUE;
      }
    }
  }
}

/*********************************************************************
 * @fn      heartRateMeasNotify
 *
 * @brief   Prepare and send a heart rate measurement notification
 *
 * @return  none
 */
static void sensMeasNotify(void)
{
  static uint16 counter=0;

  sensMeas.len = 8;

  sensMeas.value[0] = (counter & 0xFF);            // LSB
  sensMeas.value[1] = (counter & 0xFF00)>>8;       // MSB

  if (sens_MeasNotify( gapConnHandle, &sensMeas) == SUCCESS)
  {
    counter++;
    DataReadyFlag = 0;        
  }
}


/*********************************************************************
 * @fn      HeartRateGapStateCB
 *
 * @brief   Notification from the profile of a state change.
 *
 * @param   newState - new state
 *
 * @return  none
 */
static void sensGapStateCB( gaprole_States_t newState )
{
  // if connected
  if (newState == GAPROLE_CONNECTED)
  {
    // get connection handle
    GAPRole_GetParameter(GAPROLE_CONNHANDLE, &gapConnHandle);
  }
  // if disconnected
  else if (gapProfileState == GAPROLE_CONNECTED && 
           newState != GAPROLE_CONNECTED)
  {
    uint8 advState = TRUE;

    // stop periodic measurement
    osal_stop_timerEx( sensBLEPeripheral_TaskID, SENS_PERIODIC_EVT );
    sens_running = FALSE;

    // reset client characteristic configuration descriptors
    sens_HandleConnStatusCB( gapConnHandle, LINKDB_STATUS_UPDATE_REMOVED );
    Batt_HandleConnStatusCB( gapConnHandle, LINKDB_STATUS_UPDATE_REMOVED );

    if ( newState == GAPROLE_WAITING_AFTER_TIMEOUT )
    {
      // link loss timeout-- use fast advertising
      GAP_SetParamValue( TGAP_GEN_DISC_ADV_INT_MIN, DEFAULT_FAST_ADV_INTERVAL );
      GAP_SetParamValue( TGAP_GEN_DISC_ADV_INT_MAX, DEFAULT_FAST_ADV_INTERVAL );
      GAP_SetParamValue( TGAP_GEN_DISC_ADV_MIN, DEFAULT_FAST_ADV_DURATION );
    }
    else
    {
      // Else use slow advertising
      GAP_SetParamValue( TGAP_GEN_DISC_ADV_INT_MIN, DEFAULT_SLOW_ADV_INTERVAL );
      GAP_SetParamValue( TGAP_GEN_DISC_ADV_INT_MAX, DEFAULT_SLOW_ADV_INTERVAL );
      GAP_SetParamValue( TGAP_GEN_DISC_ADV_MIN, DEFAULT_SLOW_ADV_DURATION );
    }

    // Enable advertising
    GAPRole_SetParameter( GAPROLE_ADVERT_ENABLED, sizeof( uint8 ), &advState );    
  }    
  // if advertising stopped
  else if ( gapProfileState == GAPROLE_ADVERTISING && 
            newState == GAPROLE_WAITING )
  {
    // if advertising stopped by user
    if ( sensAdvCancelled )
    {
      sensAdvCancelled = FALSE;
    }
    // if fast advertising switch to slow
    else if ( GAP_GetParamValue( TGAP_GEN_DISC_ADV_INT_MIN ) == DEFAULT_FAST_ADV_INTERVAL )
    {
      uint8 advState = TRUE;
      
      GAP_SetParamValue( TGAP_GEN_DISC_ADV_INT_MIN, DEFAULT_SLOW_ADV_INTERVAL );
      GAP_SetParamValue( TGAP_GEN_DISC_ADV_INT_MAX, DEFAULT_SLOW_ADV_INTERVAL );
      GAP_SetParamValue( TGAP_GEN_DISC_ADV_MIN, DEFAULT_SLOW_ADV_DURATION );
      GAPRole_SetParameter( GAPROLE_ADVERT_ENABLED, sizeof( uint8 ), &advState );   
    }  
  }
  // if started
  else if (newState == GAPROLE_STARTED)
  {
    // Set the system ID from the bd addr
    uint8 systemId[DEVINFO_SYSTEM_ID_LEN];
    GAPRole_GetParameter(GAPROLE_BD_ADDR, systemId);
    
    // shift three bytes up
    systemId[7] = systemId[5];
    systemId[6] = systemId[4];
    systemId[5] = systemId[3];
    
    // set middle bytes to zero
    systemId[4] = 0;
    systemId[3] = 0;
    
    DevInfo_SetParameter(DEVINFO_SYSTEM_ID, DEVINFO_SYSTEM_ID_LEN, systemId);
  }
  
  gapProfileState = newState;
}

/*********************************************************************
 * @fn      heartRateCB
 *
 * @brief   Callback function for heart rate service.
 *
 * @param   event - service event
 *
 * @return  none
 */
static void sensCB(uint8 event)
{
  if (event == SENS_MEAS_NOTI_ENABLED)
  {
    // if connected start periodic measurement
    if (gapProfileState == GAPROLE_CONNECTED)
    {
      osal_start_timerEx( sensBLEPeripheral_TaskID, SENS_PERIODIC_EVT, DEFAULT_SENS_PERIOD );
      sens_running = TRUE;
    } 
  }
  else if (event == SENS_MEAS_NOTI_DISABLED)
  {
    // stop periodic measurement
    osal_stop_timerEx( sensBLEPeripheral_TaskID, SENS_PERIODIC_EVT );
    sens_running = FALSE;
  }
  else if (event == SENS_COMMAND_SET)
  {
    // reset energy expended
//    ecgEnergy = 0;
  }
}

/*********************************************************************
 * @fn      heartRateBattCB
 *
 * @brief   Callback function for battery service.
 *
 * @param   event - service event
 *
 * @return  none
 */
static void sensBattCB(uint8 event)
{
  if (event == BATT_LEVEL_NOTI_ENABLED)
  {
    // if connected start periodic measurement
    if (gapProfileState == GAPROLE_CONNECTED)
    {
      osal_start_timerEx( sensBLEPeripheral_TaskID, BATT_PERIODIC_EVT, DEFAULT_BATT_PERIOD );
    } 
  }
  else if (event == BATT_LEVEL_NOTI_DISABLED)
  {
    // stop periodic measurement
    osal_stop_timerEx( sensBLEPeripheral_TaskID, BATT_PERIODIC_EVT );
  }
}

/*********************************************************************
 * @fn      heartRatePeriodicTask
 *
 * @brief   Perform a periodic heart rate application task.
 *
 * @param   none
 *
 * @return  none
 */
static void sensPeriodicTask( void )
{
  if (gapProfileState == GAPROLE_CONNECTED)
  {
    // send heart rate measurement notification
    sensMeasNotify();
    
  }
}

/*********************************************************************
 * @fn      heartRateBattPeriodicTask
 *
 * @brief   Perform a periodic task for battery measurement.
 *
 * @param   none
 *
 * @return  none
 */
static void sensBattPeriodicTask( void )
{
  if (gapProfileState == GAPROLE_CONNECTED)
  {
    // perform battery level check
    Batt_MeasLevel( );
    
    // Restart timer
    osal_start_timerEx( sensBLEPeripheral_TaskID, BATT_PERIODIC_EVT, DEFAULT_BATT_PERIOD );
  }
}

/*********************************************************************
 * @fn      updateSensorData
 *
 * @brief   Load parameters into advertising data array.
 *
 * @return  none
 */
#if (ADV_DEBUG_MESSAGE_FORMAT==0)
void updateSensorData (uint16 time, uint16 tempval, uint16 sensorval)
#else
void updateSensorData (uint16 time, uint16 tempval, uint16 sensorval,uint16 CC2541tempval,uint16 vdd,uint16 spare)
#endif
{
    // Note that byte order of these fields is MSB first
  sensMeas.value[2] = (uint8)(time & 0xff);
  sensMeas.value[3] = (uint8)(time >> 8);
  sensMeas.value[4] = (uint8)(tempval & 0xff);
  sensMeas.value[5] = (uint8)(tempval >> 8);
  sensMeas.value[6] = (uint8)(sensorval & 0xff);
  sensMeas.value[7] = (uint8)(sensorval >> 8);

#if ADV_DEBUG_MESSAGE_FORMAT==1
    // Fill in additional fields
    
    // Alter low-order byte (byte 14), but not high-order byte (byte 15)
  if (lmp91kOK)
  {
    sensMeas.value[8] |= 0x40;
  }
  else
  {
    sensMeas.value[8] &= 0xBF;
  }

  // the high order byte of HW status is currently unused
  sensMeas.value[10] = (uint8)(CC2541tempval & 0xff);
  sensMeas.value[11] = (uint8)(CC2541tempval >> 8);
  sensMeas.value[12] = (uint8)(vdd & 0xff);
  sensMeas.value[13] = (uint8)(vdd >> 8);
  sensMeas.value[14] = (uint8)(spare & 0xff);
  sensMeas.value[15] = (uint8)(spare >> 8);
       
#endif    
   
} // end updateSensorData()

/*********************************************************************
*********************************************************************/
