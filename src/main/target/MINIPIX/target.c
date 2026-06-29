/*
* This file is part of INAV.
*
* INAV is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* INAV is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with INAV.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <stdint.h>

#include <platform.h>
#include "drivers/io.h"
#include "drivers/pwm_mapping.h"
#include "drivers/timer.h"
#include "drivers/sensor.h"
#include "drivers/bus.h"

// MPU6500 on SPI1, CS=PC15
BUSDEV_REGISTER_SPI_TAG(busdev_mpu6500,     DEVHW_MPU6500,      BUS_SPI1,           PC15,               NONE,       0,  DEVFLAGS_NONE,  IMU_MPU6500_ALIGN);

// LPS22HB baro on SPI2, CS=PC2
BUSDEV_REGISTER_SPI(busdev_lps22hb,         DEVHW_LPS22HB,      BUS_SPI2,           PC2,                NONE,           DEVFLAGS_NONE,      0);

// QMC5883L compass on I2C1, address 0x0D
BUSDEV_REGISTER_I2C(busdev_qmc5883,         DEVHW_QMC5883,      BUS_I2C1,           0x0D,               NONE,           DEVFLAGS_NONE,      0);

timerHardware_t timerHardware[] = {
    DEF_TIM(TIM1, CH4, PE14, TIM_USE_OUTPUT_AUTO, 0, 0),   // S1_OUT
    DEF_TIM(TIM1, CH3, PE13, TIM_USE_OUTPUT_AUTO, 0, 0),   // S2_OUT
    DEF_TIM(TIM1, CH2, PE11, TIM_USE_OUTPUT_AUTO, 0, 0),   // S3_OUT
    DEF_TIM(TIM1, CH1, PE9,  TIM_USE_OUTPUT_AUTO, 0, 0),   // S4_OUT
    DEF_TIM(TIM4, CH2, PD13, TIM_USE_OUTPUT_AUTO, 0, 0),   // S5_OUT
    DEF_TIM(TIM4, CH3, PD14, TIM_USE_OUTPUT_AUTO, 0, 0),   // S6_OUT

    // PPM inputs (for legacy receivers, disabled by default in serial RX mode)
    DEF_TIM(TIM3, CH3, PB0,  TIM_USE_PPM,         0, 0),   // PPM input
    DEF_TIM(TIM8, CH2, PC7,  TIM_USE_PPM,         0, 0),   // RC input capture
};

const int timerHardwareCount = sizeof(timerHardware) / sizeof(timerHardware[0]);
