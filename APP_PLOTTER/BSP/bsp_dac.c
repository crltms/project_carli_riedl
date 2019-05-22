/*
 *
 * @author: Thomas Carli
 */

#include <bsp_dac.h>

XMC_DAC_CH_CONFIG_t const config=
{
  .output_offset  = 0U,
  .data_type      = XMC_DAC_CH_DATA_TYPE_UNSIGNED,
  .output_scale   = XMC_DAC_CH_OUTPUT_SCALE_NONE,
  .output_negation = XMC_DAC_CH_OUTPUT_NEGATION_DISABLED,
};


// init of DAC0, OUT_1, port / pin 14.9
void BSP_DAC0_1_Init(void){
  /* API to initial DAC Module*/

  XMC_DAC_CH_Init(XMC_DAC0, 1, &config);

  /* API to initial DAC in SingleValue mode */
  XMC_DAC_CH_StartSingleValueMode(XMC_DAC0, 1);

}

/*! EOF */
