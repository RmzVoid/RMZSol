#include "Irp.h"
#include "ConnectionContext.h"

_Use_decl_annotations_
NTSTATUS rmzDispatchCreate(PDEVICE_OBJECT deviceObject, PIRP irp)
{
	UNREFERENCED_PARAMETER(deviceObject);
	irp->IoStatus.Status = STATUS_SUCCESS;
	irp->IoStatus.Information = 0;
	IoCompleteRequest(irp, IO_NO_INCREMENT);
	DbgPrint("*****Requiest create*******\r\n");
	return STATUS_SUCCESS;
}

_Use_decl_annotations_
NTSTATUS rmzDispatchClose(PDEVICE_OBJECT deviceObject, PIRP irp)
{
	UNREFERENCED_PARAMETER(deviceObject);
	irp->IoStatus.Status = STATUS_SUCCESS;
	irp->IoStatus.Information = 0;
	IoCompleteRequest(irp, IO_NO_INCREMENT);
	DbgPrint("*****Requiest close********\r\n");
	return STATUS_SUCCESS;
}

_Use_decl_annotations_
NTSTATUS rmzDispatchRead(PDEVICE_OBJECT deviceObject, PIRP irp)
{
	UNREFERENCED_PARAMETER(deviceObject);

	NTSTATUS status = STATUS_SUCCESS;

	PIO_STACK_LOCATION stackLocation = IoGetCurrentIrpStackLocation(irp);

	//
	// in read dispatcher here we have size buffer proded by user app
	ULONG bufferSize = stackLocation->Parameters.Read.Length;

	//
	// here we place data which will be readed by user app
	PVOID buffer = irp->AssociatedIrp.SystemBuffer;

	SIZE_T bytesMoved = 0;

	if (stackLocation && buffer && bufferSize > 0)
	{
		if (RmzWaitOnQueue())
		{
			PPACKET packet = RmzPopPacket();

			if (packet)
			{
				RtlCopyMemory(buffer, &packet->flowId, sizeof(UINT64) + sizeof(UINT64) + sizeof(SOURCE));
				bytesMoved = sizeof(UINT64) + sizeof(UINT64) + sizeof(SOURCE);
				RmzFreePacket(packet);

				// now, each read request consume only one packet
				// in future need to put all packets which
				// fits to buffer at once
				if (!RmzIsQueueEmpty())
					RmzNotifyQueueNotEmpty();
			}
		}
	}

	irp->IoStatus.Status = status;
	irp->IoStatus.Information = bytesMoved;	// Suka nu gde napisano 4to suda nado pihat' 4islo zapisannih bait!!!!
	IoCompleteRequest(irp, IO_NO_INCREMENT);

	return status;
}

_Use_decl_annotations_
NTSTATUS rmzDispatchWrite(PDEVICE_OBJECT deviceObject, PIRP irp)
{
	UNREFERENCED_PARAMETER(deviceObject);
	DbgPrint("******Requiest write*******\r\n");

	PIO_STACK_LOCATION stackLocation = IoGetCurrentIrpStackLocation(irp);

	// in write dispatcher here we have data length from user app
	ULONG dataLength = stackLocation->Parameters.Write.Length;
	// here buffer with data from user app
	PVOID buffer = irp->AssociatedIrp.SystemBuffer;

	if (stackLocation && buffer && dataLength > 0)
	{
		UNICODE_STRING string;
		LPWSTR pfff = (LPWSTR)ExAllocatePoolWithTag(NonPagedPool, dataLength + 2, 'rwsD');

		if (pfff)
		{
			RtlCopyMemory(pfff, buffer, dataLength);
			pfff[dataLength / 2] = 0;
			RtlUnicodeStringInit(&string, pfff);
			DbgPrint("Recieved data from app: %wZ\r\n", string);
			ExFreePoolWithTag(pfff, 'rwsD');
		}
	}

	irp->IoStatus.Status = STATUS_SUCCESS;
	irp->IoStatus.Information = dataLength;
	IoCompleteRequest(irp, IO_NO_INCREMENT);

	return STATUS_SUCCESS;
}