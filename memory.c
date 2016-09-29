/*
    MICROCHIP SOFTWARE NOTICE AND DISCLAIMER:

    You may use this software, and any derivatives created by any person or
    entity by or on your behalf, exclusively with Microchip's products.
    Microchip and its subsidiaries ("Microchip"), and its licensors, retain all
    ownership and intellectual property rights in the accompanying software and
    in all derivatives hereto.

    This software and any accompanying information is for suggestion only. It
    does not modify Microchip's standard warranty for its products.  You agree
    that you are solely responsible for testing the software and determining
    its suitability.  Microchip has no obligation to modify, test, certify, or
    support the software.

    THIS SOFTWARE IS SUPPLIED BY MICROCHIP "AS IS".  NO WARRANTIES, WHETHER
    EXPRESS, IMPLIED OR STATUTORY, INCLUDING, BUT NOT LIMITED TO, IMPLIED
    WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY, AND FITNESS FOR A
    PARTICULAR PURPOSE APPLY TO THIS SOFTWARE, ITS INTERACTION WITH MICROCHIP'S
    PRODUCTS, COMBINATION WITH ANY OTHER PRODUCTS, OR USE IN ANY APPLICATION.

    IN NO EVENT, WILL MICROCHIP BE LIABLE, WHETHER IN CONTRACT, WARRANTY, TORT
    (INCLUDING NEGLIGENCE OR BREACH OF STATUTORY DUTY), STRICT LIABILITY,
    INDEMNITY, CONTRIBUTION, OR OTHERWISE, FOR ANY INDIRECT, SPECIAL, PUNITIVE,
    EXEMPLARY, INCIDENTAL OR CONSEQUENTIAL LOSS, DAMAGE, FOR COST OR EXPENSE OF
    ANY KIND WHATSOEVER RELATED TO THE SOFTWARE, HOWSOEVER CAUSED, EVEN IF
    MICROCHIP HAS BEEN ADVISED OF THE POSSIBILITY OR THE DAMAGES ARE
    FORESEEABLE.  TO THE FULLEST EXTENT ALLOWABLE BY LAW, MICROCHIP'S TOTAL
    LIABILITY ON ALL CLAIMS IN ANY WAY RELATED TO THIS SOFTWARE WILL NOT EXCEED
    THE AMOUNT OF FEES, IF ANY, THAT YOU HAVE PAID DIRECTLY TO MICROCHIP FOR
    THIS SOFTWARE.

    MICROCHIP PROVIDES THIS SOFTWARE CONDITIONALLY UPON YOUR ACCEPTANCE OF
    THESE TERMS.
*/

#include <stdint.h>

#include "configuration.h"
#include "mtouch.h"

#if defined(UART_ENABLED) || defined(I2C_ENABLED)

/* Extern'd Global Variables */
extern uint8_t              MTOUCH_delta_scaling    [MTOUCH_SENSORS];
extern MTOUCH_DELTA_t       MTOUCH_button_threshold [MTOUCH_SENSORS];
extern MTOUCH_FLAGS_t       MTOUCH_flags;
extern MTOUCH_SAMPLE_t      MTOUCH_lastSample       [MTOUCH_SENSORS];
extern MTOUCH_DELTA_t       MTOUCH_proximity_threshold;
extern uint8_t              MTOUCH_proximity_scaling;  
extern uint8_t              MTOUCH_baselineGain;
extern uint8_t              MTOUCH_baselineRate;
extern uint8_t              MTOUCH_negDeltaCount;
extern uint8_t              MTOUCH_pressTimeout;
extern uint8_t              MTOUCH_debounce;
extern uint8_t              MTOUCH_readingGain;

/* Local Variables */
uint8_t bank = 0;

/* Local Functions */
uint8_t GetState        (void);
uint8_t GetInterruptBit (void);
void    CalibrateSensors(uint8_t mask);

