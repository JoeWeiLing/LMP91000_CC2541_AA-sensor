#ifndef __CO_SENSOR_SETTINGS_H
#define __CO_SENSOR_SETTINGS_H

#include "TI_LMP91000.h"

// Initial values for the algorithm used to convert ADC output into meaningful value for the CO Sensor
#define SENS_DENOM_0                            204800        // 2048 x 100  -- ADC resolution
#define SENS_NUMER_0                            250           // 2.5V x 100    

// 0.50V x 100 is theoretical, however, if we are seeing anything less than exactly 2.5V, the values in room 
//  air, that are VERY close to 0 may become negative. We will slightly bias this slightly to assure positive numbers
#define SENS_SUB_0                              50            
#define SENS_DENOM_1                            700000        // 7000 x 100
#define SENS_SCALE_FACTOR_NUM                   -1000000000   // Convert into ppm from nA
#define SENS_SCALE_FACTOR_DENOM                 70            // 70nA per ppm

// LMP91000 Settings for the CO Sensor
#define SENS_OPERATIONAL_MODE                   OP_MODE_3_LEAD
#define SENS_FEEDBACK_GAIN                      TIA_GAIN_350K_OHM
#define SENS_RLOAD                              R_LOAD_10_OHM
#define SENS_INT_Z_REF_DIVIDER                  INT_Z_SEL_50_PERCENT
#define SENS_REF_SOURCE                         REF_SOURCE_EXTERNAL


// Parameters used by iPhone/iPad app for graphing data
#define SENS_Y_DISPLAY_MAX                      2000
#define SENS_Y_DISPLAY_MIN                      410
#define SENS_GRAPH_TOP_MID_BOUNDARY             500    
#define SENS_GRAPH_MID_LOW_BOUNDARY             445   
#define SENS_GRAPH_TOP_COLOR                    0xff0a00      // Red
#define SENS_GRAPH_MID_COLOR                    0xfffa00      // Yellow
#define SENS_GRAPH_LOW_COLOR                    0x0aff00      // Green
#define SENS_CALIB                              0xffffffff    // No Calibration
#define SENS_TYPE                               CO_SENSOR_TYPE    
#define SENS_DISPLAY_CURRENT_VALUE              1             // Display current value
#define SENS_DISPLAY_LOG_SCALE                  1             // Do use logarithmic scale


// Note that 19 character maximum for Characteristic strings, that is 18 characters + \0!!!
#define SENS_GRAPH_TITLE                        "Carbon Monoxide\0"
#define SENS_GRAPH_TITLE_SIZE                   16
#define SENS_GRAPH_SUBTITLE                     "Using the LMP91000\0"
#define SENS_GRAPH_SUBTITLE_SIZE                19
#define SENS_GRAPH_X_AXIS_CAPTION               "Time in seconds\0"
#define SENS_GRAPH_X_AXIS_CAPTION_SIZE          16
#define SENS_GRAPH_Y_AXIS_CAPTION               "CO (ppm)\0"    
#define SENS_GRAPH_Y_AXIS_CAPTION_SIZE          9
#define SENS_SHORT_CAPTION_VALUE                "ppm CO\0"
#define SENS_SHORT_CAPTION_SIZE                 7

#endif // __CO_SENSOR_SETTINGS_H
