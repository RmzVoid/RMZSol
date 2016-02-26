#include "Irp.h"

_Use_decl_annotations_
NTSTATUS rmzDispatchCreate(PDEVICE_OBJECT deviceObject, PIRP irp)
{
	UNREFERENCED_PARAMETER(deviceObject);
	UNREFERENCED_PARAMETER(irp);

	DbgPrint("*****Requiest create*******\r\n");
	return STATUS_SUCCESS;
}

_Use_decl_annotations_
NTSTATUS rmzDispatchClose(PDEVICE_OBJECT deviceObject, PIRP irp)
{
	UNREFERENCED_PARAMETER(deviceObject);
	UNREFERENCED_PARAMETER(irp);

	DbgPrint("*****Requiest close********\r\n");
	return STATUS_SUCCESS;
}

_Use_decl_annotations_
NTSTATUS rmzDispatchRead(PDEVICE_OBJECT deviceObject, PIRP irp)
{
	UNREFERENCED_PARAMETER(deviceObject);
	UNREFERENCED_PARAMETER(irp);

	DbgPrint("******Requiest read********\r\n");

	PIO_STACK_LOCATION stackLocation = IoGetCurrentIrpStackLocation(irp);
	ULONG bufferSize = stackLocation->Parameters.Read.Length;
	PVOID buffer = irp->AssociatedIrp.SystemBuffer;

	if (stackLocation && buffer && bufferSize > 0)
	{
		DbgPrint("System buffer length: %lu\r\n", bufferSize);
		DbgPrint("System buffer pointer: %p\r\n", buffer);
		RtlCopyMemory(irp->AssociatedIrp.SystemBuffer, L"Hello from driver!!!", 42);
	}

	irp->IoStatus.Status = STATUS_SUCCESS;
	irp->IoStatus.Information = 42;	// Suka nu gde napisano 4to suda nado pihat' 4islo zapisannih bait!!!!

	IoCompleteRequest(irp, IO_NO_INCREMENT);


	return STATUS_SUCCESS;
}

_Use_decl_annotations_
NTSTATUS rmzDispatchWrite(PDEVICE_OBJECT deviceObject, PIRP irp)
{
	UNREFERENCED_PARAMETER(deviceObject);
	UNREFERENCED_PARAMETER(irp);

	DbgPrint("******Requiest write*******\r\n");
	return STATUS_SUCCESS;
}