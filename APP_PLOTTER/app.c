/**
 * \file app.c
 *
 * \mainpage Application for UART communication tests
 *
 * Build: make debug OR make flash
 * Connect a TTL USB cable to UART1, launch a terminal program and initiate
 * communication, e.g. (without the quotes):
 *         PC -> uC: "#50$"
 *         uC -> PC: "DUC: 50%"
 *
 * @author Thomas Carli
 */

/******************************************************************* INCLUDES */
#include <app_cfg.h>
#include <cpu_core.h>
#include <os.h>

#include <bsp.h>
#include <bsp_sys.h>
#include <bsp_int.h>
#include <bsp_ccu4.h>
#include <bsp_ccu8.h>
#include <bsp_dac.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>

#include <mcp23s08_drv.h>
#include <mcp3004_drv.h>

#include <xmc_uart.h>
#include <xmc_gpio.h>

#include <GPIO.h>

#include <errno.h>
#include <limits.h>
#include <lib_math.h>

#if SEMI_HOSTING
#include <debug_lib.h>
#endif

#if JLINK_RTT
#include <SEGGER_RTT.h>
#include <SEGGER_RTT_Conf.h>
#endif

/******************************************************************** DEFINES */
#define DEBUG_DIM 0
#define ACK  0x6
#define MAX_MSG_LENGTH 40
#define NUM_MSG        3

/********************************************************* FILE LOCAL GLOBALS */
static  CPU_STK  AppStartTaskStk[APP_CFG_TASK_START_STK_SIZE];            // <1>
static  OS_TCB   AppStartTaskTCB;

static  CPU_STK  AppTaskPlotStk[APP_CFG_TASK_PLOT_STK_SIZE];
static  OS_TCB   AppTaskPlotTCB;

static  CPU_STK  AppTaskComStk[APP_CFG_TASK_COM_STK_SIZE];
static  OS_TCB   AppTaskComTCB;

static  CPU_STK  AppTaskXYTestStk[APP_CFG_TASK_XYTEST_STK_SIZE];
static  OS_TCB   AppTaskXYTestTCB;

// Memory Block                                                           // <2>
OS_MEM      Mem_Partition;
CPU_CHAR    MyPartitionStorage[NUM_MSG - 1][MAX_MSG_LENGTH];
// Message Queue
OS_Q        UART_ISR;
OS_Q        DUTY_QUEUE;
// OS_Q        START_QUEUE;
// OS_SEM      XYTEST_SEM;

/****************************************************** FILE LOCAL PROTOTYPES */
static  void AppTaskStart (void  *p_arg);
static  void AppTaskCreate (void);
static  void AppObjCreate (void);
static  void AppTaskCom (void  *p_arg);
static  void AppTaskPlot (void  *p_arg);
// static  void AppTaskXYTest (void  *p_arg);

void SendAcknowledge(uint8_t cmd);

uint8_t dir = 0;

/************************************************************ FUNCTIONS/TASKS */

/*********************************************************************** MAIN */
/**
 * \function main
 * \params none
 * \returns 0 always
 *
 * \brief This is the standard entry point for C code.
 */

int main (void)
{
  OS_ERR  err;

  // Disable all interrupts                                               // <3>
  BSP_IntDisAll();
  // Enable Interrupt UART
  BSP_IntEn (BSP_INT_ID_USIC1_01); //**
  BSP_IntEn (BSP_INT_ID_USIC1_00); //**
  BSP_IntEn (BSP_INT_ID_CCU40_00); //** PORT 1.3
// init SEMI Hosting DEBUG Support                                        // <4>
#if SEMI_HOSTING
  initRetargetSwo();
#endif

// init JLINK RTT DEBUG Support
#if JLINK_RTT
  SEGGER_RTT_ConfigDownBuffer (0, NULL, NULL, 0,
             SEGGER_RTT_MODE_BLOCK_IF_FIFO_FULL);
  SEGGER_RTT_ConfigUpBuffer (0, NULL, NULL, 0,
           SEGGER_RTT_MODE_BLOCK_IF_FIFO_FULL);
#endif

  // Init uC/OS-III
  OSInit (&err);                                                          // <5>
  if (err != OS_ERR_NONE)
    APP_TRACE_DBG ("Error OSInit: main\n");

  /* Create the start task */                                             // <6>
  OSTaskCreate ( (OS_TCB     *) &AppStartTaskTCB,
           (CPU_CHAR   *) "Startup Task",
           (OS_TASK_PTR) AppTaskStart,
           (void       *) 0,
           (OS_PRIO) APP_CFG_TASK_START_PRIO,
           (CPU_STK    *) &AppStartTaskStk[0],
           (CPU_STK_SIZE) APP_CFG_TASK_START_STK_SIZE / 10u,
           (CPU_STK_SIZE) APP_CFG_TASK_START_STK_SIZE,
           (OS_MSG_QTY) 0u,
           (OS_TICK) 0u,
           (void       *) 0,
           (OS_OPT) (OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR),
           (OS_ERR     *) &err);

  // Start multitasking (i.e., give control to uC/OS-III)

  OSStart (&err);                                                         // <7>
  if (err != OS_ERR_NONE)
    APP_TRACE_DBG ("Error OSStart: main\n");

  while (1) {                                                             // <8>
    APP_TRACE_DBG ("Should never be output! Bug?\n");
  }
  return 0;
}

