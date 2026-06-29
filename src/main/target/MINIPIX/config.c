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

#include "platform.h"

#include "config/feature.h"
#include "fc/config.h"
#include "sensors/compass.h"
#include "drivers/pwm_mapping.h"

void targetConfiguration(void)
{
    // Compass alignment: ROTATION_PITCH_180_YAW_270 in ArduPilot terms
    compassConfigMutable()->mag_align = CW90_DEG;

    // TIM4 outputs as motors for backward compatibility
    timerOverridesMutable(timer2id(TIM4))->outputMode = OUTPUT_MODE_MOTORS;
}
