/*
 * @file bsp_dac.h
 *
 * @author: Thomas Carli
 */

#ifndef SRC_BSP_BSP_CCU4_H_
#define SRC_BSP_BSP_CCU4_H_

#include <bsp_int.h>
#include <xmc_gpio.h>
#include <xmc_ccu4.h>
#include <xmc_scu.h>
#include <stdio.h>

#include "xmc4500_spi_lib.h"
#include "mcp3004_drv.h"
#include "mcp23s08_drv.h"

#define SLICE_PTR_CCU40_CC40       CCU40_CC40  // braucht modul-number 2U für Modul(CCU42) und  slice-number 2U für CCU42_CCU42
#define MODULE_PTR_CCU40           CCU40
#define SLICE_NUMBER_CCU40_CC40    (0U)  // weil CCU42_CC42

#define D5 P1_15
#define D6 P1_13
#define D7 P1_14
#define D8 P1_12

// int end1, end2, end3, end4;

_Bool BSP_ConfigCCU4_Timer (void);
_Bool BSP_PWM_SetDutyCycle(int led, uint16_t dutycycle);

#endif

/*! EOF */