/*************************************************************** STARTUP TASK */
/**
 * \function AppTaskStart
 * \ params p_arg ... argument passed to AppTaskStart() by
 *                    OSTaskCreate()
 * \returns none
 *
 * \brief Startup (init) task that loads board support functions,
 *        initializes CPU services, the memory, the systick timer,
 *        etc. and finally invokes other application tasks.
 */
static void AppTaskStart (void *p_arg)
{
  CPU_INT32U  cpu_clk_freq;
  CPU_INT32U  cnts;
  OS_ERR      err;

  (void) p_arg;
  // initialize BSP functions
  BSP_Init();                                                             // <9>
  // initialize the uC/CPU services
  CPU_Init();
  // determine SysTick reference frequency
  cpu_clk_freq = BSP_SysClkFreqGet();
  // determine nbr SysTick increments
  cnts = cpu_clk_freq / (CPU_INT32U) OSCfg_TickRate_Hz;
  // init uCOS-III periodic time src (SysTick)
  OS_CPU_SysTickInit (cnts);
  // initialize memory management module
  Mem_Init();
  // initialize mathematical module
  Math_Init();

// compute CPU capacity with no task running
#if (OS_CFG_STAT_TASK_EN > 0u)                                           // <10>
  OSStatTaskCPUUsageInit (&err);
  if (err != OS_ERR_NONE)
    APP_TRACE_DBG ("Error OSStatTaskCPUUsageInit: AppTaskStart\n");
#endif

  APP_TRACE_INFO ("Creating Application Objects...\n");                  // <11>
  // create application objects
  AppObjCreate();

  APP_TRACE_INFO ("Creating Application Tasks...\n");                    // <12>
  // create application tasks
  AppTaskCreate();

  while (DEF_TRUE) {                                                     // <13>
    // Suspend current task
    OSTaskSuspend ( (OS_TCB *) 0, &err);
    if (err != OS_ERR_NONE)
      APP_TRACE_DBG ("Error OSTaskSuspend: AppTaskStart\n");
  }
}

/************************************************* Create Application Objects */
/**
 * \function AppObjCreate()
 * \brief Creates application objects.
 * \params none
 * \returns none
 */
static void AppObjCreate (void)
{
  OS_ERR      err;

  // Create Shared Memory
  OSMemCreate ( (OS_MEM    *) &Mem_Partition,
          (CPU_CHAR  *) "Mem Partition",
          (void      *) &MyPartitionStorage[0][0],
          (OS_MEM_QTY)  NUM_MSG,
          (OS_MEM_SIZE) MAX_MSG_LENGTH * sizeof (CPU_CHAR),
          (OS_ERR    *) &err);
  if (err != OS_ERR_NONE)
    APP_TRACE_DBG ("Error OSMemCreate: AppObjCreate\n");

  // Create Message Queue
  // You may re-use code fragments of the bare-bone IO flat exa
  OSQCreate ( (OS_Q *)     &UART_ISR,
        (CPU_CHAR *) "ISR Queue",
        (OS_MSG_QTY) NUM_MSG,
        (OS_ERR   *) &err);
  if (err != OS_ERR_NONE)
    APP_TRACE_DBG ("ErroBSP_PWM_SetDutyCycler OSQCreate: AppObjCreate\n");

  OSQCreate ( (OS_Q *)     &DUTY_QUEUE,
        (CPU_CHAR *) "DUTY Queue",
        (OS_MSG_QTY) NUM_MSG,
        (OS_ERR   *) &err);
  if (err != OS_ERR_NONE)
    APP_TRACE_DBG ("Error OSQCreate: AppObjCreate\n");

  // OSQCreate ( (OS_Q *)     &START_QUEUE,
  //       (CPU_CHAR *) "START Queue",
  //       (OS_MSG_QTY) NUM_MSG,
  //       (OS_ERR   *) &err);
  // if (err != OS_ERR_NONE)
  //   APP_TRACE_DBG ("Error OSQCreate: AppObjCreate\n");

  // OSSemCreate(&XYTEST_SEM, "XYTest_sem", 0, &err);
  // if (err != OS_ERR_NONE)
  //     APP_TRACE_DBG ("Error OSQCreate: AppObjCreate\n");
}

/*************************************************** Create Application Tasks */
/**
 * \function AppTaskCreate()
 * \brief Creates one application task.
 * \params none
 * \returns none
 */
