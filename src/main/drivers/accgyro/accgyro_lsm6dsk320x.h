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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See tLSM6DXXhe
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with INAV.  If not, see <http://www.gnu.org/licenses/>.
 * moved from atbetaflight https://github.com/flightng/atbetaflight by tcdddd
 */

#pragma once

#include "drivers/bus.h"

// typedef enum {
//     GYRO_HARDWARE_LPF_NORMAL,
//     GYRO_HARDWARE_LPF_OPTION_1,
//     GYRO_HARDWARE_LPF_OPTION_2,
//     GYRO_HARDWARE_LPF_EXPERIMENTAL,
//     GYRO_HARDWARE_LPF_COUNT
// } gyroHardwareLpf_e;

bool lsm6dsk320xAccDetect(accDev_t *acc);
bool lsm6dsk320xGyroDetect(gyroDev_t *gyro);
