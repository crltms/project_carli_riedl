
/*
 * tio_delay.c
 *
 *  Created on: DD-MM-YYYY
 *      Author: YourName
 */
/*********************************************************************** LIBS */

#include <bsp_ccu8.h>

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
#define PORT0_5 P0_5
#define PORT0_9 P0_9
#define PORT0_10 P0_10
#define PWM_PERIOD_VALUE 29296      // 1 sek. --> clock frequency / 2^12 = 29296 --> period of 1sec.
#define PWM_DEF_COMP_VALUE 14648    // 0.5 sek. --> (clock frequency / 2^12) = 14648 --> count until this value and reset output --> 0.5sec.


XMC_CCU8_SLICE_COMPARE_CONFIG_t g_timer_object_CCU8_CH1 =
{
  .timer_mode          = XMC_CCU8_SLICE_TIMER_COUNT_MODE_EA,
  .monoshot            = false,
  .shadow_xfer_clear   = false,
  .dither_timer_period = false,
  .dither_duty_cycle   = false,
  .prescaler_mode      = XMC_CCU8_SLICE_PRESCALER_MODE_NORMAL,
  .mcm_ch1_enable      = false,
  .mcm_ch2_enable      = false,
  .slice_status        = XMC_CCU8_SLICE_STATUS_CHANNEL_1,
  .passive_level_out0  = XMC_CCU8_SLICE_OUTPUT_PASSIVE_LEVEL_LOW,
  .passive_level_out1  = XMC_CCU8_SLICE_OUTPUT_PASSIVE_LEVEL_LOW,
  .passive_level_out2  = XMC_CCU8_SLICE_OUTPUT_PASSIVE_LEVEL_LOW,
  .passive_level_out3  = XMC_CCU8_SLICE_OUTPUT_PASSIVE_LEVEL_LOW,
  .asymmetric_pwm      = false,
  .invert_out0         = false,
  .invert_out1         = false,
  .invert_out2         = false,
  .invert_out3         = false,
  .prescaler_initval   = 12U,
  .float_limit         = 0U,
  .dither_limit        = 0U,
  .timer_concatenation = false
};

/* CCU Slice Capture Initialization Data */
XMC_CCU8_SLICE_CAPTURE_CONFIG_t g_capture_object =
{
  .fifo_enable 		     = false,
  .timer_clear_mode    = XMC_CCU8_SLICE_TIMER_CLEAR_MODE_NEVER,
  .same_event          = false,
  .ignore_full_flag    = false,
  .prescaler_mode	     = XMC_CCU8_SLICE_PRESCALER_MODE_NORMAL,
  .prescaler_initval   = 0U,
  .float_limit		     = 0U,
  .timer_concatenation = false
};

const XMC_GPIO_CONFIG_t  PWM_1_gpio_out_config	=
{
  .mode                = XMC_GPIO_MODE_OUTPUT_PUSH_PULL_ALT3,
  .output_level        = XMC_GPIO_OUTPUT_LEVEL_LOW,
	.output_strength     = XMC_GPIO_OUTPUT_STRENGTH_MEDIUM,
};

