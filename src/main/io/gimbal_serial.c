/*
 * This file is part of INAV.
 *
 * Cleanflight is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Cleanflight is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with INAV.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdio.h>
#include <stdint.h>
#include <unistd.h>

#include "platform.h"

#ifdef USE_SERIAL_GIMBAL

#include <common/crc.h>
#include <common/utils.h>
#include <common/maths.h>
#include <build/debug.h>

#include <drivers/gimbal_common.h>
#include <drivers/serial.h>
#include <drivers/time.h>

#include <io/gimbal_serial.h>
#include <io/serial.h>

#include <rx/rx.h>
#include <fc/rc_modes.h>

#include <config/parameter_group_ids.h>

PG_REGISTER(gimbalSerialConfig_t, gimbalSerialConfig, PG_GIMBAL_SERIAL_CONFIG, 0);

STATIC_ASSERT(sizeof(gimbalHtkAttitudePkt_t) == 10, gimbalHtkAttitudePkt_t_size_not_10);

#define GIMBAL_SERIAL_BUFFER_SIZE 512

#ifndef GIMBAL_UNIT_TEST
static volatile uint8_t txBuffer[GIMBAL_SERIAL_BUFFER_SIZE];

static gimbalSerialHtrkState_t headTrackerState = { 
    .lastUpdate = 0,
    .payloadSize = 0,
    .state = WAITING_HDR1,
};
#endif

static serialPort_t *headTrackerPort = NULL;
static serialPort_t *gimbalPort = NULL;

gimbalVTable_t gimbalSerialVTable = {
    .process = gimbalSerialProcess,
    .getDeviceType = gimbalSerialGetDeviceType,
    .isReady = gimbalSerialIsReady,
    .hasHeadTracker = gimbalSerialHasHeadTracker,

};

static gimbalDevice_t serialGimbalDevice = {
    .vTable = &gimbalSerialVTable
};

gimbalDevType_e gimbalSerialGetDeviceType(const gimbalDevice_t *gimbalDevice)
{
    UNUSED(gimbalDevice);
    return GIMBAL_DEV_SERIAL;
}

bool gimbalSerialIsReady(const gimbalDevice_t *gimbalDevice)
{
    return gimbalPort != NULL && gimbalDevice->vTable != NULL;
}

bool gimbalSerialHasHeadTracker(const gimbalDevice_t *gimbalDevice)
{
    UNUSED(gimbalDevice);
    return headTrackerPort;
}

bool gimbalSerialInit(void)
{
    if(gimbalSerialDetect()) {
        gimbalCommonSetDevice(&serialGimbalDevice);
        return true;
    }

    return false;
}

#ifdef GIMBAL_UNIT_TEST
bool gimbalSerialDetect(void)
{
    return false;
}
#else
bool gimbalSerialDetect(void)
{
    SD(fprintf(stderr, "[GIMBAL]: serial Detect...\n"));
    serialPortConfig_t *portConfig = findSerialPortConfig(FUNCTION_GIMBAL);
    bool singleUart = gimbalSerialConfig()->singleUart;

    if (portConfig) {
        SD(fprintf(stderr, "[GIMBAL]: found port...\n"));
        gimbalPort = openSerialPort(portConfig->identifier, FUNCTION_GIMBAL, singleUart ? gimbalSerialHeadTrackerReceive : NULL, singleUart ? &headTrackerState :  NULL,
                baudRates[portConfig->peripheral_baudrateIndex], MODE_RXTX, SERIAL_NOT_INVERTED);

        if (gimbalPort) {
            SD(fprintf(stderr, "[GIMBAL]: port open!\n"));
            gimbalPort->txBuffer = txBuffer;
            gimbalPort->txBufferSize = GIMBAL_SERIAL_BUFFER_SIZE;
            gimbalPort->txBufferTail = 0;
            gimbalPort->txBufferHead = 0;
        } else {
            SD(fprintf(stderr, "[GIMBAL]: port NOT open!\n"));
            return false;
        }
    }

    SD(fprintf(stderr, "[GIMBAL_HTRK]: headtracker Detect...\n"));
    portConfig = singleUart ? NULL : findSerialPortConfig(FUNCTION_GIMBAL_HEADTRACKER);

    if (portConfig) {
        SD(fprintf(stderr, "[GIMBAL_HTRK]: found port...\n"));
        headTrackerPort = openSerialPort(portConfig->identifier, FUNCTION_GIMBAL_HEADTRACKER, gimbalSerialHeadTrackerReceive, &headTrackerState,
                baudRates[portConfig->peripheral_baudrateIndex], MODE_RXTX, SERIAL_NOT_INVERTED);

        if (headTrackerPort) {
            SD(fprintf(stderr, "[GIMBAL_HTRK]: port open!\n"));
            headTrackerPort->txBuffer = txBuffer;
            headTrackerPort->txBufferSize = GIMBAL_SERIAL_BUFFER_SIZE;
            headTrackerPort->txBufferTail = 0;
            headTrackerPort->txBufferHead = 0;
        } else {
            SD(fprintf(stderr, "[GIMBAL_HTRK]: port NOT open!\n"));
            return false;
        }
    }

    return gimbalPort || headTrackerPort;
}
#endif

#ifdef GIMBAL_UNIT_TEST
void gimbalSerialProcess(gimbalDevice_t *gimbalDevice, timeUs_t currentTime)
{
    UNUSED(gimbalDevice);
    UNUSED(currentTime);
}
#else
void gimbalSerialProcess(gimbalDevice_t *gimbalDevice, timeUs_t currentTime)
{
    UNUSED(currentTime);

    if (!gimbalSerialIsReady(gimbalDevice)) {
        SD(fprintf(stderr, "[GIMBAL] gimbal not ready...\n"));
        return;
    }

    gimbalHtkAttitudePkt_t attittude = {
        .sync = {HTKATTITUDE_SYNC0, HTKATTITUDE_SYNC1},
        .mode = GIMBAL_MODE_DEFAULT
    };

    const gimbalConfig_t *cfg = gimbalConfig();

    int pan = 1500;
    int tilt = 1500;
    int roll = 1500;

    if (IS_RC_MODE_ACTIVE(BOXGIMBALTLOCK)) {
        attittude.mode |= GIMBAL_MODE_TILT_LOCK;
    }

    if (IS_RC_MODE_ACTIVE(BOXGIMBALRLOCK)) {
        attittude.mode |= GIMBAL_MODE_ROLL_LOCK;
    }

    // Follow center overrides all
    if (IS_RC_MODE_ACTIVE(BOXGIMBALCENTER) || IS_RC_MODE_ACTIVE(BOXGIMBALHTRK)) {
        attittude.mode = GIMBAL_MODE_FOLLOW;
    }
    
    if (rxAreFlightChannelsValid() && !IS_RC_MODE_ACTIVE(BOXGIMBALCENTER)) {
        if (cfg->panChannel > 0) {
            pan = rxGetChannelValue(cfg->panChannel - 1);
            pan = constrain(pan, 1000, 2000);
        }

        if (cfg->tiltChannel > 0) {
            tilt = rxGetChannelValue(cfg->tiltChannel - 1);
            tilt = constrain(tilt, 1000, 2000);
        }

        if (cfg->rollChannel > 0) {
            roll = rxGetChannelValue(cfg->rollChannel - 1);
            roll = constrain(roll, 1000, 2000);
        }
    }

    if(IS_RC_MODE_ACTIVE(BOXGIMBALHTRK)) {
        if (gimbalCommonHtrkIsEnabled() && (micros() - headTrackerState.lastUpdate) < MAX_INVALID_RX_PULSE_TIME) {
            tilt = headTrackerState.tilt;
            pan = headTrackerState.pan;
            roll = headTrackerState.roll;
        } else {
            tilt = 0;
            pan = 0;
            roll = 0;
        }
    }

    attittude.sensibility = cfg->sensitivity; //gimbal_scale5(-16, 15, -16, 15, cfg->sensitivity);

    // Radio endpoints may need to be adjusted, as it seems ot go a bit bananas
    // at the extremes
    attittude.pan = gimbal_scale12(1000, 2000, pan);
    attittude.tilt = gimbal_scale12(1000, 2000, tilt);
    attittude.roll = gimbal_scale12(1000, 2000, roll);

    uint16_t crc16 = 0;
    uint8_t *b = (uint8_t *)&attittude;
    for (uint8_t i = 0; i < sizeof(gimbalHtkAttitudePkt_t) - 2; i++) {
        crc16 = crc16_ccitt(crc16, *(b + i));
    }
    attittude.crch = (crc16 >> 8) & 0xFF;
    attittude.crcl = crc16 & 0xFF;

    serialBeginWrite(gimbalPort);
    serialWriteBuf(gimbalPort, (uint8_t *)&attittude, sizeof(gimbalHtkAttitudePkt_t));
    serialEndWrite(gimbalPort);
}
#endif

int16_t gimbal_scale12(int16_t inputMin, int16_t inputMax, int16_t value)
{
    int16_t ret = 0;
    ret = scaleRange(value, inputMin, inputMax, -2048, 2047);
    return ret;
}

#ifndef GIMBAL_UNIT_TEST
static void resetState(gimbalSerialHtrkState_t *state)
{
    state->state = WAITING_HDR1;
    state->payloadSize = 0;
}

static bool checkCrc(gimbalHtkAttitudePkt_t *attitude)
{
    uint8_t *attitudePkt = (uint8_t *)attitude;
    uint16_t crc = 0;

    for(uint8_t i = 0; i < sizeof(gimbalHtkAttitudePkt_t) - 2; ++i) {
        crc = crc16_ccitt(crc, attitudePkt[i]);
    }

    return (attitude->crch == ((crc >> 8) & 0xFF)) &&
           (attitude->crcl == (crc & 0xFF));
}


void gimbalSerialHeadTrackerReceive(uint16_t c, void *data)
{
    gimbalSerialHtrkState_t *state = (gimbalSerialHtrkState_t *)data;
    uint8_t *payload = (uint8_t *)&(state->attittude);
    payload += 2;

    switch(state->state) {
        case WAITING_HDR1:
            if(c == HTKATTITUDE_SYNC0) {
                state->attittude.sync[0] = c;
                state->state = WAITING_HDR2;
            }
            break;
        case WAITING_HDR2:
            if(c == HTKATTITUDE_SYNC1) {
                state->attittude.sync[1] = c;
                state->state = WAITING_PAYLOAD;
            } else {
                resetState(state);
            }
            break;
        case WAITING_PAYLOAD:
            payload[state->payloadSize++] = c;
            if(state->payloadSize == HEADTRACKER_PAYLOAD_SIZE)
            {
                state->state = WAITING_CRCH;
            }
            break;
        case WAITING_CRCH:
            state->attittude.crch = c;
            break;
        case WAITING_CRCL:
            state->attittude.crcl = c;
            if(checkCrc(&(state->attittude))) {
                state->lastUpdate = micros();
                state->pan = state->attittude.pan;
                state->tilt = state->attittude.tilt;
                state->roll = state->attittude.roll;
            }
            resetState(state);
            break;
    }
}
#endif

#endif