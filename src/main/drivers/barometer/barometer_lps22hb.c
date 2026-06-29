/*
 * This file is part of INAV.
 *
 * LPS22HB barometer driver.
 *
 * INAV's bus.c already handles SPI R/W command-byte encoding:
 *   busRead()     → ORs 0x80 onto the register address
 *   busWrite()    → ANDs 0x7F onto the register address
 * So we pass PLAIN register addresses — never OR 0x40/0x80 ourselves.
 */

#include <stdbool.h>
#include <stdint.h>

#include <platform.h>
#include "build/build_config.h"
#include "common/utils.h"

#include "drivers/time.h"
#include "drivers/bus.h"
#include "drivers/barometer/barometer.h"
#include "drivers/barometer/barometer_lps22hb.h"

#if defined(USE_BARO_LPS22HB)

#define LPS22HB_WHO_AM_I            0x0F
#define LPS22HB_CTRL_REG1           0x10
#define LPS22HB_CTRL_REG2           0x11
#define LPS22HB_STATUS              0x27
#define LPS22HB_PRESS_OUT_XL        0x28
#define LPS22HB_TEMP_OUT_L          0x2B

#define LPS22HB_CHIP_ID             0xB1

// CTRL_REG2 bits
#define CTRL2_IF_ADD_INC            (1 << 4)
#define CTRL2_I2C_DIS               (1 << 3)
#define CTRL2_ONE_SHOT              (1 << 0)

// STATUS bits
#define STATUS_P_DA                 (1 << 0)
#define STATUS_T_DA                 (1 << 1)

static int32_t pressureRaw;
static int32_t temperatureRaw;

/* ── Initialisation ─────────────────────────────────────────────────── */

static bool lps22hbInit(baroDev_t *baro)
{
    // Power down (required for ONE_SHOT mode)
    busWrite(baro->busDev, LPS22HB_CTRL_REG1, 0x00);

    // Disable I2C, enable auto-increment (no FIFO, no ODR — one-shot mode)
    busWrite(baro->busDev, LPS22HB_CTRL_REG2,
             CTRL2_I2C_DIS | CTRL2_IF_ADD_INC);   // 0x18

    delay(10);
    return true;
}

/* ── INAV driver interface ──────────────────────────────────────────── */

static bool lps22hbStartUP(baroDev_t *baro)
{
    // Trigger one-shot conversion
    busWrite(baro->busDev, LPS22HB_CTRL_REG2,
             CTRL2_I2C_DIS | CTRL2_IF_ADD_INC | CTRL2_ONE_SHOT);   // 0x19
    return true;
}

static bool lps22hbGetUP(baroDev_t *baro)
{
    // Poll for pressure data-ready, then read P (3 bytes) + T (2 bytes)
    for (int i = 0; i < 30; i++) {
        delay(1);
        uint8_t s;
        busRead(baro->busDev, LPS22HB_STATUS, &s);          // bus.c sends 0xA7

        if (s & STATUS_P_DA) {
            uint8_t buf[5];
            busReadBuf(baro->busDev, LPS22HB_PRESS_OUT_XL,  // bus.c sends 0xA8
                       buf, 5);                              // IF_ADD_INC → reads 0x28..0x2C
            pressureRaw    = (int32_t)((uint32_t)buf[2] << 16
                                     | (uint32_t)buf[1] << 8
                                     | (uint32_t)buf[0]);
            temperatureRaw = (int16_t)((uint16_t)buf[4] << 8
                                     | (uint16_t)buf[3]);
            return true;
        }
    }
    return false;   // conversion timeout
}

static bool lps22hbStartUT(baroDev_t *baro)
{
    UNUSED(baro);
    // No-op: start_up() already triggers ONE_SHOT (produces both P+T).
    return true;
}

static bool lps22hbGetUT(baroDev_t *baro)
{
    UNUSED(baro);
    // No-op: get_up() already read both P+T together.
    return true;
}

STATIC_UNIT_TESTED bool lps22h_calculate(baroDev_t *baro,
                                          int32_t *pressure,
                                          int32_t *temperature)
{
    UNUSED(baro);
    if (pressure)    *pressure    = pressureRaw * 100 / 4096;
    if (temperature) *temperature = temperatureRaw;
    return true;
}

/* ── Detection ──────────────────────────────────────────────────────── */

static bool deviceDetect(busDevice_t *busDev)
{
    for (int i = 0; i < 5; i++) {
        delay(30);
        uint8_t id;
        if (busRead(busDev, LPS22HB_WHO_AM_I, &id)          // bus.c sends 0x8F
            && id == LPS22HB_CHIP_ID)
            return true;
    }
    return false;
}

bool lps22hbDetect(baroDev_t *baro)
{
    baro->busDev = busDeviceInit(BUSTYPE_ANY, DEVHW_LPS22HB, 0, OWNER_BARO);
    if (!baro->busDev) return false;

    busSetSpeed(baro->busDev, BUS_SPEED_SLOW);

    if (!deviceDetect(baro->busDev)) {
        busDeviceDeInit(baro->busDev);
        return false;
    }

    lps22hbInit(baro);

    baro->ut_delay    = 0;
    baro->up_delay    = 12000;
    baro->start_up    = lps22hbStartUP;
    baro->get_up      = lps22hbGetUP;
    baro->start_ut    = lps22hbStartUT;
    baro->get_ut      = lps22hbGetUT;
    baro->calculate   = lps22h_calculate;

    return true;
}

#endif
