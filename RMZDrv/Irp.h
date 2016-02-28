#include <wdm.h>
#include <ntstrsafe.h>

_Dispatch_type_(IRP_MJ_CREATE) DRIVER_DISPATCH rmzDispatchCreate;
_Dispatch_type_(IRP_MJ_CLOSE) DRIVER_DISPATCH rmzDispatchClose;
_Dispatch_type_(IRP_MJ_READ) DRIVER_DISPATCH rmzDispatchRead;
_Dispatch_type_(IRP_MJ_WRITE) DRIVER_DISPATCH rmzDispatchWrite;