_Bool BSP_ConfigCCU8_Timer(void)
{
  /* Enable CCU8 module */
  XMC_CCU8_Init(MODULE_PTR, XMC_CCU8_SLICE_MCMS_ACTION_TRANSFER_PR_CR);
  /* Start the prescaler */
  XMC_CCU8_StartPrescaler(MODULE_PTR);
  /* Ensure fCCU reaches CCU80 */
  XMC_CCU8_SetModuleClock(MODULE_PTR, XMC_CCU8_CLOCK_SCU);
  /* Configure CCU8x_CC8y slice as timer */
  XMC_CCU8_SLICE_CompareInit(SLICE0_PTR, &g_timer_object_CCU8_CH1);
  XMC_CCU8_SLICE_CompareInit(SLICE1_PTR, &g_timer_object_CCU8_CH1);

  /* Set period match value of the timer */
  XMC_CCU8_SLICE_SetTimerPeriodMatch(SLICE0_PTR, PWM_PERIOD_VALUE);
  XMC_CCU8_SLICE_SetTimerPeriodMatch(SLICE1_PTR, PWM_PERIOD_VALUE);

  /* Set timer compare match value for channel 1 - 50% duty */
  XMC_CCU8_SLICE_SetTimerCompareMatch(SLICE0_PTR, XMC_CCU8_SLICE_COMPARE_CHANNEL_1, PWM_DEF_COMP_VALUE);
  XMC_CCU8_SLICE_SetTimerCompareMatch(SLICE0_PTR, XMC_CCU8_SLICE_COMPARE_CHANNEL_2, PWM_DEF_COMP_VALUE);

  XMC_CCU8_SLICE_SetTimerCompareMatch(SLICE1_PTR, XMC_CCU8_SLICE_COMPARE_CHANNEL_2, PWM_DEF_COMP_VALUE);

  /* Transfer value from shadow timer registers to actual timer registers */
  XMC_CCU8_EnableShadowTransfer(MODULE_PTR, XMC_CCU8_SHADOW_TRANSFER_SLICE_0);

  XMC_CCU8_EnableShadowTransfer(MODULE_PTR, XMC_CCU8_SHADOW_TRANSFER_SLICE_1);

  /* Configure events */
  /* Enable events: Period Match and Compare Match-Ch2 */
  XMC_CCU8_SLICE_EnableEvent(SLICE0_PTR, XMC_CCU8_SLICE_IRQ_ID_COMPARE_MATCH_UP_CH_1);
  XMC_CCU8_SLICE_EnableEvent(SLICE0_PTR, XMC_CCU8_SLICE_IRQ_ID_COMPARE_MATCH_UP_CH_2);

  XMC_CCU8_SLICE_EnableEvent(SLICE1_PTR, XMC_CCU8_SLICE_IRQ_ID_COMPARE_MATCH_UP_CH_2);

  /* Connect event to SR0 and SR2 */
  XMC_CCU8_SLICE_SetInterruptNode(SLICE0_PTR, XMC_CCU8_SLICE_IRQ_ID_PERIOD_MATCH, XMC_CCU8_SLICE_SR_ID_0);
  XMC_CCU8_SLICE_SetInterruptNode(SLICE0_PTR, XMC_CCU8_SLICE_IRQ_ID_PERIOD_MATCH, XMC_CCU8_SLICE_SR_ID_2);

  XMC_CCU8_SLICE_SetInterruptNode(SLICE1_PTR, XMC_CCU8_SLICE_IRQ_ID_PERIOD_MATCH, XMC_CCU8_SLICE_SR_ID_2);

  /* Configure NVIC */
  /* Set priority */
  NVIC_SetPriority(CCU80_0_IRQn, 5U);
  NVIC_SetPriority(CCU80_2_IRQn, 4U);

  NVIC_SetPriority(CCU80_3_IRQn, 3U);
  /* Enable IRQ */
  NVIC_EnableIRQ(CCU80_0_IRQn);
  NVIC_EnableIRQ(CCU80_2_IRQn);

  NVIC_EnableIRQ(CCU80_3_IRQn);

   /*Initializes the GPIO*/
  // XMC_GPIO_Init(PORT0_5, &PWM_1_gpio_out_config);
  // XMC_GPIO_Init(PORT0_10, &PWM_1_gpio_out_config);
  // XMC_GPIO_Init(PORT0_9, &PWM_1_gpio_out_config);
  /* Get the slice out of idle mode */
  XMC_CCU8_EnableClock(MODULE_PTR, SLICE0_NUMBER);

  XMC_CCU8_EnableClock(MODULE_PTR, SLICE1_NUMBER);
  /* Start the timer */
  XMC_CCU8_SLICE_StartTimer(SLICE0_PTR);

  XMC_CCU8_SLICE_StartTimer(SLICE1_PTR);

  return true;
}

_Bool BSP_PWM_SetDutyCycle_CCU80(int portpin, uint16_t dutycycle)
{
	uint32_t period = PWM_PERIOD_VALUE;
  uint32_t compare = 0;

  compare = (100-dutycycle)*(period+1)/100; // --> from page 15 of AP32287-XMC1000_XMC4000-Capture Compare Unit 4 (CCU4).pdf
  if(portpin == 5)
  {
    XMC_CCU8_SLICE_SetTimerCompareMatch(SLICE0_PTR, XMC_CCU8_SLICE_COMPARE_CHANNEL_1, (uint16_t)compare);
    XMC_CCU8_EnableShadowTransfer(MODULE_PTR, XMC_CCU8_SHADOW_TRANSFER_SLICE_0);
  }
  if(portpin == 9)
  {
    XMC_CCU8_SLICE_SetTimerCompareMatch(SLICE1_PTR, XMC_CCU8_SLICE_COMPARE_CHANNEL_2, (uint16_t)compare);
    XMC_CCU8_EnableShadowTransfer(MODULE_PTR, XMC_CCU8_SHADOW_TRANSFER_SLICE_1);
  }
  if(portpin == 10)
  {
    XMC_CCU8_SLICE_SetTimerCompareMatch(SLICE0_PTR, XMC_CCU8_SLICE_COMPARE_CHANNEL_2, (uint16_t)compare);
    XMC_CCU8_EnableShadowTransfer(MODULE_PTR, XMC_CCU8_SHADOW_TRANSFER_SLICE_0);
  }

	return true;
}
/*! EOF */
