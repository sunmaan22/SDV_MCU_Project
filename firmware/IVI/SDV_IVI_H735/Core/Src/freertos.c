/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * File Name          : freertos.c
  * Description        : Code for freertos applications
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2020 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under Ultimate Liberty license
  * SLA0044, the "License"; You may not use this file except in compliance with
  * the License. You may obtain a copy of the License at:
  *                             www.st.com/SLA0044
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "cmsis_os2.h"
#include <string.h>

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN Variables */

/* USER CODE END Variables */

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */
extern portBASE_TYPE IdleTaskHook(void* p);
/* USER CODE END FunctionPrototypes */

/* Hook prototypes */
void vApplicationIdleHook(void);

/* USER CODE BEGIN 2 */
void vApplicationIdleHook( void )
{
   /* vApplicationIdleHook() will only be called if configUSE_IDLE_HOOK is set
   to 1 in FreeRTOSConfig.h. It will be called on each iteration of the idle
   task. It is essential that code added to this hook function never attempts
   to block in any way (for example, call xQueueReceive() with a block time
   specified, or call vTaskDelay()). If the application makes use of the
   vTaskDelete() API function (as this demo application does) then it is also
   important that vApplicationIdleHook() is permitted to return to its calling
   function, because it is the responsibility of the idle task to clean up
   memory allocated by the kernel to any task that has since been deleted. */
  
  vTaskSetApplicationTaskTag(NULL, IdleTaskHook);
}
/* USER CODE END 2 */

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */
/* Bench-only Classic CAN loopback. No SDV network contract is defined here. */
extern FDCAN_HandleTypeDef hfdcan2;
typedef struct {
  FDCAN_RxHeaderTypeDef header;
  uint8_t data[8];
} LoopbackFrame;
typedef struct {
  uint32_t state; /* 0 idle, 1 running, 2 PASS, 3 FAIL */
  uint32_t tx_count, rx_count, pass_count, mismatch_count, timeout_count;
  uint32_t irq_count, queue_overflow, rx_error, api_error;
  uint32_t last_hal_error, tx_error_counter, rx_error_counter, bus_off;
  uint32_t stack_free_bytes;
} LoopbackStats;
volatile LoopbackStats g_fdcan_loopback;
static osMessageQueueId_t loopbackQueue;
static const osThreadAttr_t loopbackTaskAttr = {
  .name = "CanLoopback", .stack_size = 2048, .priority = osPriorityBelowNormal
};

static uint32_t LoopbackTicks(uint32_t ms)
{
  return (osKernelGetTickFreq() * ms + 999U) / 1000U;
}

void HAL_FDCAN_RxFifo0Callback(FDCAN_HandleTypeDef *h, uint32_t flags)
{
  if (h->Instance != FDCAN2 || !(flags & FDCAN_IT_RX_FIFO0_NEW_MESSAGE)) return;
  ++g_fdcan_loopback.irq_count;
  /* Bounded drain; comparison stays in the task. */
  for (uint32_t i = 0; i < 8U && HAL_FDCAN_GetRxFifoFillLevel(h, FDCAN_RX_FIFO0); ++i) {
    LoopbackFrame f = {0};
    if (HAL_FDCAN_GetRxMessage(h, FDCAN_RX_FIFO0, &f.header, f.data) != HAL_OK) {
      ++g_fdcan_loopback.rx_error;
      break;
    }
    if (osMessageQueuePut(loopbackQueue, &f, 0, 0) != osOK)
      ++g_fdcan_loopback.queue_overflow;
  }
}

