
/**
 * @file xmc_eth_mac.c
 * @date 2015-06-20 
 *
 * @cond
 *********************************************************************************************************************
 * XMClib v2.0.0 - XMC Peripheral Driver Library
 *
 * Copyright (c) 2015, Infineon Technologies AG
 * All rights reserved.                        
 *                                             
 * Redistribution and use in source and binary forms, with or without modification,are permitted provided that the 
 * following conditions are met:   
 *                                                                              
 * Redistributions of source code must retain the above copyright notice, this list of conditions and the following 
 * disclaimer.                        
 * 
 * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following 
 * disclaimer in the documentation and/or other materials provided with the distribution.
 * 
 * Neither the name of the copyright holders nor the names of its contributors may be used to endorse or promote 
 * products derived from this software without specific prior written permission.                                           
 *                                                                              
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, 
 * INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE  
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE  FOR ANY DIRECT, INDIRECT, INCIDENTAL, 
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR  
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, 
 * WHETHER IN CONTRACT, STRICT LIABILITY,OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE 
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.                                                  
 *                                                                              
 * To improve the quality of the software, users are encouraged to share modifications, enhancements or bug fixes with 
 * Infineon Technologies AG dave@infineon.com).                                                          
 *********************************************************************************************************************
 *
 * Change History
 * --------------
 *
 * 2015-06-20:
 *     - Initial <br>
 *
 * @endcond
 */

/*******************************************************************************
 * HEADER FILES
 *******************************************************************************/

#include <xmc_eth_mac.h>

#if defined (ETH0)

#include <stdlib.h>
#include <xmc_scu.h>

/*******************************************************************************
 * MACROS
 *******************************************************************************/

/**
 * ETH MAC clock speed
 */
#define XMC_ETH_MAC_CLK_SPEED_35MHZ  (35000000U)  /**< ETH MAC clock speed 35 MHZ */
#define XMC_ETH_MAC_CLK_SPEED_60MHZ  (60000000U)  /**< ETH MAC clock speed 60 MHZ */
#define XMC_ETH_MAC_CLK_SPEED_100MHZ (100000000U) /**< ETH MAC clock speed 100 MHZ */
#define XMC_ETH_MAC_CLK_SPEED_150MHZ (150000000U) /**< ETH MAC clock speed 150 MHZ */
#define XMC_ETH_MAC_CLK_SPEED_200MHZ (200000000U) /**< ETH MAC clock speed 200 MHZ */
#define XMC_ETH_MAC_CLK_SPEED_250MHZ (250000000U) /**< ETH MAC clock speed 250 MHZ */

/**
 * ETH MAC MDC divider
 */
#define XMC_ETH_MAC_MDC_DIVIDER_16  (2U << ETH_GMII_ADDRESS_CR_Pos) /**< MDC clock: ETH clock/16 */
#define XMC_ETH_MAC_MDC_DIVIDER_26  (3U << ETH_GMII_ADDRESS_CR_Pos) /**< MDC clock: ETH clock/26 */
#define XMC_ETH_MAC_MDC_DIVIDER_42  (0U << ETH_GMII_ADDRESS_CR_Pos) /**< MDC clock: ETH clock/42 */
#define XMC_ETH_MAC_MDC_DIVIDER_62  (1U << ETH_GMII_ADDRESS_CR_Pos) /**< MDC clock: ETH clock/62 */
#define XMC_ETH_MAC_MDC_DIVIDER_102 (4U << ETH_GMII_ADDRESS_CR_Pos) /**< MDC clock: ETH clock/102 */
#define XMC_ETH_MAC_MDC_DIVIDER_124 (5U << ETH_GMII_ADDRESS_CR_Pos) /**< MDC clock: ETH clock/124 */

/**
 * TDES0 Descriptor TX Packet Control/Status
 */
#define ETH_MAC_DMA_TDES0_OWN  (0x80000000U) /**< Own bit 1=DMA, 0=CPU */
#define ETH_MAC_DMA_TDES0_IC   (0x40000000U) /**< Interrupt on competition */
#define ETH_MAC_DMA_TDES0_LS   (0x20000000U) /**< Last segment */
#define ETH_MAC_DMA_TDES0_FS   (0x10000000U) /**< First segment */
#define ETH_MAC_DMA_TDES0_DC   (0x08000000U) /**< Disable CRC */
#define ETH_MAC_DMA_TDES0_DP   (0x04000000U) /**< Disable pad */
#define ETH_MAC_DMA_TDES0_TTSE (0x02000000U) /**< Transmit time stamp enable */
#define ETH_MAC_DMA_TDES0_CIC  (0x00C00000U) /**< Checksum insertion control */
#define ETH_MAC_DMA_TDES0_TER  (0x00200000U) /**< Transmit end of ring */
#define ETH_MAC_DMA_TDES0_TCH  (0x00100000U) /**< Second address chained */
#define ETH_MAC_DMA_TDES0_TTSS (0x00020000U) /**< Transmit time stamp status */
#define ETH_MAC_DMA_TDES0_IHE  (0x00010000U) /**< IP header error */
#define ETH_MAC_DMA_TDES0_ES   (0x00008000U) /**< Error summary */
#define ETH_MAC_DMA_TDES0_JT   (0x00004000U) /**< Jabber timeout */
#define ETH_MAC_DMA_TDES0_FF   (0x00002000U) /**< Frame flushed */
#define ETH_MAC_DMA_TDES0_IPE  (0x00001000U) /**< IP payload error */
#define ETH_MAC_DMA_TDES0_LOC  (0x00000800U) /**< Loss of carrier */
#define ETH_MAC_DMA_TDES0_NC   (0x00000400U) /**< No carrier */
#define ETH_MAC_DMA_TDES0_LC   (0x00000200U) /**< Late collision */
#define ETH_MAC_DMA_TDES0_EC   (0x00000100U) /**< Excessive collision */
#define ETH_MAC_DMA_TDES0_VF   (0x00000080U) /**< VLAN frame */
#define ETH_MAC_DMA_TDES0_CC   (0x00000078U) /**< Collision count */
#define ETH_MAC_DMA_TDES0_ED   (0x00000004U) /**< Excessive deferral */
#define ETH_MAC_DMA_TDES0_UF   (0x00000002U) /**< Underflow error */
#define ETH_MAC_DMA_TDES0_DB   (0x00000001U) /**< Deferred bit */

