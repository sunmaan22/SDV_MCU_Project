/*
 * vehicle_data.h
 *
 * VehicleModelTask + VehicleDataRepository: the single-writer, mutex-guarded
 * store of cluster signals (Speed/RPM, Gear/READY, Warning) consumed by the
 * TouchGFX GUI. Producers (DummyDataProvider today, CanRxTask later) push
 * VehicleUpdateMsg entries; they never touch the repository or the GUI
 * directly. See docs/ecus/IVI/IVI_MPU_Dummy_TouchGFX_Guide.md sections 2-3.
 */
#ifndef VEHICLE_DATA_H
#define VEHICLE_DATA_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Per-signal display status. NO_DATA and INVALID/TIMEOUT are kept distinct
 * per the guide: "source invalid와 timeout을 구분한다." */
typedef enum
{
  SIGNAL_NO_DATA = 0, /* never received since boot */
  SIGNAL_VALID,
  SIGNAL_INVALID,     /* received, but source reported invalid */
  SIGNAL_TIMEOUT      /* received before, but stale beyond the group timeout */
} SignalStatus;

typedef enum
{
  WARNING_NONE = 0,
  WARNING_INFO,
  WARNING_WARNING,
  WARNING_CRITICAL
} WarningSeverity;

typedef struct
{
  int32_t speed_kmh;
  int32_t rpm;
  uint8_t received;
  uint8_t source_valid;
  uint32_t last_update_tick;
  SignalStatus status;
} DriveSignal;

typedef struct
{
  char gear;    /* 'P','R','N','D', or '-' when not yet known */
  uint8_t ready;
  uint8_t received;
  uint8_t source_valid;
  uint32_t last_update_tick;
  SignalStatus status;
} GearReadySignal;

typedef struct
{
  WarningSeverity severity;
  uint8_t active;
  uint8_t received;
  uint32_t last_update_tick;
  SignalStatus status;
} WarningSignal;

/* Snapshot handed to the GUI. Copied out under a bounded mutex wait so all
 * fields belong to the same update pass. */
typedef struct
{
  DriveSignal drive;
  GearReadySignal gear_ready;
  WarningSignal warning;
  uint8_t demo_source;      /* 1 while DummyDataProvider is the active source */
  uint32_t snapshot_version;
} VehicleDataSnapshot;

typedef enum
{
  VEHICLE_UPDATE_DRIVE = 0,
  VEHICLE_UPDATE_GEAR_READY,
  VEHICLE_UPDATE_WARNING
} VehicleUpdateType;

typedef struct
{
  VehicleUpdateType type;
  union
  {
    struct { int32_t speed_kmh; int32_t rpm; uint8_t source_valid; } drive;
    struct { char gear; uint8_t ready; uint8_t source_valid; } gear_ready;
    struct { WarningSeverity severity; uint8_t active; } warning;
  } payload;
} VehicleUpdateMsg;

/* Debugger-visible current state (Expressions/Live view), written only by
 * VehicleModelTask under the repository mutex. Mirrors the g_fdcan_loopback
 * convention already used for the FDCAN bench in Core/Src/freertos.c. */
extern VehicleDataSnapshot g_vehicle_data;

/* Creates the update queue, repository mutex, and VehicleModelTask. Call
 * once from main()'s USER CODE BEGIN RTOS_THREADS, before osKernelStart(),
 * and before any producer (e.g. DummyDataProvider_Create()) may run. */
void VehicleModel_Create(void);

/* Non-blocking push from a producer task. Returns 1 if enqueued, 0 if the
 * update queue was full — the caller counts that as an overflow/drop. */
uint8_t VehicleModel_PushUpdate(const VehicleUpdateMsg *msg);

/* Bounded-wait read of a consistent snapshot, intended for the GUI thread
 * (Model::tick()). Returns 1 on success, 0 if the lock could not be
 * acquired within wait_ms. */
uint8_t VehicleModel_GetSnapshot(VehicleDataSnapshot *out, uint32_t wait_ms);

#ifdef __cplusplus
}
#endif

#endif /* VEHICLE_DATA_H */