static void  AppTaskCreate (void)
{
  OS_ERR      err;

  // create AppTask_COM
  OSTaskCreate ( (OS_TCB     *) &AppTaskComTCB,
           (CPU_CHAR   *) "TaskCOM",
           (OS_TASK_PTR) AppTaskCom,
           (void       *) 0,
           (OS_PRIO) APP_CFG_TASK_COM_PRIO,
           (CPU_STK    *) &AppTaskComStk[0],
           (CPU_STK_SIZE) APP_CFG_TASK_COM_STK_SIZE / 10u,
           (CPU_STK_SIZE) APP_CFG_TASK_COM_STK_SIZE,
           (OS_MSG_QTY) 0u,
           (OS_TICK) 0u,
           (void       *) 0,
           (OS_OPT) (OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR),
           (OS_ERR     *) &err);
  if (err != OS_ERR_NONE)
  {
    APP_TRACE_DBG ("Error OSTaskCreate: AppTaskCreate\n");
  }

  OSTaskCreate ( (OS_TCB     *) &AppTaskPlotTCB,
           (CPU_CHAR   *) "TaskPlot",
           (OS_TASK_PTR) AppTaskPlot,
           (void       *) 0,
           (OS_PRIO) APP_CFG_TASK_PLOT_PRIO,
           (CPU_STK    *) &AppTaskPlotStk[0],
           (CPU_STK_SIZE) APP_CFG_TASK_PLOT_STK_SIZE / 10u,
           (CPU_STK_SIZE) APP_CFG_TASK_PLOT_STK_SIZE,
           (OS_MSG_QTY) 0u,
           (OS_TICK) 0u,
           (void       *) 0,
           (OS_OPT) (OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR),
           (OS_ERR     *) &err);
  if (err != OS_ERR_NONE)
    APP_TRACE_DBG ("Error OSTaskCreate: AppTaskCreate(LED)\n");

/* --> Enable to count the steps for the xy-axis
      OSTaskCreate ( (OS_TCB     *) &AppTaskXYTestTCB,
               (CPU_CHAR   *) "TestTaskXY",
               (OS_TASK_PTR) AppTaskXYTest,
               (void       *) 0,
               (OS_PRIO) APP_CFG_TASK_XYTEST_PRIO,
               (CPU_STK    *) &AppTaskXYTestStk[0],
               (CPU_STK_SIZE) APP_CFG_TASK_XYTEST_STK_SIZE / 10u,
               (CPU_STK_SIZE) APP_CFG_TASK_XYTEST_STK_SIZE,
               (OS_MSG_QTY) 0u,
               (OS_TICK) 0u,
               (void       *) 0,
               (OS_OPT) (OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR),
               (OS_ERR     *) &err);
      if (err != OS_ERR_NONE)
        APP_TRACE_DBG ("Error OSTaskCreate: AppTaskCreate(LED)\n");
*/
}

/*********************************************************************** AppTaskXYTest */
/**
 * \function XYTEst
 * \params none
 * \returns 0 always
 *
 * \brief This is the code to test the Plotter and to count the steps for the dimension of the XY-Axis.
 */
