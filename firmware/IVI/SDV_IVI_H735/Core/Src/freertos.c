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
#include "vehicle_data.h"
#include "dummy_data_provider.h"

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

/* ---- VehicleModelTask + VehicleDataRepository (see Core/Inc/vehicle_data.h) ---- */

#define VEHICLE_MODEL_PERIOD_MS   100u
#define DRIVE_TIMEOUT_MS          500u
#define GEAR_READY_TIMEOUT_MS     800u
#define WARNING_TIMEOUT_MS        3000u
#define UPDATE_QUEUE_DEPTH        8u
#define REPO_LOCK_WAIT_MS         20u

VehicleDataSnapshot g_vehicle_data;

static osMessageQueueId_t s_updateQueue;
static osMutexId_t s_repoMutex;

static DriveSignal s_drive;
static GearReadySignal s_gearReady;
static WarningSignal s_warning;

static const osThreadAttr_t vehicleModelTaskAttr = {
  .name = "VehicleModel", .stack_size = 1024, .priority = (osPriority_t) osPriorityAboveNormal
};

static uint32_t VehicleModelTicks(uint32_t ms)
{
  return (osKernelGetTickFreq() * ms + 999u) / 1000u;
}

/* now - last_update_tick as unsigned wraps correctly across tick overflow. */
static SignalStatus ComputeSignalStatus(uint8_t received, uint8_t source_valid,
                                         uint32_t last_update_tick, uint32_t now,
                                         uint32_t timeout_ticks)
{
  if (!received) return SIGNAL_NO_DATA;
  if (!source_valid) return SIGNAL_INVALID;
  if ((uint32_t)(now - last_update_tick) > timeout_ticks) return SIGNAL_TIMEOUT;
  return SIGNAL_VALID;
}

static void VehicleModel_ApplyUpdate(const VehicleUpdateMsg *msg, uint32_t now)
{
  switch (msg->type) {
    case VEHICLE_UPDATE_DRIVE:
      s_drive.speed_kmh = msg->payload.drive.speed_kmh;
      s_drive.rpm = msg->payload.drive.rpm;
      s_drive.source_valid = msg->payload.drive.source_valid;
      s_drive.received = 1;
      s_drive.last_update_tick = now;
      break;
    case VEHICLE_UPDATE_GEAR_READY:
      s_gearReady.gear = msg->payload.gear_ready.gear;
      s_gearReady.ready = msg->payload.gear_ready.ready;
      s_gearReady.source_valid = msg->payload.gear_ready.source_valid;
      s_gearReady.received = 1;
      s_gearReady.last_update_tick = now;
      break;
    case VEHICLE_UPDATE_WARNING:
      s_warning.severity = msg->payload.warning.severity;
      s_warning.active = msg->payload.warning.active;
      s_warning.received = 1;
      s_warning.last_update_tick = now;
      break;
    default:
      break;
  }
}

static void VehicleModelTask(void *argument)
{
  (void)argument;
  memset(&s_drive, 0, sizeof(s_drive));
  memset(&s_gearReady, 0, sizeof(s_gearReady));
  memset(&s_warning, 0, sizeof(s_warning));
  s_gearReady.gear = '-';

  for (;;) {
    VehicleUpdateMsg msg;
    osStatus_t got = osMessageQueueGet(s_updateQueue, &msg, NULL,
                                        VehicleModelTicks(VEHICLE_MODEL_PERIOD_MS));
    uint32_t now = osKernelGetTickCount();

    if (got == osOK) {
      VehicleModel_ApplyUpdate(&msg, now);
    }

    /* Recompute every wake, whether or not a message arrived, so a signal
     * that simply stopped updating still ages into SIGNAL_TIMEOUT. */
    s_drive.status = ComputeSignalStatus(s_drive.received, s_drive.source_valid,
                                          s_drive.last_update_tick, now,
                                          VehicleModelTicks(DRIVE_TIMEOUT_MS));
    s_gearReady.status = ComputeSignalStatus(s_gearReady.received, s_gearReady.source_valid,
                                              s_gearReady.last_update_tick, now,
                                              VehicleModelTicks(GEAR_READY_TIMEOUT_MS));
    s_warning.status = ComputeSignalStatus(s_warning.received, 1u,
                                            s_warning.last_update_tick, now,
                                            VehicleModelTicks(WARNING_TIMEOUT_MS));

    if (osMutexAcquire(s_repoMutex, VehicleModelTicks(REPO_LOCK_WAIT_MS)) == osOK) {
      g_vehicle_data.drive = s_drive;
      g_vehicle_data.gear_ready = s_gearReady;
      g_vehicle_data.warning = s_warning;
      g_vehicle_data.demo_source = 1u;
      g_vehicle_data.snapshot_version++;
      osMutexRelease(s_repoMutex);
    }
  }
}