/**
 * RDES0 Descriptor RX Packet Status
 */
#define ETH_MAC_DMA_RDES0_OWN  (0x80000000U) /**< Own bit 1=DMA, 0=CPU */
#define ETH_MAC_DMA_RDES0_AFM  (0x40000000U) /**< Destination address filter fail */
#define ETH_MAC_DMA_RDES0_FL   (0x3FFF0000U) /**< Frame length mask */
#define ETH_MAC_DMA_RDES0_ES   (0x00008000U) /**< Error summary */
#define ETH_MAC_DMA_RDES0_DE   (0x00004000U) /**< Descriptor error */
#define ETH_MAC_DMA_RDES0_SAF  (0x00002000U) /**< Source address filter fail */
#define ETH_MAC_DMA_RDES0_LE   (0x00001000U) /**< Length error */
#define ETH_MAC_DMA_RDES0_OE   (0x00000800U) /**< Overflow error */
#define ETH_MAC_DMA_RDES0_VLAN (0x00000400U) /**< VLAN tag */
#define ETH_MAC_DMA_RDES0_FS   (0x00000200U) /**< First descriptor */
#define ETH_MAC_DMA_RDES0_LS   (0x00000100U) /**< Last descriptor */
#define ETH_MAC_DMA_RDES0_TSA  (0x00000080U) /**< Timestamp available */
#define ETH_MAC_DMA_RDES0_LC   (0x00000040U) /**< Late collision */
#define ETH_MAC_DMA_RDES0_FT   (0x00000020U) /**< Frame type */
#define ETH_MAC_DMA_RDES0_RWT  (0x00000010U) /**< Receive watchdog timeout */
#define ETH_MAC_DMA_RDES0_RE   (0x00000008U) /**< Receive error */
#define ETH_MAC_DMA_RDES0_DBE  (0x00000004U) /**< Dribble bit error */
#define ETH_MAC_DMA_RDES0_CE   (0x00000002U) /**< CRC error */
#define ETH_MAC_DMA_RDES0_ESA  (0x00000001U) /**< Extended Status/Rx MAC address */

/**
 * RDES1 Descriptor RX Packet Control
 */
#define ETH_MAC_DMA_RDES1_RBS2 (0x1FFF0000U) /**< Receive buffer 2 size */
#define ETH_MAC_DMA_RDES1_RER  (0x00008000U) /**< Receive end of ring */
#define ETH_MAC_DMA_RDES1_RCH  (0x00004000U) /**< Second address chained */
#define ETH_MAC_DMA_RDES1_RBS1 (0x00001FFFU) /**< Receive buffer 1 size */

/**
 * Normal MAC events
 */
#define ETH_MAC_EVENT_NORMAL (XMC_ETH_MAC_EVENT_TRANSMIT |\
                              XMC_ETH_MAC_EVENT_TRANSMIT_BUFFER_UNAVAILABLE |\
                              XMC_ETH_MAC_EVENT_RECEIVE |\
                              XMC_ETH_MAC_EVENT_EARLY_RECEIVE)

/**
 * Abnormal MAC events
 */
#define ETH_MAC_EVENT_ABNORMAL (XMC_ETH_MAC_EVENT_TRANSMIT_PROCESS_STOPPED |\
                                XMC_ETH_MAC_EVENT_TRANSMIT_JABBER_TIMEOUT |\
                                XMC_ETH_MAC_EVENT_RECEIVE_OVERFLOW |\
                                XMC_ETH_MAC_EVENT_TRANSMIT_UNDERFLOW |\
                                XMC_ETH_MAC_EVENT_RECEIVE_BUFFER_UNAVAILABLE |\
                                XMC_ETH_MAC_EVENT_RECEIVE_PROCESS_STOPPED |\
                                XMC_ETH_MAC_EVENT_RECEIVE_WATCHDOG_TIMEOUT |\
                                XMC_ETH_MAC_EVENT_EARLY_TRANSMIT |\
                                XMC_ETH_MAC_EVENT_BUS_ERROR)

/* Definition needed in case of device header file previous to v1.5.1*/
#ifndef ETH_BUS_MODE_ATDS_Msk
#define ETH_BUS_MODE_ATDS_Msk  (0x00000080UL)
#endif
/*******************************************************************************
 * API IMPLEMENTATION
 *******************************************************************************/