static void LoopbackTask(void *argument)
{
  (void)argument;
  FDCAN_FilterTypeDef filter = {0};
  FDCAN_TxHeaderTypeDef tx = {0};
  FDCAN_ErrorCountersTypeDef errors = {0};
  FDCAN_ProtocolStatusTypeDef protocol = {0};
  g_fdcan_loopback.state = 1;
  /* Never transmit the bench ID on a physical bus by accidental mode change. */
  if (hfdcan2.Init.Mode != FDCAN_MODE_INTERNAL_LOOPBACK ||
      hfdcan2.Init.FrameFormat != FDCAN_FRAME_CLASSIC) goto fail;
  filter.IdType = FDCAN_STANDARD_ID;
  filter.FilterIndex = 0;
  filter.FilterType = FDCAN_FILTER_MASK;
  filter.FilterConfig = FDCAN_FILTER_TO_RXFIFO0;
  filter.FilterID1 = 0x123;
  filter.FilterID2 = 0x7FF;
  if (HAL_FDCAN_ConfigFilter(&hfdcan2, &filter) != HAL_OK ||
      HAL_FDCAN_ConfigGlobalFilter(&hfdcan2, FDCAN_REJECT, FDCAN_REJECT,
        FDCAN_REJECT_REMOTE, FDCAN_REJECT_REMOTE) != HAL_OK ||
      HAL_FDCAN_ConfigInterruptLines(&hfdcan2, FDCAN_IT_RX_FIFO0_NEW_MESSAGE,
        FDCAN_INTERRUPT_LINE0) != HAL_OK ||
      HAL_FDCAN_ActivateNotification(&hfdcan2, FDCAN_IT_RX_FIFO0_NEW_MESSAGE, 0) != HAL_OK ||
      HAL_FDCAN_Start(&hfdcan2) != HAL_OK) goto fail;
  tx.Identifier = 0x123;
  tx.IdType = FDCAN_STANDARD_ID;
  tx.TxFrameType = FDCAN_DATA_FRAME;
  tx.DataLength = FDCAN_DLC_BYTES_8;
  tx.ErrorStateIndicator = FDCAN_ESI_ACTIVE;
  tx.BitRateSwitch = FDCAN_BRS_OFF;
  tx.FDFormat = FDCAN_CLASSIC_CAN;
  tx.TxEventFifoControl = FDCAN_NO_TX_EVENTS;
  for (uint32_t sequence = 0; sequence < 100U; ++sequence) {
    uint8_t data[8] = {0x53, 0x44, 0x56, 0, 0, 0, 0, 0xA5};
    LoopbackFrame received;
    data[3] = (uint8_t)sequence;
    data[4] = (uint8_t)(sequence >> 8);
    data[5] = (uint8_t)(sequence >> 16);
    data[6] = (uint8_t)(sequence >> 24);
    if (HAL_FDCAN_AddMessageToTxFifoQ(&hfdcan2, &tx, data) != HAL_OK) goto fail;
    ++g_fdcan_loopback.tx_count;
    if (osMessageQueueGet(loopbackQueue, &received, NULL, LoopbackTicks(100)) != osOK) {
      ++g_fdcan_loopback.timeout_count;
      goto fail;
    }
    ++g_fdcan_loopback.rx_count;
    if (received.header.Identifier != tx.Identifier || received.header.IdType != FDCAN_STANDARD_ID ||
        received.header.RxFrameType != FDCAN_DATA_FRAME || received.header.DataLength != FDCAN_DLC_BYTES_8 ||
        received.header.FDFormat != FDCAN_CLASSIC_CAN || memcmp(data, received.data, 8) != 0) {
      ++g_fdcan_loopback.mismatch_count;
      goto fail;
    }
    if (HAL_FDCAN_GetErrorCounters(&hfdcan2, &errors) != HAL_OK ||
        HAL_FDCAN_GetProtocolStatus(&hfdcan2, &protocol) != HAL_OK) goto fail;
    g_fdcan_loopback.tx_error_counter = errors.TxErrorCnt;
    g_fdcan_loopback.rx_error_counter = errors.RxErrorCnt;
    g_fdcan_loopback.bus_off = protocol.BusOff;
    if (errors.TxErrorCnt || errors.RxErrorCnt || protocol.BusOff ||
        g_fdcan_loopback.queue_overflow || g_fdcan_loopback.rx_error) goto fail;
    ++g_fdcan_loopback.pass_count;
    g_fdcan_loopback.stack_free_bytes = osThreadGetStackSpace(osThreadGetId());
    osDelay(LoopbackTicks(100));
  }
  if (HAL_FDCAN_Stop(&hfdcan2) != HAL_OK) goto fail;
  g_fdcan_loopback.state = 2;
  osThreadExit();
  return;
fail:
  g_fdcan_loopback.last_hal_error = HAL_FDCAN_GetError(&hfdcan2);
  if (!g_fdcan_loopback.timeout_count && !g_fdcan_loopback.mismatch_count)
    ++g_fdcan_loopback.api_error;
  (void)HAL_FDCAN_Stop(&hfdcan2);
  g_fdcan_loopback.state = 3;
  osThreadExit();
}

void CanLoopback_Create(void)
{
  loopbackQueue = osMessageQueueNew(8, sizeof(LoopbackFrame), NULL);
  if (loopbackQueue == NULL || osThreadNew(LoopbackTask, NULL, &loopbackTaskAttr) == NULL) {
    g_fdcan_loopback.api_error = 1;
    g_fdcan_loopback.state = 3;
  }
}

/* USER CODE END Application */

