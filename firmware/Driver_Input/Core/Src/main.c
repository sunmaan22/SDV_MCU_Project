/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <string.h>
#include "task.h" /* uxTaskGetStackHighWaterMark */
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define NRF24_CMD_R_REGISTER   0x00u
#define NRF24_CMD_W_REGISTER   0x20u
#define NRF24_CMD_NOP          0xFFu

#define NRF24_REG_CONFIG       0x00u
#define NRF24_REG_STATUS       0x07u
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */
/* Tick rate is 1000Hz (FreeRTOSConfig.h) so 1 tick == 1ms. */
#define CONTROL_TASK_PERIOD_MS       10U  /* TBD: 5~10ms 후보, Owner FROZEN 필요 (ARCHITECTURE.md 7.1) */
#define DRIVER_INPUT_TASK_PERIOD_MS  10U  /* TBD: RF IRQ 핀 미배정 상태라 periodic polling, ARCHITECTURE.md 7.6 */
#define CONTROL_LED_TOGGLE_PERIOD_MS    500U
#define CONTROL_STACK_LOG_EVERY_N       100U
#define DRIVER_INPUT_STACK_LOG_EVERY_N  100U
/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

COM_InitTypeDef BspCOMInit;

FDCAN_HandleTypeDef hfdcan1;

SPI_HandleTypeDef hspi1;

/* Definitions for defaultTask */
osThreadId_t defaultTaskHandle;
const osThreadAttr_t defaultTask_attributes = {
  .name = "defaultTask",
  .priority = (osPriority_t) osPriorityNormal,
  .stack_size = 128 * 4
};
/* Definitions for ControlTask */
osThreadId_t ControlTaskHandle;
const osThreadAttr_t ControlTask_attributes = {
  .name = "ControlTask",
  .priority = (osPriority_t) osPriorityAboveNormal,
  .stack_size = 256 * 4
};
/* Definitions for DriverInputTask */
osThreadId_t DriverInputTaskHandle;
const osThreadAttr_t DriverInputTask_attributes = {
  .name = "DriverInputTask",
  .priority = (osPriority_t) osPriorityNormal,
  .stack_size = 256 * 4
};
/* USER CODE BEGIN PV */
volatile uint32_t g_controlCycleCount = 0;
volatile uint32_t g_controlOverrunCount = 0;
volatile uint32_t g_controlStackHighWaterMark = 0;

volatile uint32_t g_driverInputCycleCount = 0;
volatile uint32_t g_driverInputOverrunCount = 0;
volatile uint32_t g_driverInputStackHighWaterMark = 0;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_FDCAN1_Init(void);
static void MX_SPI1_Init(void);
void StartDefaultTask(void *argument);
void StartControlTask(void *argument);
void StartDriverInputTask(void *argument);

/* USER CODE BEGIN PFP */
static void NRF24_CSN_Low(void);
static void NRF24_CSN_High(void);
static uint8_t NRF24_ReadRegister(uint8_t reg);
static void NRF24_WriteRegister(uint8_t reg, uint8_t value);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
static void NRF24_CSN_Low(void)
{
  HAL_GPIO_WritePin(NRF_CSN_GPIO_Port, NRF_CSN_Pin, GPIO_PIN_RESET);
}

static void NRF24_CSN_High(void)
{
  HAL_GPIO_WritePin(NRF_CSN_GPIO_Port, NRF_CSN_Pin, GPIO_PIN_SET);
}

/* R_REGISTER: 두 번째로 보낸 바이트에 대한 응답이 레지스터 값, 첫 번째 응답 바이트는 STATUS.
 * Timeout을 짧게 둬서 SPI가 응답 안 해도 코드가 멈추지 않게 한다(HAL_MAX_DELAY 금지). */
static uint8_t NRF24_ReadRegister(uint8_t reg)
{
  uint8_t tx[2] = { (uint8_t)(NRF24_CMD_R_REGISTER | (reg & 0x1Fu)), NRF24_CMD_NOP };
  uint8_t rx[2] = { 0 };
  HAL_StatusTypeDef status;

  NRF24_CSN_Low();
  status = HAL_SPI_TransmitReceive(&hspi1, tx, rx, 2, 100);
  NRF24_CSN_High();

  if (status != HAL_OK)
  {
    printf("NRF24 SPI read reg=0x%02X FAILED, HAL status=%d, ErrorCode=0x%08lX\r\n",
           reg, (int)status, (unsigned long)HAL_SPI_GetError(&hspi1));
  }

  return rx[1];
}