/* Check if the event passed is a normal event */
__STATIC_INLINE bool XCM_ETH_MAC_IsNormalEvent(uint32_t event)
{
  return (bool)((event & ((uint32_t)XMC_ETH_MAC_EVENT_TRANSMIT |
                          (uint32_t)XMC_ETH_MAC_EVENT_TRANSMIT_BUFFER_UNAVAILABLE |
                          (uint32_t)XMC_ETH_MAC_EVENT_RECEIVE |
                          (uint32_t)XMC_ETH_MAC_EVENT_EARLY_RECEIVE)) != (uint32_t)0);
}

/* Check if the event passed is an abnormal event */
__STATIC_INLINE bool XCM_ETH_MAC_IsAbnormalEvent(uint32_t event)
{
   return (bool)((event & ((uint32_t)XMC_ETH_MAC_EVENT_TRANSMIT_PROCESS_STOPPED |
                           (uint32_t)XMC_ETH_MAC_EVENT_TRANSMIT_JABBER_TIMEOUT |
                           (uint32_t)XMC_ETH_MAC_EVENT_RECEIVE_OVERFLOW |
                           (uint32_t)XMC_ETH_MAC_EVENT_TRANSMIT_UNDERFLOW |
                           (uint32_t)XMC_ETH_MAC_EVENT_RECEIVE_BUFFER_UNAVAILABLE |
                           (uint32_t)XMC_ETH_MAC_EVENT_RECEIVE_PROCESS_STOPPED |
                           (uint32_t)XMC_ETH_MAC_EVENT_RECEIVE_WATCHDOG_TIMEOUT |
                           (uint32_t)XMC_ETH_MAC_EVENT_EARLY_TRANSMIT |
                           (uint32_t)XMC_ETH_MAC_EVENT_BUS_ERROR)) != (uint32_t)0);
 }

#ifdef XMC_ASSERT_ENABLE 
 
/* Check if the passed argument is a valid ETH module */ 
__STATIC_INLINE bool XMC_ETH_MAC_IsValidModule(ETH_GLOBAL_TypeDef *const eth)
{
  return (eth == ETH0);
}

#endif

/* ETH MAC initialize */
XMC_ETH_MAC_STATUS_t XMC_ETH_MAC_Init(XMC_ETH_MAC_t *const eth_mac)
{
  XMC_ETH_MAC_STATUS_t status;

  XMC_ASSERT("XMC_ETH_MAC_Init: eth_mac is invalid", XMC_ETH_MAC_IsValidModule(eth_mac->regs));

  XMC_ETH_MAC_Enable(eth_mac);
  XMC_ETH_MAC_Reset(eth_mac);

  status = XMC_ETH_MAC_SetManagmentClockDivider(eth_mac);

  XMC_ETH_MAC_SetAddress(eth_mac, eth_mac->address);

  /* Initialize MAC configuration */
  eth_mac->regs->MAC_CONFIGURATION = (uint32_t)ETH_MAC_CONFIGURATION_IPC_Msk;

  /* Initialize Filter registers */
  eth_mac->regs->FLOW_CONTROL = ETH_FLOW_CONTROL_DZPQ_Msk; /* Disable Zero Quanta Pause */

  eth_mac->regs->OPERATION_MODE = (uint32_t)ETH_OPERATION_MODE_RSF_Msk |
		                          (uint32_t)ETH_OPERATION_MODE_TSF_Msk;

  /* Increase enhanced descriptor to 8 WORDS, required when the Advanced Time-Stamp feature or Full IPC Offload Engine is enabled */
  eth_mac->regs->BUS_MODE |= (uint32_t)ETH_BUS_MODE_ATDS_Msk;

  /* Initialize DMA Descriptors */
  XMC_ETH_MAC_InitRxDescriptors(eth_mac);
  XMC_ETH_MAC_InitTxDescriptors(eth_mac);

  /* Clear interrupts */
  eth_mac->regs->STATUS = 0xFFFFFFFFUL;

  eth_mac->frame_end = NULL;

  return status;
}

/* Initialize RX descriptors */
void XMC_ETH_MAC_InitRxDescriptors(XMC_ETH_MAC_t *const eth_mac)
{
  uint32_t i;
  uint32_t next;

  XMC_ASSERT("XMC_ETH_MAC_InitRxDescriptors: eth_mac is invalid", XMC_ETH_MAC_IsValidModule(eth_mac->regs));

  /*
   * Chained structure (ETH_MAC_DMA_RDES1_RCH), second address in the descriptor
   * (buffer2) is the next descriptor address
   */
  for (i = 0U; i < eth_mac->num_rx_buf; ++i)
  {
	eth_mac->rx_desc[i].status = (uint32_t)ETH_MAC_DMA_RDES0_OWN;
	eth_mac->rx_desc[i].length = (uint32_t)ETH_MAC_DMA_RDES1_RCH | (uint32_t)XMC_ETH_MAC_BUF_SIZE;
	eth_mac->rx_desc[i].buffer1 = (uint32_t)&(eth_mac->rx_buf[i * XMC_ETH_MAC_BUF_SIZE]);
    next = i + 1U;
    if (next == eth_mac->num_rx_buf)
    {
      next = 0U;
    }
    eth_mac->rx_desc[i].buffer2 = (uint32_t)&(eth_mac->rx_desc[next]);
  }
  eth_mac->regs->RECEIVE_DESCRIPTOR_LIST_ADDRESS = (uint32_t)&(eth_mac->rx_desc[0]);
  eth_mac->rx_index = 0U;
}