/* Enable for testing
void AppTaskXYTest(void *p_arg)
{
    static int lowhigh = 0;
    int end1, end2, end3, end4;
    int steps = 0;
    int rounds = 0;
    int timer = 0;
    while(DEF_TRUE)
    {
      while(timer < 255)
      {
        timer++;
      }
      timer = 0;
      if(XMC_GPIO_GetInput(D5) == 1)  // y-Axis minus
        end1 = 1;
      else
        end1 = 0;

      if(XMC_GPIO_GetInput(D6) == 1)  // y-Axis plus
        end2 = 1;
      else
        end2 = 0;

      if(XMC_GPIO_GetInput(D7) == 1)  // x-Axis minus
        end3 = 1;
      else
        end3 = 0;

      if(XMC_GPIO_GetInput(D8) == 1)  // x-Axis plus
        end4 = 1;
      else
        end4 = 0;

      // Count the steps for each direction of the XY-Axis
      if((dir == 0)||(dir == 1)||(dir == 2)||(dir == 3))
      {

        // if(steps <= 200)
        //   steps++;
        // else
        // {
        //   steps = 0;
        //   rounds++;
        // }

        if(dir == 0) // y- minus
        {
          if(end1 == 1)
          {
            steps++;
            if(lowhigh == 0)
            {
              lowhigh = 1;
              _mcp23s08_reset_ss(MCP23S08_SS);
              _mcp23s08_reg_xfer(XMC_SPI1_CH0,MCP23S08_GPIO,Y_MINUS_PLOT_LOW,MCP23S08_WR);
              _mcp23s08_set_ss(MCP23S08_SS);
            }
            else
            {
              lowhigh = 0;
              _mcp23s08_reset_ss(MCP23S08_SS);
              _mcp23s08_reg_xfer(XMC_SPI1_CH0,MCP23S08_GPIO,Y_MINUS_PLOT_HIGH,MCP23S08_WR);
              _mcp23s08_set_ss(MCP23S08_SS);
            }
          }
          else
          {
            dir = 1;
            steps = 0;
          }
        }
        if(dir == 1) // y- plus
        {
          steps++;
          if(end2 == 1)
          {
            if(lowhigh == 0)
            {
              lowhigh = 1;
              _mcp23s08_reset_ss(MCP23S08_SS);
              _mcp23s08_reg_xfer(XMC_SPI1_CH0,MCP23S08_GPIO,Y_PLUS_PLOT_LOW,MCP23S08_WR);
              _mcp23s08_set_ss(MCP23S08_SS);
            }
            else
            {
              lowhigh = 0;
              _mcp23s08_reset_ss(MCP23S08_SS);
              _mcp23s08_reg_xfer(XMC_SPI1_CH0,MCP23S08_GPIO,Y_PLUS_PLOT_HIGH,MCP23S08_WR);
              _mcp23s08_set_ss(MCP23S08_SS);
            }
          }
          else
          {
            dir = 2;
            steps = 0;
          }
        }
        if(dir == 2) // x- minus
        {
          if(end3 == 1)
          {
            steps++;
            if(lowhigh == 0)
            {
              lowhigh = 1;
              _mcp23s08_reset_ss(MCP23S08_SS);
              _mcp23s08_reg_xfer(XMC_SPI1_CH0,MCP23S08_GPIO,X_MINUS_PLOT_LOW,MCP23S08_WR);
              _mcp23s08_set_ss(MCP23S08_SS);
            }
            else
            {
              lowhigh = 0;
              _mcp23s08_reset_ss(MCP23S08_SS);
              _mcp23s08_reg_xfer(XMC_SPI1_CH0,MCP23S08_GPIO,X_MINUS_PLOT_HIGH,MCP23S08_WR);
              _mcp23s08_set_ss(MCP23S08_SS);
            }
          }
          else
          {
            dir = 3;
            steps = 0;
          }
        }
        if(dir == 3) // x- plus
        {
          if(end4 == 1)
          {
            steps++;
            if(lowhigh == 0)
            {
              lowhigh = 1;
              _mcp23s08_reset_ss(MCP23S08_SS);
              _mcp23s08_reg_xfer(XMC_SPI1_CH0,MCP23S08_GPIO,X_PLUS_PLOT_LOW,MCP23S08_WR);
              _mcp23s08_set_ss(MCP23S08_SS);
            }
            else
            {
              lowhigh = 0;
              _mcp23s08_reset_ss(MCP23S08_SS);
              _mcp23s08_reg_xfer(XMC_SPI1_CH0,MCP23S08_GPIO,X_PLUS_PLOT_HIGH,MCP23S08_WR);
              _mcp23s08_set_ss(MCP23S08_SS);
            }
          }
          else
          {
            dir = 5;
            steps = 0;
          }
        }
      }
    }
  }
*/

void SendAcknowledge(uint8_t cmd)
{
  switch(cmd)
  {
    case 1:
            XMC_UART_CH_Transmit (XMC_UART1_CH1, 'D');
            XMC_UART_CH_Transmit (XMC_UART1_CH1, 'O');
            XMC_UART_CH_Transmit (XMC_UART1_CH1, 'N');
            XMC_UART_CH_Transmit (XMC_UART1_CH1, 'E');
            XMC_UART_CH_Transmit (XMC_UART1_CH1, '\n');
            break;
    case 3:
            XMC_UART_CH_Transmit (XMC_UART1_CH1, 'E');
            XMC_UART_CH_Transmit (XMC_UART1_CH1, 'R');
            XMC_UART_CH_Transmit (XMC_UART1_CH1, 'R');
            XMC_UART_CH_Transmit (XMC_UART1_CH1, '\n');
            break;
    default:
            XMC_UART_CH_Transmit (XMC_UART1_CH1, 'E');
            XMC_UART_CH_Transmit (XMC_UART1_CH1, 'R');
            XMC_UART_CH_Transmit (XMC_UART1_CH1, 'R');
            XMC_UART_CH_Transmit (XMC_UART1_CH1, '\n');
            break;
  }
}

