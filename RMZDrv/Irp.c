#include "Irp.h"

NTSTATUS rmzDispatchCreate(PDEVICE_OBJECT deviceObject, PIRP irp)
{
	UNREFERENCED_PARAMETER(deviceObject);
	UNREFERENCED_PARAMETER(irp);

	DbgPrint("*****Requiest create*******");
	return STATUS_SUCCESS;
}

NTSTATUS rmzDispatchClose(PDEVICE_OBJECT deviceObject, PIRP irp)
{
	UNREFERENCED_PARAMETER(deviceObject);
	UNREFERENCED_PARAMETER(irp);

	DbgPrint("*****Requiest close********");
	return STATUS_SUCCESS;
}

NTSTATUS rmzDispatchRead(PDEVICE_OBJECT deviceObject, PIRP irp)
{
	UNREFERENCED_PARAMETER(deviceObject);
	UNREFERENCED_PARAMETER(irp);

	DbgPrint("******Requiest read********");
	return STATUS_SUCCESS;
}

NTSTATUS rmzDispatchWrite(PDEVICE_OBJECT deviceObject, PIRP irp)
{
	UNREFERENCED_PARAMETER(deviceObject);
	UNREFERENCED_PARAMETER(irp);

	DbgPrint("******Requiest write*******");
	return STATUS_SUCCESS;
}