/* Initialize TX descriptors */
void XMC_ETH_MAC_InitTxDescriptors(XMC_ETH_MAC_t *const eth_mac)
{
  uint32_t i;
  uint32_t next;

  XMC_ASSERT("XMC_ETH_MAC_InitTxDescriptors: eth_mac is invalid", XMC_ETH_MAC_IsValidModule(eth_mac->regs));

  /* Chained structure (ETH_MAC_DMA_TDES0_TCH), second address in the descriptor (buffer2) is the next descriptor address */
  for (i = 0U; i < eth_mac->num_tx_buf; ++i)
  {
	eth_mac->tx_desc[i].status = ETH_MAC_DMA_TDES0_TCH | ETH_MAC_DMA_TDES0_LS | ETH_MAC_DMA_TDES0_FS;
	eth_mac->tx_desc[i].buffer1 = (uint32_t)&(eth_mac->tx_buf[i * XMC_ETH_MAC_BUF_SIZE]);
    next = i + 1U;
    if (next == eth_mac->num_tx_buf)
    {
        next = 0U;
    }
    eth_mac->tx_desc[i].buffer2 = (uint32_t)&(eth_mac->tx_desc[next]);
  }
  eth_mac->regs->TRANSMIT_DESCRIPTOR_LIST_ADDRESS = (uint32_t)&(eth_mac->tx_desc[0]);
  eth_mac->tx_index = 0U;
}

/* Set address perfect filter */
void XMC_ETH_MAC_SetAddressPerfectFilter(XMC_ETH_MAC_t *const eth_mac,
		                                 uint8_t index,
								         const uint64_t addr,
								         uint32_t flags)
{
  __IO uint32_t *reg;

  XMC_ASSERT("XMC_ETH_MAC_SetAddressPerfectFilter: eth_mac is invalid", XMC_ETH_MAC_IsValidModule(eth_mac->regs));
  XMC_ASSERT("XMC_ETH_MAC_SetAddressFilter: index is out of range", ((index > 0) && (index < 4)));

  reg = &(eth_mac->regs->MAC_ADDRESS0_HIGH);
  reg[index] = (uint32_t)(addr >> 32U) | flags;
  reg[index + 1U] = (uint32_t)addr;
}

/* Set address hash filter */
void XMC_ETH_MAC_SetAddressHashFilter(XMC_ETH_MAC_t *const eth_mac, const uint64_t hash)
{
  eth_mac->regs->HASH_TABLE_HIGH = (uint32_t)(hash >> 32);
  eth_mac->regs->HASH_TABLE_LOW = (uint32_t)hash;
}

/* Send frame */
XMC_ETH_MAC_STATUS_t XMC_ETH_MAC_SendFrame(XMC_ETH_MAC_t *const eth_mac, const uint8_t *frame, uint32_t len, uint32_t flags)
{
  XMC_ETH_MAC_STATUS_t status;
  uint8_t *dst;
  uint32_t ctrl;

  XMC_ASSERT("XMC_ETH_MAC_SendFrame:", eth_mac != NULL);
  XMC_ASSERT("XMC_ETH_MAC_SendFrame:", eth_mac->regs != ETH0);
  XMC_ASSERT("XMC_ETH_MAC_SendFrame:", (frame != NULL) && (len > 0));

  dst = eth_mac->frame_end;

  if (eth_mac->tx_desc[eth_mac->tx_index].status & ETH_MAC_DMA_TDES0_OWN)
  {
    /* Transmitter is busy, wait */
    status = XMC_ETH_MAC_STATUS_BUSY;
  }
  else
  {
    if (dst == NULL)
    {
      /* Start of a new transmit frame */
      dst = (uint8_t *)eth_mac->tx_desc[eth_mac->tx_index].buffer1;
      eth_mac->tx_desc[eth_mac->tx_index].length = len;
    }
    else
    {
      /* Sending data fragments in progress */
      eth_mac->tx_desc[eth_mac->tx_index].length += len;
    }

    memcpy(dst, frame, len);

    if (flags & (uint32_t)XMC_ETH_MAC_TX_FRAME_FRAGMENT)
    {
      /* More data to come, remember current write position */
      eth_mac->frame_end = dst;
    }
    else
    {
      /* Frame is now ready, send it to DMA */
      ctrl = eth_mac->tx_desc[eth_mac->tx_index].status | ETH_MAC_DMA_TDES0_CIC;
      ctrl &= ~(ETH_MAC_DMA_TDES0_IC | ETH_MAC_DMA_TDES0_TTSE);

      if (flags & (uint32_t)XMC_ETH_MAC_TX_FRAME_EVENT)
      {
    	ctrl |= ETH_MAC_DMA_TDES0_IC;
      }

      if (flags & (uint32_t)XMC_ETH_MAC_TX_FRAME_TIMESTAMP)
      {
    	ctrl |= ETH_MAC_DMA_TDES0_TTSE;
      }
      eth_mac->tx_ts_index = eth_mac->tx_index;

      eth_mac->tx_desc[eth_mac->tx_index].status = ctrl | ETH_MAC_DMA_TDES0_OWN;

      eth_mac->tx_index++;
      if (eth_mac->tx_index == eth_mac->num_tx_buf)
      {
    	eth_mac->tx_index = 0U;
      }
      eth_mac->frame_end = NULL;

      /* Start frame transmission */
      eth_mac->regs->STATUS = (uint32_t)ETH_STATUS_TPS_Msk;
      eth_mac->regs->TRANSMIT_POLL_DEMAND = 0U;
    }
    status = XMC_ETH_MAC_STATUS_OK;
  }

  return status;
}