void VehicleModel_Create(void)
{
  memset(&g_vehicle_data, 0, sizeof(g_vehicle_data));
  g_vehicle_data.gear_ready.gear = '-';

  s_repoMutex = osMutexNew(NULL);
  s_updateQueue = osMessageQueueNew(UPDATE_QUEUE_DEPTH, sizeof(VehicleUpdateMsg), NULL);
  if (s_repoMutex == NULL || s_updateQueue == NULL ||
      osThreadNew(VehicleModelTask, NULL, &vehicleModelTaskAttr) == NULL) {
    /* Nothing to fall back to for the dummy phase; the NO_DATA/'-' init
     * state left in g_vehicle_data is the safe failure. */
  }
}

uint8_t VehicleModel_PushUpdate(const VehicleUpdateMsg *msg)
{
  if (s_updateQueue == NULL) return 0u;
  return (osMessageQueuePut(s_updateQueue, msg, 0, 0) == osOK) ? 1u : 0u;
}

uint8_t VehicleModel_GetSnapshot(VehicleDataSnapshot *out, uint32_t wait_ms)
{
  if (s_repoMutex == NULL) return 0u;
  if (osMutexAcquire(s_repoMutex, VehicleModelTicks(wait_ms)) != osOK) return 0u;
  *out = g_vehicle_data;
  osMutexRelease(s_repoMutex);
  return 1u;
}

/* ---- DummyDataProvider (see Core/Inc/dummy_data_provider.h) ----
 * Bench-only signal source for the Cluster data path before real CAN
 * exists. Only calls VehicleModel_PushUpdate(); never touches the GUI.
 * Example values (speed=24, rpm=1250, gear=D) are the guide's bench
 * defaults, not a vehicle spec. */

#define DUMMY_PERIOD_MS   200u

volatile DummyMode g_dummy_mode = DUMMY_MODE_NORMAL;
volatile DummyDataStats g_dummy_stats;

static const osThreadAttr_t dummyDataTaskAttr = {
  .name = "DummyDataProvider", .stack_size = 768, .priority = (osPriority_t) osPriorityNormal
};

static void DummyData_Push(const VehicleUpdateMsg *msg)
{
  g_dummy_stats.update_count++;
  if (!VehicleModel_PushUpdate(msg)) {
    g_dummy_stats.queue_overflow++;
  }
}

static void DummyDataTask(void *argument)
{
  (void)argument;
  uint32_t counter = 0;

  for (;;) {
    DummyMode mode = g_dummy_mode;
    int32_t wobble = (int32_t)(counter % 7u) - 3; /* -3..+3, just to show motion */

    if (mode != DUMMY_MODE_PAUSE_DRIVE) {
      VehicleUpdateMsg drive = {0};
      drive.type = VEHICLE_UPDATE_DRIVE;
      drive.payload.drive.speed_kmh = 24 + wobble;
      drive.payload.drive.rpm = 1250 + wobble * 10;
      drive.payload.drive.source_valid = (mode != DUMMY_MODE_SOURCE_INVALID) ? 1u : 0u;
      DummyData_Push(&drive);
    }
    /* PAUSE_DRIVE intentionally sends nothing here: DUMMY-04 relies on
     * VehicleModelTask aging this signal into SIGNAL_TIMEOUT on its own. */

    {
      VehicleUpdateMsg gearReady = {0};
      gearReady.type = VEHICLE_UPDATE_GEAR_READY;
      gearReady.payload.gear_ready.gear = 'D';
      gearReady.payload.gear_ready.ready = 1u;
      gearReady.payload.gear_ready.source_valid = (mode != DUMMY_MODE_SOURCE_INVALID) ? 1u : 0u;
      DummyData_Push(&gearReady);
    }

    {
      VehicleUpdateMsg warning = {0};
      warning.type = VEHICLE_UPDATE_WARNING;
      if (mode == DUMMY_MODE_CRITICAL) {
        warning.payload.warning.severity = WARNING_CRITICAL;
        warning.payload.warning.active = 1u;
      } else {
        warning.payload.warning.severity = WARNING_NONE;
        warning.payload.warning.active = 0u;
      }
      DummyData_Push(&warning);
    }

    counter++;
    osDelay((osKernelGetTickFreq() * DUMMY_PERIOD_MS + 999u) / 1000u);
  }
}

void DummyDataProvider_Create(void)
{
  (void)osThreadNew(DummyDataTask, NULL, &dummyDataTaskAttr);
}

/* USER CODE END Application */

