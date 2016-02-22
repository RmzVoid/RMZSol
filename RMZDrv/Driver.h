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

//#include "device.h"
#include "..\RMZCommon\common.h"

//
// WDFDRIVER Events
//

DRIVER_INITIALIZE DriverEntry;