uint8_t MEMORY_Read(uint8_t addr)
{
    uint8_t output = 0;
    if (bank == 0)
    {
        switch(addr)
        {
            /* Sensor Deltas */
            //case 0x10:  output = MTOUCH_Sensor_Delta_Get(0);    break;
            case 0x10:  output = MTOUCH_Proximity_Delta_Get();    break;
            case 0x11:  output = MTOUCH_Sensor_Delta_Get(1);    break;
            case 0x12:  output = MTOUCH_Sensor_Delta_Get(2);    break;
            case 0x13:  output = MTOUCH_Sensor_Delta_Get(3);    break;
            case 0x14:  output = MTOUCH_Sensor_Delta_Get(4);    break;
            case 0x15:  output = MTOUCH_Sensor_Delta_Get(5);    break;
            #if (MTOUCH_SENSORS != 6)
            #warning("Memory mapping should be updated to reflect the number of sensors.")
            #endif

            /* Sensor Thresholds */
            //case 0x40:  output = MTOUCH_button_threshold[0];    break;
            case 0x40:  output = MTOUCH_proximity_threshold;    break;
            case 0x41:  output = MTOUCH_button_threshold[1];    break;
            case 0x42:  output = MTOUCH_button_threshold[2];    break;
            case 0x43:  output = MTOUCH_button_threshold[3];    break;
            case 0x44:  output = MTOUCH_button_threshold[4];    break;
            case 0x45:  output = MTOUCH_button_threshold[5];    break;
            #if (MTOUCH_SENSORS != 6)
            #warning("Memory mapping should be updated to reflect the number of sensors.")
            #endif

            /* Sensor Delta Scaling */
            //case 0x80:  output = MTOUCH_delta_scaling[0];       break;
            case 0x80:  output = MTOUCH_proximity_scaling;      break;
            case 0x81:  output = MTOUCH_delta_scaling[1];       break;
            case 0x82:  output = MTOUCH_delta_scaling[2];       break;
            case 0x83:  output = MTOUCH_delta_scaling[3];       break;
            case 0x84:  output = MTOUCH_delta_scaling[4];       break;
            case 0x85:  output = MTOUCH_delta_scaling[5];       break;
            #if (MTOUCH_SENSORS != 6)
            #warning("Memory mapping should be updated to reflect the number of sensors.")
            #endif

            case 0xA0:  output = MTOUCH_SAMPLES;                break;
            
            case 0xB0:  output = MTOUCH_readingGain;            break;
            case 0xB1:  output = MTOUCH_baselineGain;           break;
            case 0xB2:  output = MTOUCH_baselineRate;           break;
            case 0xC0:  output = MTOUCH_negDeltaCount;          break;
            case 0xC1:  output = MTOUCH_pressTimeout;           break;
            case 0xC2:  output = MTOUCH_debounce;               break;
            default: break;
        }
    }
    else if (bank == 1)
    {
        switch(addr)
        {
        #define READ_READING(addr,sensor)                                                        \
            case (addr)  : output = (uint8_t)(MTOUCH_Sensor_Reading_Get((sensor)));       break; \
            case (addr+1): output = (uint8_t)(MTOUCH_Sensor_Reading_Get((sensor)) >> 8);  break; \
            case (addr+2): output = (uint8_t)(MTOUCH_Sensor_Reading_Get((sensor)) >> 16); break; 

            READ_READING(0x10, 0);
            READ_READING(0x13, 1);
            READ_READING(0x16, 2);
            READ_READING(0x19, 3);
            READ_READING(0x1C, 4);
            READ_READING(0x1F, 5);

            default: break;
        }
    }
    else if (bank == 2)
    {
        switch(addr)
        {
        #define READ_BASELINE(addr,sensor)                                                        \
            case (addr)  : output = (uint8_t)(MTOUCH_Sensor_Baseline_Get((sensor)));       break; \
            case (addr+1): output = (uint8_t)(MTOUCH_Sensor_Baseline_Get((sensor)) >> 8);  break; \
            case (addr+2): output = (uint8_t)(MTOUCH_Sensor_Baseline_Get((sensor)) >> 16); break; 
            
            READ_BASELINE(0x10, 0);
            READ_BASELINE(0x13, 1);
            READ_BASELINE(0x16, 2);
            READ_BASELINE(0x19, 3);
            READ_BASELINE(0x1C, 4);
            READ_BASELINE(0x1F, 5);

            default: break;
        }
    }
    else if (bank == 3)
    {
        switch(addr)
        {
        #define READ_SAMPLE(addr,sensor)                                                 \
            case (addr)  : output = (uint8_t)(MTOUCH_lastSample[(sensor)]);       break; \
            case (addr+1): output = (uint8_t)(MTOUCH_lastSample[(sensor)] >> 8);  break; \
            case (addr+2): output = (uint8_t)(MTOUCH_lastSample[(sensor)] >> 16); break;

            READ_SAMPLE(0x10, 0);
            READ_SAMPLE(0x13, 1);
            READ_SAMPLE(0x16, 2);
            READ_SAMPLE(0x19, 3);
            READ_SAMPLE(0x1C, 4);
            READ_SAMPLE(0x1F, 5);

            default: break;
        }
    }

    /* Common registers, regardless of bank */
    switch(addr)
    {
        case 0x00:  output += GetInterruptBit();            break;

        case 0x01:  output = GetState();                    break;

        /* Reserved memory space */
        case 0xF0:  output = bank;                          break;
        case 0xF1:  output = MTOUCH_SENSORS;                break;
        case 0xF7:  output = ID_PRODUCT & 0xFF;             break;
        case 0xF8:  output = (ID_PRODUCT>>8) & 0xFF;        break;
        case 0xF9:  output = (ID_PRODUCT>>16)& 0xFF;        break;
        case 0xFA:  output = (ID_PRODUCT>>24)& 0xFF;        break;
        case 0xFB:  output = ID_MANUFACTURER;               break;
        case 0xFC:  output = ID_MANUFACTURER>>8 & 0xFF;     break;
        case 0xFD:  output = ID_BUILD;                      break;
        case 0xFE:  output = ID_MINOR;                      break;
        case 0xFF:  output = ID_MAJOR;                      break;

        default: break;
    }

    return output;
}