void AppTaskPlot(void *p_arg)
{
  void        *errmem = NULL;
  OS_ERR      err;
  CPU_TS      ts;
  void        *p_msg;
  OS_MSG_SIZE msg_size;
  uint16_t    dutycycle;
  char        data[MAX_MSG_LENGTH];
  char        *pEnd;
  _Bool       ret = false;
  uint8_t reg_val = 0;
  char  compG00[] = "G00";
  char  compG01[] = "G01";
  char  compG28[] = "G28";
  uint8_t   ack = 3;
  uint8_t   countsteps = 255;
  uint8_t   dir_x = 0x00;
  uint8_t   dir_y = 0x00;
  uint8_t   dir_xy = 0x00;
  uint8_t   x_prop = 0;
  uint8_t   y_prop = 0;
  uint8_t   x_prop_count = 0;
  uint8_t   y_prop_count = 0;
  int   x_new = 0;
  int   y_new = 0;
  int   x_axis_mov = 0;
  int   x_axis_curr = 0;
  int   x_axis_end = 0;
  int   y_axis_mov = 0;
  int   y_axis_curr = 0;
  int   y_axis_end = 0;

  // PEN UP and move to the 0-0-pos
  /**/
  BSP_PWM_SetPen(1);
  OSTimeDlyHMSM(0,0,0,100,OS_OPT_TIME_HMSM_STRICT, &err);
  if(err != OS_ERR_NONE)
  {
    SendAcknowledge(ack);
    ack = 3;
    APP_TRACE_DBG ("Error TimeDelay: AppTaskPlot\n");
  }
  // Set the output of the pen OFF
  BSP_PWM_SetPen(3);
  while(XMC_GPIO_GetInput(D6)||XMC_GPIO_GetInput(D7))
  {
    dir_xy = 0x00;
    _mcp23s08_reset_ss(MCP23S08_SS);
    _mcp23s08_reg_xfer(XMC_SPI1_CH0,MCP23S08_GPIO,dir_xy,MCP23S08_WR);
    _mcp23s08_set_ss(MCP23S08_SS);
    OSTimeDly(0.5, OS_OPT_TIME_DLY, &err);

    if(XMC_GPIO_GetInput(D7))
      dir_xy = dir_xy | X_MINUS_PLOT_HIGH;
    if(XMC_GPIO_GetInput(D6))
      dir_xy = dir_xy | Y_MINUS_PLOT_HIGH;
    _mcp23s08_reset_ss(MCP23S08_SS);
    _mcp23s08_reg_xfer(XMC_SPI1_CH0,MCP23S08_GPIO,dir_xy,MCP23S08_WR);
    _mcp23s08_set_ss(MCP23S08_SS);
    OSTimeDly(0.5, OS_OPT_TIME_DLY, &err);
  }
  while(DEF_TRUE)
  {
    ack = 2;
    errno = 0;
    errmem = memset(&data[0], 0, MAX_MSG_LENGTH);
    if(errmem != &data)
    {
      SendAcknowledge(ack);
      ack = 3;
      APP_TRACE_DBG ("Error TimeDelay: AppTaskPlot\n");
    }

    p_msg = OSQPend (&DUTY_QUEUE,                                          // <16>
          0,
          OS_OPT_PEND_BLOCKING,
          &msg_size,
          &ts,
          &err);
    if (err != OS_ERR_NONE)
    {
      SendAcknowledge(ack);
      ack = 3;
      APP_TRACE_DBG ("Error TimeDelay: AppTaskPlot\n");
    }

    if(msg_size < 40)
      errmem = memcpy (data, p_msg, msg_size - 1);
    else
    {
      SendAcknowledge(ack);
      ack = 3;
      APP_TRACE_DBG ("Error TimeDelay: AppTaskPlot\n");
    }
    if(errmem != &data)
    {
      SendAcknowledge(ack);
      ack = 3;
      APP_TRACE_DBG ("Error TimeDelay: AppTaskPlot\n");
    }

    char *token = strtok(data, " ");

    if((strncmp(data, compG28, 3) == 0)&&(ack==2)) // Go to homeposition
    {
      ack = 1;
      // Reset x- and y- axisv
      x_axis_curr = 0;
      y_axis_curr = 0;
      x_axis_end = 0;
      y_axis_end = 0;
      x_new = 0;
      y_new = 0;
      // PEN UP and move to the 0-0-pos
      BSP_PWM_SetPen(1);
      OSTimeDlyHMSM(0,0,0,100,OS_OPT_TIME_HMSM_STRICT, &err);
      if(err != OS_ERR_NONE)
      {
        SendAcknowledge(ack);
        ack = 3;
        APP_TRACE_DBG ("Error TimeDelay: AppTaskPlot\n");
      }
      // Set the output of the pen OFF
      BSP_PWM_SetPen(3);
      while((XMC_GPIO_GetInput(D6)||XMC_GPIO_GetInput(D7))&&(ack==1))
      {
        dir_xy = 0x00;
        _mcp23s08_reset_ss(MCP23S08_SS);
        _mcp23s08_reg_xfer(XMC_SPI1_CH0,MCP23S08_GPIO,dir_xy,MCP23S08_WR);
        _mcp23s08_set_ss(MCP23S08_SS);

        while(countsteps!=0)
          countsteps--;
        countsteps = 255;

        if(XMC_GPIO_GetInput(D7))
          dir_xy = dir_xy | X_MINUS_PLOT_HIGH;
        if(XMC_GPIO_GetInput(D5))
          dir_xy = dir_xy | Y_MINUS_PLOT_HIGH;
        _mcp23s08_reset_ss(MCP23S08_SS);
        _mcp23s08_reg_xfer(XMC_SPI1_CH0,MCP23S08_GPIO,dir_xy,MCP23S08_WR);
        _mcp23s08_set_ss(MCP23S08_SS);
      }
    }
    else
    {
      if(strncmp(data, compG00, 3) == 0)           // timer on and pen UP
      {
        ret = BSP_PWM_SetPen(1);
        ack = 1;
      }
      if(strncmp(data, compG01, 3) == 0)           // timer on and pen DOWN
      {
        ack = 1;
        ret = BSP_PWM_SetPen(2);
      }
      if(!ret)
      {
        SendAcknowledge(ack);
        ack = 3;
        APP_TRACE_DBG ("Error TimeDelay: AppTaskPlot\n");
      }
      // Wait 100ms to move the PEN
      OSTimeDlyHMSM(0,0,0,100,OS_OPT_TIME_HMSM_STRICT, &err);
      if(err != OS_ERR_NONE)
      {
        SendAcknowledge(ack);
        ack = 3;
        APP_TRACE_DBG ("Error TimeDelay: AppTaskPlot\n");
      }
      // Set the output of the pen OFF
      ret = BSP_PWM_SetPen(3);
      if(!ret)
      {
        SendAcknowledge(ack);
        ack = 3;
        APP_TRACE_DBG ("Error TimeDelay: AppTaskPlot\n");
      }
      if(ack == 1)
      {
        // Wait 500ms to move the PEN
        OSTimeDlyHMSM(0,0,0,500,OS_OPT_TIME_HMSM_STRICT, &err);

        token = strtok(NULL, " ");
        x_axis_end = strtol(token, &pEnd,10);
        token = strtok(NULL, " ");
        y_axis_end = strtol(token, &pEnd,10);
        if((x_axis_end - x_axis_curr)==0) // next point is equally
          x_axis_mov = 0;
        if((x_axis_end - x_axis_curr)<0)  // go in direction left --> minus
        {
          x_axis_mov = -1;
          dir_x = X_MINUS_PLOT_HIGH;
        }
        if((x_axis_end - x_axis_curr)>0)  // go in direction right --> plus
        {
          x_axis_mov = 1;
          dir_x = X_PLUS_PLOT_HIGH;
        }
        if((y_axis_end - y_axis_curr)==0) // next point is equally
          y_axis_mov = 0;
        if((y_axis_end - y_axis_curr)<0)  // go "up" --> minus
        {
          y_axis_mov = -1;
          dir_y = Y_MINUS_PLOT_HIGH;
        }
        if((y_axis_end - y_axis_curr)>0)  // go "down" --> plus
        {
          y_axis_mov = 1;
          dir_y = Y_PLUS_PLOT_HIGH;
        }
        if(((x_axis_mov==0)&&(y_axis_mov!=0))||((x_axis_mov!=0)&&(y_axis_mov==0)))
        {
          // multiply number of g-code 200
          // move pen x axis
          while(x_axis_mov!=0)
          {
            if(((XMC_GPIO_GetInput(D7) == 0)&&(x_axis_mov==-1))||((XMC_GPIO_GetInput(D8) == 0)&&(x_axis_mov==1))||(x_axis_curr == x_axis_end))
              break;
            x_axis_curr+=x_axis_mov;
            _mcp23s08_reset_ss(MCP23S08_SS);
            _mcp23s08_reg_xfer(XMC_SPI1_CH0,MCP23S08_GPIO,0x00,MCP23S08_WR);
            _mcp23s08_set_ss(MCP23S08_SS);
            OSTimeDly(0.5, OS_OPT_TIME_DLY, &err);

            _mcp23s08_reset_ss(MCP23S08_SS);
            _mcp23s08_reg_xfer(XMC_SPI1_CH0,MCP23S08_GPIO,dir_x,MCP23S08_WR);
            _mcp23s08_set_ss(MCP23S08_SS);
            OSTimeDly(0.5, OS_OPT_TIME_DLY, &err);
          }
          // move pen y axis
          while(y_axis_mov!=0)
          {
            if(((XMC_GPIO_GetInput(D6) == 0)&&(y_axis_mov==-1))||((XMC_GPIO_GetInput(D5) == 0)&&(y_axis_mov==1))||(y_axis_curr == y_axis_end))//(y_axis_curr == y_axis_end)//
              break;
            y_axis_curr+=y_axis_mov;
            _mcp23s08_reset_ss(MCP23S08_SS);
            _mcp23s08_reg_xfer(XMC_SPI1_CH0,MCP23S08_GPIO,0x00,MCP23S08_WR);
            _mcp23s08_set_ss(MCP23S08_SS);
            OSTimeDly(0.5, OS_OPT_TIME_DLY, &err);

            _mcp23s08_reset_ss(MCP23S08_SS);
            _mcp23s08_reg_xfer(XMC_SPI1_CH0,MCP23S08_GPIO,dir_y,MCP23S08_WR);
            _mcp23s08_set_ss(MCP23S08_SS);
            OSTimeDly(0.5, OS_OPT_TIME_DLY, &err);
          }
        }
        else
        {
          /**/
          int temp_x = 0;
          int temp_y = 0;
          x_new = abs(x_axis_end - x_axis_curr);
          y_new = abs(y_axis_end - y_axis_curr);
          if(x_new > y_new)
          {
            y_prop = round((float)x_new / (float)y_new);
            x_prop = 1;
          }
          if(y_new > x_new)
          {
            x_prop = round((float)y_new / (float)x_new);
            y_prop = 1;
          }
          if(x_new ==  y_new)
          {
            x_prop = 1;
            y_prop = 1;
          }
          while(1)
          {
            dir_xy = 0;
            if((XMC_GPIO_GetInput(D8) == 1)&&(x_axis_curr != x_axis_end)&&(x_axis_mov==1))
            {
              dir_xy = dir_xy | X_PLUS_PLOT_HIGH;
            }
            if((XMC_GPIO_GetInput(D7) == 1)&&(x_axis_curr != x_axis_end)&&(x_axis_mov==-1))
            {
              dir_xy = dir_xy | X_MINUS_PLOT_HIGH;
            }
            if((XMC_GPIO_GetInput(D5) == 1)&&(y_axis_curr != y_axis_end)&&(y_axis_mov==1))
            {
              dir_xy = dir_xy | Y_PLUS_PLOT_HIGH;
            }
            if((XMC_GPIO_GetInput(D6) == 1)&&(y_axis_curr != y_axis_end)&&(y_axis_mov==-1))
            {
              dir_xy = dir_xy | Y_MINUS_PLOT_HIGH;
            }
            if((((XMC_GPIO_GetInput(D8) == 0)&&(XMC_GPIO_GetInput(D6) == 0))||((x_axis_curr == x_axis_end)&&(y_axis_curr == y_axis_end)))&&((x_axis_mov==1)&&(y_axis_mov==-1)))
              break;
            if((((XMC_GPIO_GetInput(D8) == 0)&&(XMC_GPIO_GetInput(D5) == 0))||((x_axis_curr == x_axis_end)&&(y_axis_curr == y_axis_end)))&&((x_axis_mov==1)&&(y_axis_mov==1)))
              break;
            if((((XMC_GPIO_GetInput(D7) == 0)&&(XMC_GPIO_GetInput(D6) == 0))||((x_axis_curr == x_axis_end)&&(y_axis_curr == y_axis_end)))&&((x_axis_mov==-1)&&(y_axis_mov==-1)))
              break;
            if((((XMC_GPIO_GetInput(D7) == 0)&&(XMC_GPIO_GetInput(D5) == 0))||((x_axis_curr == x_axis_end)&&(y_axis_curr == y_axis_end)))&&((x_axis_mov==-1)&&(y_axis_mov==1)))
              break;

           if(x_axis_curr != x_axis_end)
           {
              if((x_prop == 1)&&(x_axis_mov==1))
                x_axis_curr+=1;
              else
              {
                if((x_prop == 1)&&(x_axis_mov==-1))
                  x_axis_curr-=1;
                else
                {
                  x_prop_count++;
                  if(x_prop_count == x_prop)
                  {
                    /* Neue Funktion, Verhältnis wird immer neu berechnet*/
                    x_new = abs(x_axis_end-x_axis_curr);
                    y_new = abs(y_axis_end-y_axis_curr);
                    temp_x = round((float)y_new / (float)x_new);
                    if(temp_x != x_prop)
                      x_prop = temp_x;
/***                /****************************************************/
                    x_prop_count = 0;
                    if(x_axis_mov==-1)
                      x_axis_curr-=1;
                    else
                      x_axis_curr+=1;
                  }
                  else
                  {
                    if(x_axis_mov==-1)
                      dir_xy = dir_xy & 0xf3; // set impulse of X_MINUS_PLOT_HIGH to 0
                    else if(x_axis_mov==1)
                      dir_xy = dir_xy & 0xf7; // set impulse of X_PLUS_PLOT_HIGH to 0
                  }
                }
              }
             }
           else
            dir_xy = dir_xy & 0xf3;
           if(y_axis_curr != y_axis_end)
           {
                if((y_prop == 1)&&(y_axis_mov==1))
                  y_axis_curr+=1;
                else
                {
                  if((y_prop == 1)&&(y_axis_mov==-1))
                    y_axis_curr-=1;
                  else
                  {
                    y_prop_count++;
                    if(y_prop_count == y_prop)
                    {
                      /* Neue Funktion, Verhältnis wird immer neu berechnet*/
                      x_new = abs(x_axis_end-x_axis_curr);
                      y_new = abs(y_axis_end-y_axis_curr);
                      temp_y = round((float)x_new / (float)y_new);
                      if(temp_y != y_prop)
                        y_prop = temp_y;
                      /*****************************************************/
                      y_prop_count = 0;
                      if(y_axis_mov == -1)
                        y_axis_curr -= 1;
                      else
                      y_axis_curr+=1;
                    }
                    else
                    {
                      if(y_axis_mov==-1)
                        dir_xy = dir_xy & 0xfc; // set impulse of Y_MINUS_PLOT_HIGH to 0
                      else if(y_axis_mov==1)
                        dir_xy = dir_xy & 0xfd; // set impulse of Y_PLUS_PLOT_HIGH to 0
                    }
                  }
                }
             }
           else
            dir_xy = dir_xy & 0xfc;

            //temp = round(y_prop_count*x_prop);
            _mcp23s08_reset_ss(MCP23S08_SS);
            _mcp23s08_reg_xfer(XMC_SPI1_CH0,MCP23S08_GPIO,0x00,MCP23S08_WR);
            _mcp23s08_set_ss(MCP23S08_SS);

            while(countsteps!=0)
              countsteps--;
            countsteps = 255;

            _mcp23s08_reset_ss(MCP23S08_SS);
            _mcp23s08_reg_xfer(XMC_SPI1_CH0,MCP23S08_GPIO,dir_xy,MCP23S08_WR);
            _mcp23s08_set_ss(MCP23S08_SS);
          }
        }
      }
    }
    SendAcknowledge(ack);
    APP_TRACE_DBG ("Led Task\n");
  }
}