/* Read frame */
uint32_t XMC_ETH_MAC_ReadFrame(XMC_ETH_MAC_t *const eth_mac, uint8_t *frame, uint32_t len)
{
  uint8_t const *src;

  XMC_ASSERT("XMC_ETH_MAC_ReadFrame:", eth_mac != NULL);
  XMC_ASSERT("XMC_ETH_MAC_ReadFrame:", eth_mac->regs != ETH0);
  XMC_ASSERT("XMC_ETH_MAC_ReadFrame:", (frame != NULL) && (len > 0));

  /* Fast-copy data to packet buffer */
  src = (uint8_t const *)eth_mac->rx_desc[eth_mac->rx_index].buffer1;

  memcpy(frame, src, len);

  /* Return this block back to DMA */
  eth_mac->rx_desc[eth_mac->rx_index].status = ETH_MAC_DMA_RDES0_OWN;

  eth_mac->rx_index++;
  if (eth_mac->rx_index == eth_mac->num_rx_buf)
  {
    eth_mac->rx_index = 0U;
  }

  if (eth_mac->regs->STATUS & ETH_STATUS_RU_Msk)
  {
    /* Receive buffer unavailable, resume DMA */
	eth_mac->regs->STATUS = (uint32_t)ETH_STATUS_RU_Msk;
	eth_mac->regs->RECEIVE_POLL_DEMAND = 0U;
  }

  return (len);
}

/* Get RX frame size */
uint32_t XMC_ETH_MAC_GetRxFrameSize(XMC_ETH_MAC_t *const eth_mac)
{
  uint32_t status;
  uint32_t len = 0U;

  status = eth_mac->rx_desc[eth_mac->rx_index].status;

  if (status & ETH_MAC_DMA_RDES0_OWN)
  {
    /* Owned by DMA */
    len = 0U;
  }

  if (((status & ETH_MAC_DMA_RDES0_ES) != 0U) ||
      ((status & ETH_MAC_DMA_RDES0_FS) == 0U) ||
      ((status & ETH_MAC_DMA_RDES0_LS) == 0U)) {
    /* Error, this block is invalid */
    len = 0xFFFFFFFFU;
  }

  /* Subtract CRC */
  len = ((status & ETH_MAC_DMA_RDES0_FL) >> 16U) - 4U;

  return len;
}

/* Set management clock divider */
XMC_ETH_MAC_STATUS_t XMC_ETH_MAC_SetManagmentClockDivider(XMC_ETH_MAC_t *const eth_mac)
{
  uint32_t eth_mac_clk;
  XMC_ETH_MAC_STATUS_t status;
  __IO uint32_t *reg;

  eth_mac_clk = XMC_SCU_CLOCK_GetEthernetClockFrequency();
  status = XMC_ETH_MAC_STATUS_OK;

  reg = &(eth_mac->regs->GMII_ADDRESS);
  if (eth_mac_clk <= XMC_ETH_MAC_CLK_SPEED_35MHZ)
  {
	*reg = XMC_ETH_MAC_MDC_DIVIDER_16;
  }
  else if (eth_mac_clk <= XMC_ETH_MAC_CLK_SPEED_60MHZ)
  {
	*reg = XMC_ETH_MAC_MDC_DIVIDER_26;
  }
  else if (eth_mac_clk <= XMC_ETH_MAC_CLK_SPEED_100MHZ)
  {
	*reg = XMC_ETH_MAC_MDC_DIVIDER_42;
  }
  else if (eth_mac_clk <= XMC_ETH_MAC_CLK_SPEED_150MHZ)
  {
	*reg = XMC_ETH_MAC_MDC_DIVIDER_62;
  }
  else if (eth_mac_clk <= XMC_ETH_MAC_CLK_SPEED_200MHZ)
  {
	*reg = XMC_ETH_MAC_MDC_DIVIDER_102;
  }
  else if (eth_mac_clk <= XMC_ETH_MAC_CLK_SPEED_250MHZ)
  {
	*reg = XMC_ETH_MAC_MDC_DIVIDER_124;
  }
  else
  {
    status = XMC_ETH_MAC_STATUS_ERROR;
  }

  return status;
}

/* ETH MAC enable */
void XMC_ETH_MAC_Enable(XMC_ETH_MAC_t *const eth_mac)
{
  XMC_SCU_CLOCK_EnableClock(XMC_SCU_CLOCK_ETH);
  XMC_SCU_RESET_DeassertPeripheralReset(XMC_SCU_PERIPHERAL_RESET_ETH0);
}

/* ETH MAC disable */
void XMC_ETH_MAC_Disable(XMC_ETH_MAC_t *const eth_mac)
{
  XMC_SCU_RESET_AssertPeripheralReset(XMC_SCU_PERIPHERAL_RESET_ETH0);
  XMC_SCU_CLOCK_DisableClock(XMC_SCU_CLOCK_ETH);
}

