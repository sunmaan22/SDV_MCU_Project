/*
 * dummy_data_provider.h
 *
 * Bench-only signal source for the Cluster data path before real CAN
 * exists. Pushes logical updates through VehicleModel_PushUpdate() -- it
 * never touches the GUI or the repository directly. See
 * docs/ecus/IVI/IVI_MPU_Dummy_TouchGFX_Guide.md section 4.
 */
#ifndef DUMMY_DATA_PROVIDER_H
#define DUMMY_DATA_PROVIDER_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Guide's 5 test scenarios (DUMMY-01..10). Change g_dummy_mode from a
 * debugger Expressions/Live view to switch scenario; this is a test
 * selector, not a production input path. */
typedef enum
{
  DUMMY_MODE_NORMAL = 0,
  DUMMY_MODE_SOURCE_INVALID,
  DUMMY_MODE_PAUSE_DRIVE,
  DUMMY_MODE_CRITICAL,
  DUMMY_MODE_RECOVERY
} DummyMode;

typedef struct
{
  uint32_t update_count;
  uint32_t queue_overflow;
} DummyDataStats;

extern volatile DummyMode g_dummy_mode;
extern volatile DummyDataStats g_dummy_stats;

/* Creates DummyDataTask. Call once from main()'s USER CODE BEGIN
 * RTOS_THREADS, after VehicleModel_Create(), before osKernelStart(). */
void DummyDataProvider_Create(void);

#ifdef __cplusplus
}
#endif

#endif /* DUMMY_DATA_PROVIDER_H */
