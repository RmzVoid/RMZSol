#define NDIS_SUPPORT_NDIS6 1

#include <ntddk.h>
#include <wdf.h>

#pragma warning(push)
#pragma warning(disable:4201)       // unnamed struct/union
#include <fwpsk.h>
#pragma warning(pop)

#include <fwpmk.h>

#define INITGUID
#include <guiddef.h>

#include "device.h"
#include "queue.h"
#include "trace.h"
#include "..\RMZCommon\common.h"

//
// WDFDRIVER Events
//

DRIVER_INITIALIZE DriverEntry;
EVT_WDF_DRIVER_DEVICE_ADD RMZDrvEvtDeviceAdd;
EVT_WDF_OBJECT_CONTEXT_CLEANUP RMZDrvEvtDriverContextCleanup;