/* Read physical layer and obtain status */
XMC_ETH_MAC_STATUS_t XMC_ETH_MAC_ReadPhy(XMC_ETH_MAC_t *eth_mac, uint8_t phy_addr, uint8_t reg_addr, uint16_t *data)
{
  uint32_t retries;

  XMC_ASSERT("XMC_ETH_MAC_PhyRead: Parameter error", data != NULL);

  eth_mac->regs->GMII_ADDRESS = (uint32_t)((eth_mac->regs->GMII_ADDRESS & (uint32_t)ETH_GMII_ADDRESS_CR_Msk) |
                                           (uint32_t)ETH_GMII_ADDRESS_MB_Msk |
                                           (uint32_t)((uint32_t)phy_addr << ETH_GMII_ADDRESS_PA_Pos) |
                                           (uint32_t)((uint32_t)reg_addr << ETH_GMII_ADDRESS_MR_Pos));

  /* Poll busy bit during max PHY_TIMEOUT time */
  retries = 0U;
  do
  {
    if ((eth_mac->regs->GMII_ADDRESS & ETH_GMII_ADDRESS_MB_Msk) == 0U)
    {
      *data = (uint16_t)(eth_mac->regs->GMII_DATA & ETH_GMII_DATA_MD_Msk);
      return XMC_ETH_MAC_STATUS_OK;
    }
    ++retries;
  } while (retries < XMC_ETH_MAC_PHY_MAX_RETRIES);

  return XMC_ETH_MAC_STATUS_ERROR;
}

/* Write physical layer and return status */
XMC_ETH_MAC_STATUS_t XMC_ETH_MAC_WritePhy(XMC_ETH_MAC_t *eth_mac, uint8_t phy_addr, uint8_t reg_addr, uint16_t data)
{
  uint32_t retries;

  eth_mac->regs->GMII_DATA  = data;
  eth_mac->regs->GMII_ADDRESS = (uint32_t)((eth_mac->regs->GMII_ADDRESS & (uint32_t)ETH_GMII_ADDRESS_CR_Msk) |
                                           (uint32_t)ETH_GMII_ADDRESS_MB_Msk |
                                           (uint32_t)ETH_GMII_ADDRESS_MW_Msk |
                                           (uint32_t)((uint32_t)phy_addr << ETH_GMII_ADDRESS_PA_Pos) |
                                           (uint32_t)((uint32_t)reg_addr << ETH_GMII_ADDRESS_MR_Pos));

   /* Poll busy bit during max PHY_TIMEOUT time */
  retries = 0U;
  do
  {
    if ((eth_mac->regs->GMII_ADDRESS & ETH_GMII_ADDRESS_MB_Msk) == 0U)
    {
      return XMC_ETH_MAC_STATUS_OK;
    }
    ++retries;
  } while (retries < XMC_ETH_MAC_PHY_MAX_RETRIES);

  return XMC_ETH_MAC_STATUS_ERROR;
}

/* Flush TX */
void XMC_ETH_MAC_FlushTx(XMC_ETH_MAC_t *const eth_mac)
{
  eth_mac->regs->OPERATION_MODE &= (uint32_t)~ETH_OPERATION_MODE_ST_Msk;
  XMC_ETH_MAC_InitTxDescriptors(eth_mac);
  eth_mac->regs->OPERATION_MODE |= (uint32_t)ETH_OPERATION_MODE_ST_Msk;
}

/* Flush RX */
void XMC_ETH_MAC_FlushRx(XMC_ETH_MAC_t *const eth_mac)
{
  eth_mac->regs->OPERATION_MODE &= (uint32_t)~ETH_OPERATION_MODE_SR_Msk;
  XMC_ETH_MAC_InitRxDescriptors(eth_mac);
  eth_mac->regs->OPERATION_MODE |= (uint32_t)ETH_OPERATION_MODE_SR_Msk;
}

/* Set wakeup frame filter */
void XMC_ETH_MAC_SetWakeUpFrameFilter(XMC_ETH_MAC_t *const eth_mac,
                                      const uint32_t (*const filter)[XMC_ETH_WAKEUP_REGISTER_LENGTH])
{
  uint32_t i = 0U;

  /* Fill Remote Wake-up frame filter register with buffer data */
  for (i = 0U; i < XMC_ETH_WAKEUP_REGISTER_LENGTH; i++)
  {
    /* Write each time to the same register */
    eth_mac->regs->REMOTE_WAKE_UP_FRAME_FILTER = (*filter)[i];
  }
}

/* Enable event */
void XMC_ETH_MAC_EnableEvent(XMC_ETH_MAC_t *const eth_mac, uint32_t event)
{
  XMC_ASSERT("XMC_ETH_MAC_EnableDMAEvent: eth_mac is invalid", XMC_ETH_MAC_IsValidModule(eth_mac->regs));

  eth_mac->regs->INTERRUPT_MASK &= ~(event >> 16U);

  event &= (uint16_t)0x7fffU;
  if (XCM_ETH_MAC_IsNormalEvent(event))
  {
	event |= (uint32_t)ETH_INTERRUPT_ENABLE_NIE_Msk;
  }

  if (XCM_ETH_MAC_IsAbnormalEvent(event))
  {
	event |= (uint32_t)ETH_INTERRUPT_ENABLE_AIE_Msk;
  }

  eth_mac->regs->INTERRUPT_ENABLE |= event;
}

/* Disable event */
void XMC_ETH_MAC_DisableEvent(XMC_ETH_MAC_t *const eth_mac, uint32_t event)
{
  XMC_ASSERT("XMC_ETH_MAC_DisableDMAEvent: eth_mac is invalid", XMC_ETH_MAC_IsValidModule(eth_mac->regs));

  eth_mac->regs->INTERRUPT_MASK |= event >> 16U;

  event &= (uint16_t)~0xffffU;
  eth_mac->regs->INTERRUPT_ENABLE &= ~event;
}