/*********************************** Communication Application Task */
/**
 * \function AppTaskCom
 * \ params p_arg ... argument passed to AppTaskCom() by
 *                    AppTaskCreate()
 * \returns none
 *
 * \brief Task for Communication between UART_ISR (see BSP/bsp_int.c)
 *        and AppTaskCom. Communication from the ISR to the AppTaskCom is
 *        facilitated using a message queue.
 *
 *        Debug trace mesages are output to the SEGGER J-Link GDB Server.
 *
 *        (1) Debug or Flash the application.
 *        (2) Connect a TTL-USB UART cable:// set highbyte 0
 *            GND (BLACK) - GND, TX (GREEN) - P0.0, RX (WHITE) - P0.1
 *        (3) Launch a terminal program and connect with 9600-8N1
 *            Enter strings like: #12345$, #abc$, etc.
 *            The XMC will respond with: XMC: 12345, XMC: abc, etc.
 */
static void AppTaskCom (void *p_arg)
{
  void        *errmem = NULL;
  void        *p_msg;
  OS_ERR      err;
  OS_MSG_SIZE msg_size;
  CPU_TS      ts;
  CPU_CHAR    msg[MAX_MSG_LENGTH];
  CPU_INT08U  i = 0;
  CPU_CHAR    debug_msg[MAX_MSG_LENGTH + 30];
  CPU_CHAR    CommRxBuf[MAX_MSG_LENGTH];
  (void) p_arg;                                                          // <14>
  APP_TRACE_INFO ("Entering AppTaskCom ...\n");
  while (DEF_TRUE) {
    // empty the message buffer
    errmem = memset (&msg[0], 0, MAX_MSG_LENGTH);
    if(errmem != &msg)
      APP_TRACE_DBG ("Error memset: AppTaskCom\n");
    // wait until a message is received
    p_msg = OSQPend (&UART_ISR,                                          // <16>
         0,
         OS_OPT_PEND_BLOCKING,
         &msg_size,
         &ts,
         &err);
    if (err != OS_ERR_NONE)
      APP_TRACE_DBG ("Error OSQPend: AppTaskCom\n");

    // obtain message we received
    if(msg_size < 40)
      errmem = memset (&CommRxBuf[0], 0, MAX_MSG_LENGTH);
    if((errmem != &CommRxBuf) || (msg_size > 39))
      APP_TRACE_DBG ("Error memset: AppTaskCom\n");

    if(msg_size < 40)
      errmem = memcpy (msg, (CPU_CHAR*) p_msg, msg_size - 1);
    if((errmem != &msg) || (msg_size > 39))
      APP_TRACE_DBG ("Error memcpy: AppTaskCom\n");

    if(msg_size < 40)
      errmem = memcpy (CommRxBuf, (CPU_CHAR*) p_msg, msg_size - 1);
    if((errmem != &CommRxBuf) || (msg_size > 39))
      APP_TRACE_DBG ("Error memcpy: AppTaskCom\n");                     // <17>
    // release the memory partition allocated in the UART service routine
    OSMemPut (&Mem_Partition, p_msg, &err);                              // <18>
    if (err != OS_ERR_NONE)
      APP_TRACE_DBG ("Error OSMemPut: AppTaskCom\n");

    // print the received message to the debug interface
    sprintf (debug_msg, "Msg: %s\tLength: %d\n", msg, msg_size - 1);     // <20>
    APP_TRACE_INFO (debug_msg);

    // send the message to AppTaskPlot to change the dutycycle
    OSQPost ( (OS_Q      *) &DUTY_QUEUE,
      (void      *) &CommRxBuf[0],
      (OS_MSG_SIZE) msg_size,
      (OS_OPT)      OS_OPT_POST_FIFO,
      (OS_ERR    *) &err);
    if (err != OS_ERR_NONE)
      APP_TRACE_DBG ("Error OSQPost: AppTaskCom\n");
  }
}
/************************************************************************ EOF */
/******************************************************************************/
