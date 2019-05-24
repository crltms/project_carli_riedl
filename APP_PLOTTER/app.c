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
#define ACK  0x6
#define MAX_MSG_LENGTH 20
#define NUM_MSG        3

/********************************************************* FILE LOCAL GLOBALS */
static  CPU_STK  AppStartTaskStk[APP_CFG_TASK_START_STK_SIZE];            // <1>
static  OS_TCB   AppStartTaskTCB;

static  CPU_STK  AppTaskLEDStk[APP_CFG_TASK_LED_STK_SIZE];
static  OS_TCB   AppTaskLEDTCB;

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
OS_TMR      TMR_STEP;

OS_SEM      XYTEST_SEM;

/****************************************************** FILE LOCAL PROTOTYPES */
static  void AppTaskStart (void  *p_arg);
static  void AppTaskCreate (void);
static  void AppObjCreate (void);
static  void AppTaskCom (void  *p_arg);
static  void AppTaskLED (void  *p_arg);
static  void AppTaskXYTest (void  *p_arg);

uint8_t dir = 0;
//uint8_t dir = 0;
/* Only for testing with logic analyzer --> how long is the LedTask executing
const XMC_GPIO_CONFIG_t  config_PortOut	=
{
  .mode                = XMC_GPIO_MODE_OUTPUT_PUSH_PULL,
  .output_level        = XMC_GPIO_OUTPUT_LEVEL_LOW,
	.output_strength     = XMC_GPIO_OUTPUT_STRENGTH_MEDIUM,
};
*/
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
  //BSP_IntEn (BSP_INT_ID_CCU40_01); //** FOR MOTOR
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
    APP_TRACE_DBG ("Error OSQCreate: AppObjCreate\n");

  OSQCreate ( (OS_Q *)     &DUTY_QUEUE,
        (CPU_CHAR *) "DUTY Queue",
        (OS_MSG_QTY) NUM_MSG,
        (OS_ERR   *) &err);
  if (err != OS_ERR_NONE)
    APP_TRACE_DBG ("Error OSQCreate: AppObjCreate\n");

  // Create timer
  // OSTmrCreate(&TMR_STEP,
  //             "Timer Step",
  //             0,
  //             1,
  //             OS_OPT_TMR_PERIODIC,
  //             BSP_Timer_Handler,
  //             0,
  //             &err);
  // if (err != OS_ERR_NONE)
  //     APP_TRACE_DBG ("Error OSQCreate: AppObjCreate\n");
  // OSTmrStart (&TMR_STEP,&err);

  // OSQCreate ( (OS_Q *)     &START_QUEUE,
  //       (CPU_CHAR *) "START Queue",
  //       (OS_MSG_QTY) NUM_MSG,
  //       (OS_ERR   *) &err);
  // if (err != OS_ERR_NONE)
  //   APP_TRACE_DBG ("Error OSQCreate: AppObjCreate\n");

  OSSemCreate(&XYTEST_SEM, "XYTest_sem", 0, &err);
  if (err != OS_ERR_NONE)
      APP_TRACE_DBG ("Error OSQCreate: AppObjCreate\n");
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

  OSTaskCreate ( (OS_TCB     *) &AppTaskLEDTCB,
           (CPU_CHAR   *) "TaskLED",
           (OS_TASK_PTR) AppTaskLED,
           (void       *) 0,
           (OS_PRIO) APP_CFG_TASK_LED_PRIO,
           (CPU_STK    *) &AppTaskLEDStk[0],
           (CPU_STK_SIZE) APP_CFG_TASK_LED_STK_SIZE / 10u,
           (CPU_STK_SIZE) APP_CFG_TASK_LED_STK_SIZE,
           (OS_MSG_QTY) 0u,
           (OS_TICK) 0u,
           (void       *) 0,
           (OS_OPT) (OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR),
           (OS_ERR     *) &err);
  if (err != OS_ERR_NONE)
    APP_TRACE_DBG ("Error OSTaskCreate: AppTaskCreate(LED)\n");

  OSTaskCreate ( (OS_TCB     *) &AppTaskXYTestTCB,
           (CPU_CHAR   *) "TestTaskXY",
           (OS_TASK_PTR) AppTaskXYTest,
           (void       *) 0,
           (OS_PRIO) APP_CFG_TASK_XYTEST_PRIO,
           (CPU_STK    *) &AppTaskLEDStk[0],
           (CPU_STK_SIZE) APP_CFG_TASK_XYTEST_STK_SIZE / 10u,
           (CPU_STK_SIZE) APP_CFG_TASK_XYTEST_STK_SIZE,
           (OS_MSG_QTY) 0u,
           (OS_TICK) 0u,
           (void       *) 0,
           (OS_OPT) (OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR),
           (OS_ERR     *) &err);
  if (err != OS_ERR_NONE)
    APP_TRACE_DBG ("Error OSTaskCreate: AppTaskCreate(LED)\n");
}