static void NRF24_WriteRegister(uint8_t reg, uint8_t value)
{
  uint8_t tx[2] = { (uint8_t)(NRF24_CMD_W_REGISTER | (reg & 0x1Fu)), value };
  uint8_t rx[2] = { 0 };
  HAL_StatusTypeDef status;

  NRF24_CSN_Low();
  status = HAL_SPI_TransmitReceive(&hspi1, tx, rx, 2, 100);
  NRF24_CSN_High();

  if (status != HAL_OK)
  {
    printf("NRF24 SPI write reg=0x%02X FAILED, HAL status=%d, ErrorCode=0x%08lX\r\n",
           reg, (int)status, (unsigned long)HAL_SPI_GetError(&hspi1));
  }
}
/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_FDCAN1_Init();
  MX_SPI1_Init();
  /* USER CODE BEGIN 2 */
  /* nRF24L01 idle state: CE low(수신/송신 대기), CSN high(SPI 비선택) */
  HAL_GPIO_WritePin(NRF_CE_GPIO_Port, NRF_CE_Pin, GPIO_PIN_RESET);
  NRF24_CSN_High();
  HAL_Delay(100); /* nRF24L01 power-on 안정화 대기 */

  /* FDCAN1 internal loopback 시험: 외부 배선/트랜시버 없이 자체 확인.
   * StdFiltersNbr=0이라 non-matching 프레임을 기본값대로 두면 buffer에서 걸러지므로
   * global filter로 전부 RXFIFO0에 들어오게 한다. */
  if (HAL_FDCAN_ConfigGlobalFilter(&hfdcan1, FDCAN_ACCEPT_IN_RX_FIFO0, FDCAN_ACCEPT_IN_RX_FIFO0,
                                    FDCAN_FILTER_REMOTE, FDCAN_FILTER_REMOTE) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_FDCAN_Start(&hfdcan1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE END 2 */

  /* Init scheduler */
  osKernelInitialize();

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* creation of defaultTask */
  defaultTaskHandle = osThreadNew(StartDefaultTask, NULL, &defaultTask_attributes);

  /* creation of ControlTask */
  ControlTaskHandle = osThreadNew(StartControlTask, NULL, &ControlTask_attributes);

  /* creation of DriverInputTask */
  DriverInputTaskHandle = osThreadNew(StartDriverInputTask, NULL, &DriverInputTask_attributes);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  /* USER CODE END RTOS_THREADS */

  /* USER CODE BEGIN RTOS_EVENTS */
  /* add events, ... */
  /* USER CODE END RTOS_EVENTS */

  /* Initialize leds */
  BSP_LED_Init(LED_GREEN);

  /* Initialize COM1 port (115200, 8 bits (7-bit data + 1 stop bit), no parity */
  BspCOMInit.BaudRate   = 115200;
  BspCOMInit.WordLength = COM_WORDLENGTH_8B;
  BspCOMInit.StopBits   = COM_STOPBITS_1;
  BspCOMInit.Parity     = COM_PARITY_NONE;
  BspCOMInit.HwFlowCtl  = COM_HWCONTROL_NONE;
  if (BSP_COM_Init(COM1, &BspCOMInit) != BSP_ERROR_NONE)
  {
    Error_Handler();
  }

  /* USER CODE BEGIN BSP */

  /* -- Sample board code to send message over COM1 port ---- */
  printf("Welcome to STM32 world !\r\n");

  /* -- Sample board code to switch on leds ---- */
  BSP_LED_On(LED_GREEN);

  /* nRF24L01 SPI 레지스터 읽기/쓰기 시험 (T-RF-000 다음 bench 단계) */
  {
    uint8_t status_before = NRF24_ReadRegister(NRF24_REG_STATUS);
    uint8_t config_before = NRF24_ReadRegister(NRF24_REG_CONFIG);

    NRF24_WriteRegister(NRF24_REG_CONFIG, 0x0A);
    uint8_t config_after = NRF24_ReadRegister(NRF24_REG_CONFIG);

    printf("NRF24 STATUS=0x%02X CONFIG(before)=0x%02X CONFIG(after write 0x0A)=0x%02X\r\n",
           status_before, config_before, config_after);
  }

  /* FDCAN1 internal loopback 시험: 배선 없이 TX->RX 자체 확인 */
  {
    FDCAN_TxHeaderTypeDef TxHeader;
    FDCAN_RxHeaderTypeDef RxHeader = {0};
    uint8_t TxData[8] = {0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88};
    uint8_t RxData[8] = {0};
    uint32_t start_tick;
    uint8_t loopback_ok = 0;

    TxHeader.Identifier = 0x123;
    TxHeader.IdType = FDCAN_STANDARD_ID;
    TxHeader.TxFrameType = FDCAN_DATA_FRAME;
    TxHeader.DataLength = FDCAN_DLC_BYTES_8;
    TxHeader.ErrorStateIndicator = FDCAN_ESI_ACTIVE;
    TxHeader.BitRateSwitch = FDCAN_BRS_OFF;
    TxHeader.FDFormat = FDCAN_CLASSIC_CAN;
    TxHeader.TxEventFifoControl = FDCAN_NO_TX_EVENTS;
    TxHeader.MessageMarker = 0;

    if (HAL_FDCAN_AddMessageToTxFifoQ(&hfdcan1, &TxHeader, TxData) != HAL_OK)
    {
      Error_Handler();
    }

    start_tick = HAL_GetTick();
    while (HAL_FDCAN_GetRxFifoFillLevel(&hfdcan1, FDCAN_RX_FIFO0) == 0)
    {
      if ((HAL_GetTick() - start_tick) > 100)
      {
        break; /* timeout: loopback_ok는 0으로 유지 */
      }
    }

    if (HAL_FDCAN_GetRxFifoFillLevel(&hfdcan1, FDCAN_RX_FIFO0) > 0)
    {
      if (HAL_FDCAN_GetRxMessage(&hfdcan1, FDCAN_RX_FIFO0, &RxHeader, RxData) == HAL_OK)
      {
        loopback_ok = (RxHeader.Identifier == TxHeader.Identifier) &&
                      (memcmp(TxData, RxData, 8) == 0);
      }
    }

    printf("FDCAN internal loopback: %s (rx id=0x%03lX)\r\n",
           loopback_ok ? "PASS" : "FAIL", (unsigned long)RxHeader.Identifier);
  }

  /* USER CODE END BSP */

  /* Start scheduler */
  osKernelStart();

  /* We should never get here as control is now taken by the scheduler */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {

/* -- Sample board code to toggle leds ---- */
       BSP_LED_Toggle(LED_GREEN);

    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1_BOOST);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = RCC_PLLM_DIV4;
  RCC_OscInitStruct.PLL.PLLN = 85;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = RCC_PLLQ_DIV2;
  RCC_OscInitStruct.PLL.PLLR = RCC_PLLR_DIV2;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_4) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief FDCAN1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_FDCAN1_Init(void)
{

  /* USER CODE BEGIN FDCAN1_Init 0 */

  /* USER CODE END FDCAN1_Init 0 */

  /* USER CODE BEGIN FDCAN1_Init 1 */

  /* USER CODE END FDCAN1_Init 1 */
  hfdcan1.Instance = FDCAN1;
  hfdcan1.Init.ClockDivider = FDCAN_CLOCK_DIV1;
  hfdcan1.Init.FrameFormat = FDCAN_FRAME_CLASSIC;
  hfdcan1.Init.Mode = FDCAN_MODE_INTERNAL_LOOPBACK;
  hfdcan1.Init.AutoRetransmission = DISABLE;
  hfdcan1.Init.TransmitPause = DISABLE;
  hfdcan1.Init.ProtocolException = DISABLE;
  hfdcan1.Init.NominalPrescaler = 16;
  hfdcan1.Init.NominalSyncJumpWidth = 1;
  hfdcan1.Init.NominalTimeSeg1 = 1;
  hfdcan1.Init.NominalTimeSeg2 = 1;
  hfdcan1.Init.DataPrescaler = 1;
  hfdcan1.Init.DataSyncJumpWidth = 1;
  hfdcan1.Init.DataTimeSeg1 = 1;
  hfdcan1.Init.DataTimeSeg2 = 1;
  hfdcan1.Init.StdFiltersNbr = 0;
  hfdcan1.Init.ExtFiltersNbr = 0;
  hfdcan1.Init.TxFifoQueueMode = FDCAN_TX_FIFO_OPERATION;
  if (HAL_FDCAN_Init(&hfdcan1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN FDCAN1_Init 2 */

  /* USER CODE END FDCAN1_Init 2 */

}

/**
  * @brief SPI1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_SPI1_Init(void)
{

  /* USER CODE BEGIN SPI1_Init 0 */

  /* USER CODE END SPI1_Init 0 */

  /* USER CODE BEGIN SPI1_Init 1 */

  /* USER CODE END SPI1_Init 1 */
  /* SPI1 parameter configuration*/
  hspi1.Instance = SPI1;
  hspi1.Init.Mode = SPI_MODE_MASTER;
  hspi1.Init.Direction = SPI_DIRECTION_2LINES;
  hspi1.Init.DataSize = SPI_DATASIZE_8BIT;
  hspi1.Init.CLKPolarity = SPI_POLARITY_LOW;
  hspi1.Init.CLKPhase = SPI_PHASE_1EDGE;
  hspi1.Init.NSS = SPI_NSS_SOFT;
  hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_128;
  hspi1.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi1.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi1.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi1.Init.CRCPolynomial = 7;
  hspi1.Init.CRCLength = SPI_CRC_LENGTH_DATASIZE;
  hspi1.Init.NSSPMode = SPI_NSS_PULSE_DISABLE;
  if (HAL_SPI_Init(&hspi1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN SPI1_Init 2 */

  /* USER CODE END SPI1_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  /* USER CODE BEGIN MX_GPIO_Init_1 */

  /* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, NRF_CE_Pin|NRF_CSN_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pins : NRF_CE_Pin NRF_CSN_Pin */
  GPIO_InitStruct.Pin = NRF_CE_Pin|NRF_CSN_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /* USER CODE BEGIN MX_GPIO_Init_2 */

  /* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/* USER CODE BEGIN Header_StartDefaultTask */
/**
  * @brief  Function implementing the defaultTask thread.
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_StartDefaultTask */
void StartDefaultTask(void *argument)
{
  /* USER CODE BEGIN 5 */
  /* Infinite loop */
  for(;;)
  {
    osDelay(1);
  }
  /* USER CODE END 5 */
}

/* USER CODE BEGIN Header_StartControlTask */
/**
* @brief Function implementing the ControlTask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartControlTask */
void StartControlTask(void *argument)
{
  /* USER CODE BEGIN StartControlTask */
  uint32_t wakeTime = osKernelGetTickCount();
  uint32_t ledToggleAccumMs = 0;

  /* Motor PWM/Servo output: not yet implemented, Owner FROZEN 필요 (bench: RTOS skeleton only) */
  for(;;)
  {
    wakeTime += CONTROL_TASK_PERIOD_MS;
    if (osDelayUntil(wakeTime) != osOK)
    {
      /* Target tick already passed: previous cycle overran the period */
      g_controlOverrunCount++;
    }

    g_controlCycleCount++;

    ledToggleAccumMs += CONTROL_TASK_PERIOD_MS;
    if (ledToggleAccumMs >= CONTROL_LED_TOGGLE_PERIOD_MS)
    {
      ledToggleAccumMs = 0;
      BSP_LED_Toggle(LED_GREEN); /* bench heartbeat: task is alive */
    }

    if ((g_controlCycleCount % CONTROL_STACK_LOG_EVERY_N) == 0U)
    {
      g_controlStackHighWaterMark = (uint32_t)uxTaskGetStackHighWaterMark(NULL);
    }
  }
  /* USER CODE END StartControlTask */
}

/* USER CODE BEGIN Header_StartDriverInputTask */
/**
* @brief Function implementing the DriverInputTask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartDriverInputTask */
void StartDriverInputTask(void *argument)
{
  /* USER CODE BEGIN StartDriverInputTask */
  uint32_t wakeTime = osKernelGetTickCount();

  /* RF(nRF24L01) 패킷 읽기/Driver_Input 발행: 핀/패킷 레이아웃 TBD, Owner FROZEN 필요.
   * IRQ 핀이 배정되면 이 periodic polling을 DriverInputNotify(Task Notification)
   * 기반 event-driven으로 교체한다 (ARCHITECTURE.md 7.3, 7.4). */
  for(;;)
  {
    wakeTime += DRIVER_INPUT_TASK_PERIOD_MS;
    if (osDelayUntil(wakeTime) != osOK)
    {
      g_driverInputOverrunCount++;
    }

    g_driverInputCycleCount++;

    if ((g_driverInputCycleCount % DRIVER_INPUT_STACK_LOG_EVERY_N) == 0U)
    {
      g_driverInputStackHighWaterMark = (uint32_t)uxTaskGetStackHighWaterMark(NULL);
    }
  }
  /* USER CODE END StartDriverInputTask */
}

/**
  * @brief  Period elapsed callback in non blocking mode
  * @note   This function is called  when TIM6 interrupt took place, inside
  * HAL_TIM_IRQHandler(). It makes a direct call to HAL_IncTick() to increment
  * a global variable "uwTick" used as application time base.
  * @param  htim : TIM handle
  * @retval None
  */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  /* USER CODE BEGIN Callback 0 */

  /* USER CODE END Callback 0 */
  if (htim->Instance == TIM6)
  {
    HAL_IncTick();
  }
  /* USER CODE BEGIN Callback 1 */

  /* USER CODE END Callback 1 */
}
/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/**
  * @}
  */

/**
  * @}
  */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}
#ifdef USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
