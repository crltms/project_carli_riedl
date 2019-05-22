/**
 * @file bsp.c
 * @brief Main board support package for uCOS-III targeting the RelaxKit board.
 *
 * @author Martin Horauer, UAS Technikum Wien
 * @revision 0.1
 * @date 02-2015
 */

/******************************************************************* INCLUDES */
#include  <bsp.h>
#include  <bsp_sys.h>
#include  <bsp_int.h>
#include  <bsp_uart.h>
#include  <bsp_ccu4.h>
#include  <bsp_ccu8.h>
#include  <bsp_dac.h>
#include "xmc4500_spi_lib.h"
#include "mcp3004_drv.h"
#include "mcp23s08_drv.h"

/********************************************************* FILE LOCAL DEFINES */

/******************************************************* FILE LOCAL CONSTANTS */

/*********************************************************** FILE LOCAL TYPES */

/********************************************************* FILE LOCAL GLOBALS */

/****************************************************** FILE LOCAL PROTOTYPES */

/****************************************************************** FUNCTIONS */
/**
 * @function BSP_Init()
 * @params none
 * @returns none
 * @brief Initialization of the board support.
 */
void  BSP_Init (void)
{
	BSP_IntInit();
	BSP_UART_Init();
	BSP_DAC0_1_Init();
	BSP_ConfigCCU4_Timer();
	BSP_ConfigCCU8_Timer();
	initRetargetSwo();
  if(_init_spi()!=SPI_OK)
  {
	  /*Error should never get here*/
  }
	_mcp23s08_reset();
	_mcp23s08_reset_ss(MCP23S08_SS);
	_mcp23s08_reg_xfer(XMC_SPI1_CH0,MCP23S08_IODIR,0xf0,MCP23S08_WR);
	_mcp23s08_set_ss(MCP23S08_SS);
}
/** EOF */