/* Clear event status */
void XMC_ETH_MAC_ClearEventStatus(XMC_ETH_MAC_t *const eth_mac, uint32_t event)
{
  XMC_ASSERT("XMC_ETH_MAC_ClearDMAEventStatus: eth_mac is invalid", XMC_ETH_MAC_IsValidModule(eth_mac->regs));

  if ((eth_mac->regs->STATUS & ETH_STATUS_NIS_Msk) != 0U)
  {
	event |= (uint32_t)ETH_STATUS_NIS_Msk;
  }

  if ((eth_mac->regs->STATUS & ETH_STATUS_AIS_Msk) != 0U)
  {
	event |= (uint32_t)ETH_STATUS_AIS_Msk;
  }

  eth_mac->regs->STATUS = event;
}

/* Obtain event status */
uint32_t XMC_ETH_MAC_GetEventStatus(const XMC_ETH_MAC_t *const eth_mac)
{
  uint32_t temp_status = 0;
  XMC_ASSERT("XMC_ETH_MAC_GetDMAEventStatus: eth_mac is invalid", XMC_ETH_MAC_IsValidModule(eth_mac->regs));
  
  temp_status =  (eth_mac->regs->STATUS & (uint32_t)0x7ffUL);

  return ((uint32_t)((eth_mac->regs->INTERRUPT_STATUS & (ETH_INTERRUPT_MASK_PMTIM_Msk | ETH_INTERRUPT_MASK_TSIM_Msk)) << 16U) |
		  temp_status);
}

/* Return RX descriptor */
void XMC_ETH_MAC_ReturnRxDescriptor(XMC_ETH_MAC_t *const eth_mac)
{
  eth_mac->rx_desc[eth_mac->rx_index].status |= ETH_MAC_DMA_RDES0_OWN;
}

/* Return TX descriptor */
void XMC_ETH_MAC_ReturnTxDescriptor(XMC_ETH_MAC_t *const eth_mac)
{
  eth_mac->tx_desc[eth_mac->tx_index].status |= ETH_MAC_DMA_TDES0_OWN;
}

/* Is TX descriptor owned by DMA? */
bool XMC_ETH_MAC_IsTxDescriptorOwnedByDma(XMC_ETH_MAC_t *const eth_mac)
{
  return ((eth_mac->tx_desc[eth_mac->tx_index].status & ETH_MAC_DMA_TDES0_OWN) != 0U);
}

/* Set VLAN tag */
void XMC_ETH_MAC_SetVLANTag(XMC_ETH_MAC_t *const eth_mac, uint16_t tag)
{
  XMC_ASSERT("XMC_ETH_MAC_SetVLANTag: eth_mac is invalid", XMC_ETH_MAC_IsValidModule(eth_mac->regs));

  eth_mac->regs->VLAN_TAG = (uint32_t)tag;
}

/* Initialize PTP */
void XMC_ETH_MAC_InitPTP(XMC_ETH_MAC_t *const eth_mac, uint32_t config)
{
  XMC_ASSERT("XMC_ETH_MAC_InitPTP: eth_mac is invalid", XMC_ETH_MAC_IsValidModule(eth_mac->regs));

  /* Mask the time stamp interrupt */
  eth_mac->regs->INTERRUPT_MASK |= (uint32_t)ETH_INTERRUPT_MASK_TSIM_Msk;

  /* Enable time stamp */
  eth_mac->regs->TIMESTAMP_CONTROL = config | (uint32_t)ETH_TIMESTAMP_CONTROL_TSENA_Msk;

  if ((config & (uint32_t)XMC_ETH_MAC_TIMESTAMP_CONFIG_FINE_UPDATE) != 0U)
  {
	/* Program addend register to obtain fSYS/2 from reference clock (fSYS) */
	eth_mac->regs->TIMESTAMP_ADDEND = (uint32_t)0x80000000U;
	eth_mac->regs->TIMESTAMP_CONTROL |= (uint32_t)ETH_TIMESTAMP_CONTROL_TSADDREG_Msk;

	/* Program sub-second increment register based on PTP clock frequency = fSYS/2 */
	/* the nanoseconds register has a resolution of ~0.465ns. */
	eth_mac->regs->SUB_SECOND_INCREMENT = (uint32_t)((1.0F / (0x80000000U)) * (2.0F / XMC_SCU_CLOCK_GetSystemClockFrequency()));
  }
  else
  {
	/* Program sub-second increment register based on PTP clock frequency = fSYS */
	/* the nanoseconds register has a resolution of ~0.465ns. */
	eth_mac->regs->SUB_SECOND_INCREMENT = (uint32_t)((1.0F / (0x80000000U)) * (1.0F / XMC_SCU_CLOCK_GetSystemClockFrequency()));
  }

  eth_mac->regs->TIMESTAMP_CONTROL |= (uint32_t)ETH_TIMESTAMP_CONTROL_TSINIT_Msk;
}

/* Get PTP time */
void XMC_ETH_MAC_GetPTPTime(XMC_ETH_MAC_t *const eth_mac, XMC_ETH_MAC_TIME_t *const time)
{
  XMC_ASSERT("XMC_ETH_MAC_GetPTPTime: eth_mac is invalid", XMC_ETH_MAC_IsValidModule(eth_mac->regs));

  time->nanoseconds = (uint32_t)(eth_mac->regs->SYSTEM_TIME_NANOSECONDS * (0x80000000U / 1000000000.0F));
  time->seconds = eth_mac->regs->SYSTEM_TIME_SECONDS;
}

