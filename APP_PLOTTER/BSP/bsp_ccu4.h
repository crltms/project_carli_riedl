/*
 * @file bsp_dac.h
 *
 * @author: Thomas Carli
 */

#ifndef SRC_BSP_BSP_CCU4_H_
#define SRC_BSP_BSP_CCU4_H_

#include <xmc_gpio.h>
#include <xmc_ccu4.h>
#include <xmc_scu.h>
#include <stdio.h>

#include "xmc4500_spi_lib.h"
#include "mcp3004_drv.h"
#include "mcp23s08_drv.h"

/******************************************************************** DEFINES */
/* DIRECTIONS OF PLOTTER */
#define Y_MINUS_PLOT_LOW  0x00
#define Y_MINUS_PLOT_HIGH 0x03
#define Y_PLUS_PLOT_LOW   0x01
#define Y_PLUS_PLOT_HIGH  0x02
#define X_MINUS_PLOT_LOW  0x00
#define X_MINUS_PLOT_HIGH 0x08
#define X_PLUS_PLOT_LOW   0x04
#define X_PLUS_PLOT_HIGH  0x0C

#define SLICE_PTR_CCU40_CC40       CCU40_CC40  // braucht modul-number 2U für Modul(CCU42) und  slice-number 2U für CCU42_CCU42
// #define SLICE_PTR_CCU40_CC41       CCU40_CC41  // braucht modul-number 2U für Modul(CCU42) und  slice-number 2U für CCU42_CCU42
#define MODULE_PTR_CCU40           CCU40
#define SLICE_NUMBER_CCU40_CC40    (0U)  // weil CCU42_CC42
// #define SLICE_NUMBER_CCU40_CC41    (1U)  // weil CCU42_CC42

#define D5 P1_15
#define D6 P1_13
#define D7 P1_14
#define D8 P1_12

_Bool BSP_ConfigCCU4_Timer (void);
_Bool BSP_PWM_SetPen(uint8_t cmd);

#endif

/*! EOF */