void AppTaskXYTest(void *p_arg)
{
  void        *errmem = NULL;
  OS_ERR      err;
  CPU_TS      ts;
  void        *p_msg;
  OS_MSG_SIZE msg_size;

  static int lowhigh = 0;
  uint8_t reg_val = 0x00;
  uint8_t recv = 0;
  int end1, end2, end3, end4;
  int steps = 0;
  int rounds = 0;
  int test = 0;
  while(DEF_TRUE)
  {
    // OSSemPend(&XYTEST_SEM, 0, OS_OPT_PEND_BLOCKING, &ts,&err);
    // if ((err != OS_ERR_NONE))
    //     APP_TRACE_DBG ("Error OSSemPend: AppTaskXYTest\n");
    //
    // p_msg = OSQPend (&START_QUEUE,                                          // <16>
    //       0,
    //       OS_OPT_PEND_BLOCKING,
    //       &msg_size,
    //       &ts,
    //       &err);
    // if (err != OS_ERR_NONE)
    //   APP_TRACE_DBG ("Error OSQPend: AppTaskLED\n");
    // OSTimeDlyHMSM(0,
    //               0,
    //               0,
    //               1,
    //               OS_OPT_TIME_HMSM_STRICT,
    //               &err);
    while(test < 255)
    {
      test++;
    }
    test = 0;
    if(XMC_GPIO_GetInput(D5) == 1)  // y-Achse MinusRichtung
      end1 = 1;
    else
      end1 = 0;

    if(XMC_GPIO_GetInput(D6) == 1)  // y-Achse PlusRichtung
      end2 = 1;
    else
      end2 = 0;

    if(XMC_GPIO_GetInput(D7) == 1)  // x-Achse MinusRichtung
      end3 = 1;
    else
      end3 = 0;

    if(XMC_GPIO_GetInput(D8) == 1)  // x-Achse PlusRichtung
      end4 = 1;
    else
      end4 = 0;

    // XY Abfahren
    if((dir == 0)||(dir == 1)||(dir == 2)||(dir == 3))
    {
      // if(steps <= 200)
      //   steps++;
      // else
      // {
      //   steps = 0;
      //   rounds++;
      // }
      if(dir == 0) // y-Minusichtung
      {
        if(end1 == 1)
        {
          steps++;
          if(lowhigh == 0)
          {
            reg_val = 0x00;
            lowhigh = 1;
            _mcp23s08_reset_ss(MCP23S08_SS);
            _mcp23s08_reg_xfer(XMC_SPI1_CH0,MCP23S08_GPIO,Y_MINUS_PLOT_LOW,MCP23S08_WR);
            _mcp23s08_set_ss(MCP23S08_SS);
          }
          else
          {
            reg_val = 0x02;
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
      if(dir == 1) // y-Plusrichtung
      {
        steps++;
        if(end2 == 1)
        {
          if(lowhigh == 0)
          {
            reg_val = 0x01;
            lowhigh = 1;
            _mcp23s08_reset_ss(MCP23S08_SS);
            _mcp23s08_reg_xfer(XMC_SPI1_CH0,MCP23S08_GPIO,Y_PLUS_PLOT_LOW,MCP23S08_WR);
            _mcp23s08_set_ss(MCP23S08_SS);
          }
          else
          {
            reg_val = 0x03;
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
      if(dir == 2) // x-Minusrichtung
      {
        if(end3 == 1)
        {
          steps++;
          if(lowhigh == 0)
          {
            reg_val = 0x00;
            lowhigh = 1;
            _mcp23s08_reset_ss(MCP23S08_SS);
            _mcp23s08_reg_xfer(XMC_SPI1_CH0,MCP23S08_GPIO,X_MINUS_PLOT_LOW,MCP23S08_WR);
            _mcp23s08_set_ss(MCP23S08_SS);
          }
          else
          {
            reg_val = 0x08;
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
      if(dir == 3) // x-plusrichtung
      {
        if(end4 == 1)
        {
          steps++;
          if(lowhigh == 0)
          {
            reg_val = 0x04;
            lowhigh = 1;
            _mcp23s08_reset_ss(MCP23S08_SS);
            _mcp23s08_reg_xfer(XMC_SPI1_CH0,MCP23S08_GPIO,X_PLUS_PLOT_LOW,MCP23S08_WR);
            _mcp23s08_set_ss(MCP23S08_SS);
          }
          else
          {
            reg_val = 0x0c;
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
void AppTaskLED(void *p_arg)
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
  while(DEF_TRUE)
  {
    /****Verification****/
    //XMC_DAC_CH_Write(XMC_DAC0, 1, 2500);
    //XMC_GPIO_SetOutputHigh(P1_11);
    /******************/
    errno = 0;
    errmem = memset(&data[0], 0, MAX_MSG_LENGTH);
    if(errmem != &data)
      APP_TRACE_DBG ("Error memset: AppTaskLED\n");

    p_msg = OSQPend (&DUTY_QUEUE,                                          // <16>
          0,
          OS_OPT_PEND_BLOCKING,
          &msg_size,
          &ts,
          &err);
    if (err != OS_ERR_NONE)
      APP_TRACE_DBG ("Error OSQPend: AppTaskLED\n");

    /****Verification****/
    //XMC_DAC_CH_Write(XMC_DAC0, 1, 1500);
    //XMC_GPIO_SetOutputLow(P1_11);
    /*******************/
    if(msg_size < 20)
      errmem = memcpy (data, p_msg, msg_size - 1);
    else
      APP_TRACE_DBG ("Error msg_size/memcpy: AppTaskLED\n");
    if(errmem != &data)
      APP_TRACE_DBG ("Error memcpy: AppTaskLED\n");

    char *token = strtok(data, ":");
    //sscanf(token, "%d", &dutycycle);

    dutycycle = strtol(token, &pEnd,10);
    // control execution of strol
    if((errno == ERANGE &&((dutycycle == LONG_MAX)||(dutycycle==LONG_MIN)))||(errno != 0 && dutycycle == 0))
      APP_TRACE_DBG ("Error strtol: AppTaskLED\n");

    // jump in subroutine to set the dutycycle
    token = strtok(NULL, ":");
    if(*token == '1') // Dutycycle for Port 1 Pin 1 --> LED 1
      ret = BSP_PWM_SetDutyCycle(1, dutycycle);
    if(*token == '2') // Dutycycle for Port 1 Pin 0 --> LED2
      ret = BSP_PWM_SetDutyCycle(2, dutycycle);
    if(*token == '3') // Dutycycle for Port 0 Pin 5
      ret = BSP_PWM_SetDutyCycle(3, dutycycle);
    if(*token == '4') // Dutycycle for Port 0 Pin 9
      ret = BSP_PWM_SetDutyCycle(4, dutycycle);
    if(*token == '5') // Dutycycle for Port 0 Pin 9
      ret = BSP_PWM_SetDutyCycle(5, dutycycle);
    if(!ret)
      APP_TRACE_DBG ("Error returnval: AppTaskLED\n");

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
 *        (2) Connect a TTL-USB UART cable:
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
    if(msg_size < 20)
      errmem = memset (&CommRxBuf[0], 0, MAX_MSG_LENGTH);
    if((errmem != &CommRxBuf) || (msg_size > 19))
      APP_TRACE_DBG ("Error memset: AppTaskCom\n");

    if(msg_size < 20)
      errmem = memcpy (msg, (CPU_CHAR*) p_msg, msg_size - 1);
    if((errmem != &msg) || (msg_size > 19))
      APP_TRACE_DBG ("Error memcpy: AppTaskCom\n");

    if(msg_size < 20)
      errmem = memcpy (CommRxBuf, (CPU_CHAR*) p_msg, msg_size - 1);
    if((errmem != &CommRxBuf) || (msg_size > 19))
      APP_TRACE_DBG ("Error memcpy: AppTaskCom\n");                     // <17>
    // release the memory partition allocated in the UART service routine
    OSMemPut (&Mem_Partition, p_msg, &err);                              // <18>
    if (err != OS_ERR_NONE)
      APP_TRACE_DBG ("Error OSMemPut: AppTaskCom\n");

    // send ACK in return
    XMC_UART_CH_Transmit (XMC_UART1_CH1, ACK);                           // <19>

    // print the received message to the debug interface
    sprintf (debug_msg, "Msg: %s\tLength: %d\n", msg, msg_size - 1);     // <20>
    APP_TRACE_INFO (debug_msg);

    // send the message to AppTaskLED to change the dutycycle
    OSQPost ( (OS_Q      *) &DUTY_QUEUE,
      (void      *) &CommRxBuf[0],
      (OS_MSG_SIZE) msg_size,
      (OS_OPT)      OS_OPT_POST_FIFO,
      (OS_ERR    *) &err);
    if (err != OS_ERR_NONE)
      APP_TRACE_DBG ("Error OSQPost: AppTaskCom\n");

    // send the received message back via the UART pre-text with "DUC: " --> Dutycycle
    XMC_UART_CH_Transmit (XMC_UART1_CH1, 'D');                           // <21>
    XMC_UART_CH_Transmit (XMC_UART1_CH1, 'U');
    XMC_UART_CH_Transmit (XMC_UART1_CH1, 'C');
    XMC_UART_CH_Transmit (XMC_UART1_CH1, ':');
    XMC_UART_CH_Transmit (XMC_UART1_CH1, ' ');
    for (i = 0; i <= msg_size; i++) {
      XMC_UART_CH_Transmit (XMC_UART1_CH1, msg[i]);
    }
    XMC_UART_CH_Transmit (XMC_UART1_CH1, '%');
    XMC_UART_CH_Transmit (XMC_UART1_CH1, '\n');
  }
}
/************************************************************************ EOF */
/******************************************************************************/
