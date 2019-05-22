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

#define SLICE_PTR_CCU40_CC43       CCU40_CC43  // braucht modul-number 2U für Modul(CCU42) und  slice-number 2U für CCU42_CCU42
#define SLICE_PTR_CCU40_CC42       CCU40_CC42  // braucht modul-number 2U für Modul(CCU42) und  slice-number 2U für CCU42_CCU42
#define MODULE_PTR_CCU40           CCU40
#define SLICE_NUMBER_CCU40_CC43    (3U)  // weil CCU42_CC42
#define SLICE_NUMBER_CCU40_CC42    (2U)  // weil CCU42_CC42

_Bool BSP_ConfigCCU4_Timer (void);
_Bool BSP_PWM_SetDutyCycle(int led, uint16_t dutycycle);

#endif

/*! EOF */
