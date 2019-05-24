
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

#define PWM_PERIOD_VALUE_PEN 2343     // Period of 20ms
// #define PWM_PERIOD_VALUE_XY 176       // Period XY Motor --> 1,5ms

#define PWM_DEF_COMP_VALUE_UP 117     // Compare Match Up --> 1,5ms neutral position
#define PWM_DEF_COMP_VALUE_DOWN 175   // Compare Match Up --> 1ms pen down
// #define PWM_DEF_COMP_VALUE_XY 88      // Compare Match Up --> 0,75ms

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
  // XMC_CCU4_SLICE_CompareInit(SLICE_PTR_CCU40_CC41, &g_timer_object);

  XMC_CCU4_SLICE_SetTimerPeriodMatch(SLICE_PTR_CCU40_CC40, PWM_PERIOD_VALUE_PEN); // sollte man eine Frequenz von 1650Hz herausbekommen
  // XMC_CCU4_SLICE_SetTimerPeriodMatch(SLICE_PTR_CCU40_CC41, PWM_PERIOD_VALUE_XY); // sollte man eine Frequenz von 1650Hz herausbekommen
  XMC_CCU4_SLICE_SetTimerCompareMatch(SLICE_PTR_CCU40_CC40, PWM_DEF_COMP_VALUE_UP);
  // XMC_CCU4_SLICE_SetTimerCompareMatch(SLICE_PTR_CCU40_CC41, PWM_DEF_COMP_VALUE_XY);
  /* Enable shadow transfer */
  XMC_CCU4_EnableShadowTransfer(MODULE_PTR_CCU40, XMC_CCU4_SHADOW_TRANSFER_SLICE_0);
  // XMC_CCU4_EnableShadowTransfer(MODULE_PTR_CCU40, XMC_CCU4_SHADOW_TRANSFER_SLICE_1);
  /* Enable External Start to Event 0 */
  XMC_CCU4_SLICE_StartConfig(SLICE_PTR_CCU40_CC40, XMC_CCU4_SLICE_EVENT_0, XMC_CCU4_SLICE_START_MODE_TIMER_START_CLEAR);
  // XMC_CCU4_SLICE_StartConfig(SLICE_PTR_CCU40_CC41, XMC_CCU4_SLICE_EVENT_0, XMC_CCU4_SLICE_START_MODE_TIMER_START_CLEAR);

  /* Enable compare match events */
  XMC_CCU4_SLICE_EnableEvent(SLICE_PTR_CCU40_CC40, XMC_CCU4_SLICE_IRQ_ID_COMPARE_MATCH_UP);
  // XMC_CCU4_SLICE_EnableEvent(SLICE_PTR_CCU40_CC41, XMC_CCU4_SLICE_IRQ_ID_COMPARE_MATCH_UP);
  /* Connect compare match event to SR0 */
  XMC_CCU4_SLICE_SetInterruptNode(SLICE_PTR_CCU40_CC40, XMC_CCU4_SLICE_IRQ_ID_COMPARE_MATCH_UP, XMC_CCU4_SLICE_SR_ID_0);
  // XMC_CCU4_SLICE_SetInterruptNode(SLICE_PTR_CCU40_CC41, XMC_CCU4_SLICE_IRQ_ID_COMPARE_MATCH_UP, XMC_CCU4_SLICE_SR_ID_1);
  /* Set NVIC priority */
  NVIC_SetPriority(CCU40_0_IRQn, 14U);
  // NVIC_SetPriority(CCU40_1_IRQn, 13U);
  /* Enable IRQ */
  NVIC_EnableIRQ(CCU40_0_IRQn);
  // NVIC_EnableIRQ(CCU40_1_IRQn);
  /* Enable CCU4 PWM output */
  XMC_GPIO_Init(OUTP1_3, &PWM_0_gpio_out_config);
  /* Get the slice out of idle mode */
  XMC_CCU4_EnableClock(MODULE_PTR_CCU40, SLICE_NUMBER_CCU40_CC40);
  // XMC_CCU4_EnableClock(MODULE_PTR_CCU40, SLICE_NUMBER_CCU40_CC41);
  /* Start timer*/
  //XMC_CCU4_SLICE_StartTimer(SLICE_PTR_CCU40_CC40);
  //XMC_CCU4_SLICE_StartTimer(SLICE_PTR_CCU40_CC41);

  return true;
}

_Bool BSP_PWM_SetPen(uint8_t cmd)
{
  if(cmd == 1) // Pen UP --> 1,5ms
  {
    XMC_CCU4_SLICE_SetTimerPeriodMatch(SLICE_PTR_CCU40_CC40, PWM_PERIOD_VALUE_PEN);
    XMC_CCU4_SLICE_SetTimerCompareMatch(SLICE_PTR_CCU40_CC40, PWM_DEF_COMP_VALUE_UP);
    XMC_CCU4_EnableShadowTransfer(MODULE_PTR_CCU40, XMC_CCU4_SHADOW_TRANSFER_SLICE_0);
    XMC_CCU4_SLICE_StartTimer(SLICE_PTR_CCU40_CC40);
  }
  if(cmd == 2)  // Pen Down --> 2ms
  {
    XMC_CCU4_SLICE_SetTimerPeriodMatch(SLICE_PTR_CCU40_CC40, PWM_PERIOD_VALUE_PEN);
    XMC_CCU4_SLICE_SetTimerCompareMatch(SLICE_PTR_CCU40_CC40, PWM_DEF_COMP_VALUE_DOWN);
    XMC_CCU4_EnableShadowTransfer(MODULE_PTR_CCU40, XMC_CCU4_SHADOW_TRANSFER_SLICE_0);
    XMC_CCU4_SLICE_StartTimer(SLICE_PTR_CCU40_CC40);
  }
  // if(led == 3)
  // {
  //   XMC_CCU4_SLICE_SetTimerPeriodMatch(SLICE_PTR_CCU40_CC41, PWM_PERIOD_VALUE_XY);
  //   XMC_CCU4_SLICE_SetTimerCompareMatch(SLICE_PTR_CCU40_CC41, PWM_DEF_COMP_VALUE_XY);
  //   XMC_CCU4_EnableShadowTransfer(MODULE_PTR_CCU40, XMC_CCU4_SHADOW_TRANSFER_SLICE_1);
  //   XMC_CCU4_SLICE_StartTimer(SLICE_PTR_CCU40_CC41);
  // }
  if(cmd == 3) // Timer ausschalten
  {
    XMC_CCU4_SLICE_StopTimer(SLICE_PTR_CCU40_CC40);
  }
  // if(led == 5) // Timer ausschalten
  // {
  //   XMC_CCU4_SLICE_StopTimer(SLICE_PTR_CCU40_CC41);
  // }


	return true;
}
/*! EOF */
