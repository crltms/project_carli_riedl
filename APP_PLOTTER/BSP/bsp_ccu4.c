
/*
 * tio_delay.c
 *
 *  Created on: DD-MM-YYYY
 *      Author: YourName
 */
/*********************************************************************** LIBS */

#include <bsp_ccu4.h>

/******************************************************************** DEFINES */


/****************************************************************** FUNCTIONS */
/*!
*  @brief TODO ???
*  @param TODO ???
*  @return TODO ???
*/
/*!
*  @brief TODO ???
*  @param TODO ???
*  @return TODO ???
*/

/*********************************************************************************************************************
  * GLOBAL DATA
********************************************************************************************************************/

#define PWM_PERIOD_VALUE_1 118//(uint16_t)75757      // 35 bei 10U --> für 1650Hz //29296      // 1 sek. --> clock frequency / 2^12 = 29296 --> period of 1sec.
#define PWM_PERIOD_VALUE_2 176//(uint16_t)75757      // 35 bei 10U --> für 1650Hz //29296      // 1 sek. --> clock frequency / 2^12 = 29296 --> period of 1sec.
#define PWM_PERIOD_VALUE_3 234//(uint16_t)75757      // 35 bei 10U --> für 1650Hz //29296      // 1 sek. --> clock frequency / 2^12 = 29296 --> period of 1sec.

#define PWM_DEF_COMP_VALUE_1 59//(uint16_t)37878    // 0.5 sek. --> (clock frequency / 2^12) = 14648 --> count until this value and reset output --> 0.5sec.
#define PWM_DEF_COMP_VALUE_2 88//(uint16_t)37878    // 0.5 sek. --> (clock frequency / 2^12) = 14648 --> count until this value and reset output --> 0.5sec.
#define PWM_DEF_COMP_VALUE_3 117//(uint16_t)37878    // 0.5 sek. --> (clock frequency / 2^12) = 14648 --> count until this value and reset output --> 0.5sec.

#define OUTP1_3 P1_3

XMC_CCU4_SLICE_COMPARE_CONFIG_t g_timer_object =
{
  .timer_mode 		     = XMC_CCU4_SLICE_TIMER_COUNT_MODE_EA,
  .monoshot   		     = false,
  .shadow_xfer_clear   = 0U,
  .dither_timer_period = 0U,
  .dither_duty_cycle   = 0U,
  .prescaler_mode	     = XMC_CCU4_SLICE_PRESCALER_MODE_NORMAL,
  .mcm_enable		       = 0U,
  .prescaler_initval   = 10U,// --> periode auf 710 setzen für Frequenz von 1650Hz //12U,     // divide clock frequency by 2^12
  .float_limit		     = 0U,
  .dither_limit		     = 0U,
  .passive_level 	     = XMC_CCU4_SLICE_OUTPUT_PASSIVE_LEVEL_LOW,
  .timer_concatenation = 0U
};

/*
 * GPIO Related configuration for PWM output
 */
const XMC_GPIO_CONFIG_t  PWM_0_gpio_out_config	=
{
  .mode                = XMC_GPIO_MODE_OUTPUT_PUSH_PULL_ALT3,
  .output_level        = XMC_GPIO_OUTPUT_LEVEL_LOW,
	.output_strength     = XMC_GPIO_OUTPUT_STRENGTH_MEDIUM,
};


