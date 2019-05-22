/*
 * @file bsp_dac.h
 *
 * @author: Thomas Carli
 */

#ifndef SRC_BSP_BSP_CCU8_H_
#define SRC_BSP_BSP_CCU8_H_

#include <xmc_gpio.h>
#include <xmc_ccu8.h>
#include <xmc_scu.h>
#include <stdio.h>

#define MODULE_PTR CCU80
#define MODULE_NUMBER (0U)
#define SLICE0_PTR CCU80_CC80
#define SLICE1_PTR CCU80_CC81
#define SLICE0_NUMBER (0U)
#define SLICE1_NUMBER (1U)

_Bool BSP_ConfigCCU8_Timer (void);
_Bool BSP_PWM_SetDutyCycle_CCU80(int portpin, uint16_t dutycycle);

#endif

/*! EOF */