/* Update PTP time */
void XMC_ETH_MAC_UpdatePTPTime(XMC_ETH_MAC_t *const eth_mac, const XMC_ETH_MAC_TIME_t *const time)
{
  uint32_t temp;

  XMC_ASSERT("XMC_ETH_MAC_UpdatePTPTime: eth_mac is invalid", XMC_ETH_MAC_IsValidModule(eth_mac->regs));

  temp = (uint32_t)(abs(time->nanoseconds) * (100000000.0F / (0x80000000U)));
  if (time->nanoseconds >= 0)
  {
	temp |= (uint32_t)ETH_SYSTEM_TIME_NANOSECONDS_UPDATE_ADDSUB_Msk;
  }

  eth_mac->regs->SYSTEM_TIME_NANOSECONDS_UPDATE = temp;
  eth_mac->regs->SYSTEM_TIME_SECONDS_UPDATE = time->seconds;

  eth_mac->regs->TIMESTAMP_CONTROL |= (uint32_t)ETH_TIMESTAMP_CONTROL_TSUPDT_Msk;
}

/* Set PTP alarm */
void XMC_ETH_MAC_SetPTPAlarm(XMC_ETH_MAC_t *const eth_mac, const XMC_ETH_MAC_TIME_t *const time)
{
  XMC_ASSERT("XMC_ETH_MAC_SetPTPAlarm: eth_mac is invalid", XMC_ETH_MAC_IsValidModule(eth_mac->regs));

  eth_mac->regs->TARGET_TIME_NANOSECONDS = (uint32_t)(time->nanoseconds * (100000000.0F / (0x80000000U)));
  eth_mac->regs->TARGET_TIME_SECONDS = time->seconds;
}

/* Adjust PTP clock */
void XMC_ETH_MAC_AdjustPTPClock(XMC_ETH_MAC_t *const eth_mac, uint32_t correction)
{
  XMC_ASSERT("XMC_ETH_MAC_AdjustPTPClock: eth_mac is invalid", XMC_ETH_MAC_IsValidModule(eth_mac->regs));

  /* Correction factor is Q31 (0x80000000 = 1.000000000) */
  eth_mac->regs->TIMESTAMP_ADDEND = (uint32_t)(((uint64_t)correction * eth_mac->regs->TIMESTAMP_ADDEND) >> 31U);

  /* Update addend register */
  eth_mac->regs->TIMESTAMP_CONTROL |= (uint32_t)ETH_TIMESTAMP_CONTROL_TSADDREG_Msk;
}

/* Set PTP status */
uint32_t XMC_ETH_MAC_GetPTPStatus(const XMC_ETH_MAC_t *const eth_mac)
{
  XMC_ASSERT("XMC_ETH_MAC_GetPTPStatus: eth_mac is invalid", XMC_ETH_MAC_IsValidModule(eth_mac->regs));

  return (eth_mac->regs->TIMESTAMP_STATUS);
}

/* Get TX time-stamp */
XMC_ETH_MAC_STATUS_t XMC_ETH_MAC_GetRxTimeStamp(XMC_ETH_MAC_t *const eth_mac, XMC_ETH_MAC_TIME_t *const time)
{
  XMC_ETH_MAC_DMA_DESC_t *rx_desc;
  XMC_ETH_MAC_STATUS_t status;

  XMC_ASSERT("XMC_ETH_MAC_GetRxTimeStamp: eth_mac is invalid", XMC_ETH_MAC_IsValidModule(eth_mac->regs));
  XMC_ASSERT("XMC_ETH_MAC_GetRxTimeStamp: time is invalid", time != NULL);

  rx_desc = &eth_mac->rx_desc[eth_mac->rx_index];
  if (rx_desc->status & ETH_MAC_DMA_RDES0_OWN)
  {
    status = XMC_ETH_MAC_STATUS_BUSY;
  }
  else
  {
    time->nanoseconds = (int32_t)rx_desc->time_stamp_nanoseconds;
    time->seconds = rx_desc->time_stamp_seconds;

    status = XMC_ETH_MAC_STATUS_OK;
  }

  return status;
}

/* Get TX time-stamp */
XMC_ETH_MAC_STATUS_t XMC_ETH_MAC_GetTxTimeStamp(XMC_ETH_MAC_t *const eth_mac, XMC_ETH_MAC_TIME_t *const time)
{
  XMC_ETH_MAC_DMA_DESC_t *tx_desc;
  XMC_ETH_MAC_STATUS_t status;

  XMC_ASSERT("XMC_ETH_MAC_GetTxTimeStamp: eth_mac is invalid", XMC_ETH_MAC_IsValidModule(eth_mac->regs));
  XMC_ASSERT("XMC_ETH_MAC_GetTxTimeStamp: time is invalid", time != NULL);

  tx_desc = &eth_mac->tx_desc[eth_mac->tx_ts_index];
  if (tx_desc->status & ETH_MAC_DMA_TDES0_OWN)
  {
    status = XMC_ETH_MAC_STATUS_BUSY;
  }
  else
  {
    time->nanoseconds = (int32_t)tx_desc->time_stamp_nanoseconds;
    time->seconds = tx_desc->time_stamp_seconds;

    status = XMC_ETH_MAC_STATUS_OK;
  }

  return status;
}

#endif