_Bool BSP_ConfigCCU4_Timer(void)
{
  XMC_CCU4_Init(MODULE_PTR_CCU40, XMC_CCU4_SLICE_MCMS_ACTION_TRANSFER_PR_CR);
  /* Start the prescaler and restore clocks to slices */
  XMC_CCU4_StartPrescaler(MODULE_PTR_CCU40);
  /* Start of CCU4 configurations *//* Ensure fCCU reaches CCU4 */
  XMC_CCU4_SetModuleClock(MODULE_PTR_CCU40, XMC_CCU4_CLOCK_SCU);

  /* Initialize the Slice */
  XMC_CCU4_SLICE_CompareInit(SLICE_PTR_CCU40_CC40, &g_timer_object);

  // XMC_CCU4_SLICE_SetTimerCompareMatch(SLICE3_PTR, comparevalue[count]);
  XMC_CCU4_SLICE_SetTimerPeriodMatch(SLICE_PTR_CCU40_CC40, PWM_PERIOD_VALUE_2); // sollte man eine Frequenz von 1650Hz herausbekommen
  XMC_CCU4_SLICE_SetTimerCompareMatch(SLICE_PTR_CCU40_CC40, PWM_DEF_COMP_VALUE_2);
  /* Enable shadow transfer */
  // XMC_CCU4_EnableShadowTransfer(MODULE_PTR_CCU40, (uint32_t)(XMC_CCU4_SHADOW_TRANSFER_SLICE_0|XMC_CCU4_SHADOW_TRANSFER_PRESCALER_SLICE_0));
  XMC_CCU4_EnableShadowTransfer(MODULE_PTR_CCU40, XMC_CCU4_SHADOW_TRANSFER_SLICE_0);
  /* Enable External Start to Event 0 */
  XMC_CCU4_SLICE_StartConfig(SLICE_PTR_CCU40_CC40, XMC_CCU4_SLICE_EVENT_0, XMC_CCU4_SLICE_START_MODE_TIMER_START_CLEAR);
  /* Enable compare match events */
  XMC_CCU4_SLICE_EnableEvent(SLICE_PTR_CCU40_CC40, XMC_CCU4_SLICE_IRQ_ID_COMPARE_MATCH_UP);
  /* Connect compare match event to SR0 */
  XMC_CCU4_SLICE_SetInterruptNode(SLICE_PTR_CCU40_CC40, XMC_CCU4_SLICE_IRQ_ID_COMPARE_MATCH_UP, XMC_CCU4_SLICE_SR_ID_0); //XMC_CCU4_SLICE_IRQ_ID_COMPARE_MATCH_UP
  /* Set NVIC priority */
  NVIC_SetPriority(CCU40_0_IRQn, 3U);
  /* Enable IRQ */
  NVIC_EnableIRQ(CCU40_0_IRQn);
  /* Enable CCU4 PWM output */
  //XMC_GPIO_Init(OUTP1_3, &PWM_0_gpio_out_config);
  /* Get the slice out of idle mode */
  XMC_CCU4_EnableClock(MODULE_PTR_CCU40, SLICE_NUMBER_CCU40_CC40);
  /* Start timer*/
  //XMC_CCU4_SLICE_StartTimer(SLICE_PTR_CCU40_CC40);

  return true;
}

_Bool BSP_PWM_SetDutyCycle(int led, uint16_t dutycycle)
{
	// uint32_t period = PWM_PERIOD_VALUE;
  // uint32_t compare = 0;

  if(led == 1) // Stift hochfahren
  {
    XMC_CCU4_SLICE_SetTimerPeriodMatch(SLICE_PTR_CCU40_CC40, PWM_PERIOD_VALUE_1);
    XMC_CCU4_SLICE_SetTimerCompareMatch(SLICE_PTR_CCU40_CC40, PWM_DEF_COMP_VALUE_1);
    XMC_CCU4_EnableShadowTransfer(MODULE_PTR_CCU40, XMC_CCU4_SHADOW_TRANSFER_SLICE_0);
    XMC_CCU4_SLICE_StartTimer(SLICE_PTR_CCU40_CC40);
  }
  if(led == 2)  //Stift neutrale Position
  {
    XMC_CCU4_SLICE_SetTimerPeriodMatch(SLICE_PTR_CCU40_CC40, PWM_PERIOD_VALUE_2);
    XMC_CCU4_SLICE_SetTimerCompareMatch(SLICE_PTR_CCU40_CC40, PWM_DEF_COMP_VALUE_2);
    XMC_CCU4_EnableShadowTransfer(MODULE_PTR_CCU40, XMC_CCU4_SHADOW_TRANSFER_SLICE_0);
    XMC_CCU4_SLICE_StartTimer(SLICE_PTR_CCU40_CC40);
  }
  if(led == 3)  // Stift runterfahren
  {
    XMC_CCU4_SLICE_SetTimerPeriodMatch(SLICE_PTR_CCU40_CC40, PWM_PERIOD_VALUE_3);
    XMC_CCU4_SLICE_SetTimerCompareMatch(SLICE_PTR_CCU40_CC40, PWM_DEF_COMP_VALUE_3);
    XMC_CCU4_EnableShadowTransfer(MODULE_PTR_CCU40, XMC_CCU4_SHADOW_TRANSFER_SLICE_0);
    XMC_CCU4_SLICE_StartTimer(SLICE_PTR_CCU40_CC40);
  }
  if(led == 4) // Timer ausschalten
  {
    XMC_CCU4_SLICE_StopTimer(SLICE_PTR_CCU40_CC40);
  }

	return true;
}
/*! EOF */