void MEMORY_Write(uint8_t addr, uint8_t value)
{
    switch(addr)
    {
        case 0x00:  CalibrateSensors(0xFF);                 break;

        /* Sensor Thresholds */
        case 0x40:  if ((int8_t)value < 0) { value = 0; }
                    MTOUCH_button_threshold[0] = value;     break;
        case 0x41:  if ((int8_t)value < 0) { value = 0; }
                    MTOUCH_button_threshold[1] = value;     break;
        case 0x42:  if ((int8_t)value < 0) { value = 0; }
                    MTOUCH_button_threshold[2] = value;     break;
        case 0x43:  if ((int8_t)value < 0) { value = 0; }
                    MTOUCH_button_threshold[3] = value;     break;
        case 0x44:  if ((int8_t)value < 0) { value = 0; }
                    MTOUCH_button_threshold[4] = value;     break;
        case 0x45:  if ((int8_t)value < 0) { value = 0; }
                    MTOUCH_button_threshold[5] = value;     break;

        case 0x70:  CalibrateSensors(value);                break;

        /* Sensor Delta Scaling */
        case 0x80:  MTOUCH_delta_scaling[0] = value;        break;
        case 0x81:  MTOUCH_delta_scaling[1] = value;        break;
        case 0x82:  MTOUCH_delta_scaling[2] = value;        break;
        case 0x83:  MTOUCH_delta_scaling[3] = value;        break;
        case 0x84:  MTOUCH_delta_scaling[4] = value;        break;
        case 0x85:  MTOUCH_delta_scaling[5] = value;        break;
        #if (MTOUCH_SENSORS != 6)
        #warning("Memory mapping should be updated to reflect the number of sensors.")
        #endif

        case 0xB0:  MTOUCH_readingGain  = value;           break;
        case 0xB1:  MTOUCH_baselineGain = value;           break;
        case 0xB2:  MTOUCH_baselineRate = value;           break;
        case 0xC0:  MTOUCH_negDeltaCount= value;           break;
        case 0xC1:  MTOUCH_pressTimeout = value;           break;
        case 0xC2:  MTOUCH_debounce     = value;           break;
        
        /* Reserved memory space */
        case 0xF0:  bank = value;   break;

        default:                    break;
    }
}

uint8_t GetState(void)
{
    uint8_t output = 0;

    for (uint8_t i = 0; i < MTOUCH_SENSORS; i++)
    {
        if (MTOUCH_Button_State_Get(i) == MTOUCH_STATE_pressed)
        {
            output |= 0x01 << i;
        }
    }
    #if (MTOUCH_SENSORS != 6)
    #error("State mask creation must be updated to reflect the number of sensors.")
    #endif

    return output;
}

uint8_t GetInterruptBit(void)
{
    uint8_t i;

    for (i = 0; i < MTOUCH_SENSORS; i++)
    {
        if (MTOUCH_Button_State_Get(i) == MTOUCH_STATE_pressed)
        {
            return 1;
        }
    }
    return 0;
}

void CalibrateSensors(uint8_t mask)
{
    uint8_t i;
    for (i = 0; i < MTOUCH_SENSORS; i++)
    {
        if ((mask & (0x01 << i)) != 0)
        {
            MTOUCH_Scan_ResetCalibration(i);
        }
    }
    #if (MTOUCH_SENSORS != 6)
    #error("Recalibration command must be updated to reflect the number of sensors.")
    #endif
}
#endif
