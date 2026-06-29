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

#pragma once

#define TARGET_BOARD_IDENTIFIER "MIPX"

#define USBD_PRODUCT_STRING     "Mini-Pix"

// Use target-specific hardware descriptors
#define USE_TARGET_HARDWARE_DESCRIPTORS

// Hardware pre-boot setup for sensor power
#define USE_HARDWARE_PREBOOT_SETUP

// LEDs: RGB LED as individual GPIOs
#define LED0                    PB1     // Red
#define LED1                    PC5     // Green
#define LED2                    PE12    // Blue

// Beeper
#define BEEPER                  PA15
#define BEEPER_INVERTED

// SBUS inverter control
#define INVERTER_PIN_UART       PC13

// IMU: MPU6500 on SPI1
#define USE_IMU_MPU6500
#define IMU_MPU6500_ALIGN       CW180_DEG

// Barometer: LPS22HB on SPI2
#define USE_BARO
#define USE_BARO_LPS22HB
#define LPS22HB_SPI_BUS         BUS_SPI2
#define LPS22HB_CS_PIN          PC2

// Magnetometer: QMC5883L on I2C1
#define USE_MAG
#define MAG_I2C_BUS             BUS_I2C1
#define USE_MAG_ALL

#define TEMPERATURE_I2C_BUS     BUS_I2C1

// SD Card via SDIO
#define USE_SDCARD
#define USE_SDCARD_SDIO
#define SDCARD_SDIO_DMA         DMA_TAG(2,3,4)
#define SDCARD_SDIO_4BIT
#define ENABLE_BLACKBOX_LOGGING_ON_SDCARD_BY_DEFAULT

// USB
#define USE_VCP
#define VBUS_SENSING_PIN PA9
#define VBUS_SENSING_ENABLED

// UARTs
#define USE_UART2                       // TELEM1 (USART3)
#define UART2_RX_PIN            PD9
#define UART2_TX_PIN            PD8

#define USE_UART3                       // TELEM2 (USART2)
#define UART3_RX_PIN            PD6
#define UART3_TX_PIN            PD5

#define USE_UART4                       // GPS
#define UART4_RX_PIN            PA1
#define UART4_TX_PIN            PA0

#define USE_UART6                       // SerialRX
#define UART6_RX_PIN            PC7
#define UART6_TX_PIN            PC6

#define SERIAL_PORT_COUNT       7       // VCP + UART2/3/4/6

// SPI
#define USE_SPI

#define USE_SPI_DEVICE_1        // MPU6500 IMU
#define SPI1_NSS_PIN            PC15
#define SPI1_SCK_PIN            PA5
#define SPI1_MISO_PIN           PA6
#define SPI1_MOSI_PIN           PA7

#define USE_SPI_DEVICE_2        // LPS22HB Baro / FRAM
#define SPI2_SCK_PIN            PB13
#define SPI2_MISO_PIN           PB14
#define SPI2_MOSI_PIN           PB15

// I2C
#define USE_I2C
#define USE_I2C_DEVICE_1
#define I2C1_SCL                PB8
#define I2C1_SDA                PB9

// ADC
#define BOARD_HAS_VOLTAGE_DIVIDER
#define USE_ADC
#define ADC_CHANNEL_1_PIN               PA2
#define ADC_CHANNEL_2_PIN               PA3
#define ADC_CHANNEL_3_PIN               PC1
#define VBAT_ADC_CHANNEL                ADC_CHN_1
#define CURRENT_METER_ADC_CHANNEL       ADC_CHN_2
#define RSSI_ADC_CHANNEL                ADC_CHN_3

// Serial RX
#define SERIALRX_PROVIDER       SERIALRX_SBUS
#define SERIALRX_UART           SERIAL_PORT_USART6
#define DEFAULT_RX_TYPE         RX_TYPE_SERIAL

// Features
#define DEFAULT_FEATURES        (FEATURE_TX_PROF_SEL | FEATURE_BLACKBOX)

#define USE_SERIAL_4WAY_BLHELI_INTERFACE

// Number of available PWM outputs
#define MAX_PWM_OUTPUT_PORTS    6

// Config storage in internal flash
#define USE_FLASHFS
#define CONFIG_START_FLASH_ADDRESS (0x08080000)

// IO port bitmasks
#define TARGET_IO_PORTA         0xffff
#define TARGET_IO_PORTB         0xffff
#define TARGET_IO_PORTC         0xffff
#define TARGET_IO_PORTD         0xffff
#define TARGET_IO_PORTE         0xffff
