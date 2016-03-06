#include "Driver.h"
#include "Irp.h"
#include "ConnectionContext.h"
#include "Util.h"
#include "BinaryReader.h"
#include "BinaryWriter.h"

_Use_decl_annotations_
NTSTATUS rmzDispatchCreate(PDEVICE_OBJECT deviceObject, PIRP irp)
{
	UNREFERENCED_PARAMETER(deviceObject);

	irp->IoStatus.Status = STATUS_SUCCESS;
	irp->IoStatus.Information = 0;
	IoCompleteRequest(irp, IO_NO_INCREMENT);

	return STATUS_SUCCESS;
}

_Use_decl_annotations_
NTSTATUS rmzDispatchClose(PDEVICE_OBJECT deviceObject, PIRP irp)
{
	UNREFERENCED_PARAMETER(deviceObject);

	irp->IoStatus.Status = STATUS_SUCCESS;
	irp->IoStatus.Information = 0;
	IoCompleteRequest(irp, IO_NO_INCREMENT);

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

	UINT32 bytesMoved = 0;

	if (stackLocation && buffer && bufferSize > 0)
	{
		if (RmzWaitOnQueue())
		{
			PPACKET packet = RmzPopPacket();

			if (packet)
			{
				BINARYWRITER bw;
				RmzBwInit(&bw, buffer, bufferSize);
				RmzBwWriteUInt64(&bw, packet->flowId);
				RmzBwWriteUInt32(&bw, packet->source);
				RmzBwWriteUInt32(&bw, packet->dataSize);
				if (packet->data)
					RmzBwWriteBuffer(&bw, packet->dataSize, packet->data);

				bytesMoved = bw.currentPosition;

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

void NTAPI completionFn(_In_ void* context, _Inout_ PNET_BUFFER_LIST netBufferList, _In_ BOOLEAN dispatchLevel)
{
	UNREFERENCED_PARAMETER(context);
	UNREFERENCED_PARAMETER(dispatchLevel);

	PMDL mdl = context;
	FwpsFreeNetBufferList(netBufferList);

	if (mdl)
	{
		IoFreeMdl(mdl);
		ExFreePoolWithTag(mdl->MappedSystemVa, 'rwsD');
	}
}

_Use_decl_annotations_
NTSTATUS rmzDispatchWrite(PDEVICE_OBJECT deviceObject, PIRP irp)
{
	UNREFERENCED_PARAMETER(deviceObject);

	NTSTATUS status = STATUS_SUCCESS;
	PIO_STACK_LOCATION stackLocation = IoGetCurrentIrpStackLocation(irp);
	PNET_BUFFER_LIST nbl = NULL;
	MDL* mdl = NULL;

	//
	// in write dispatcher here we have data length from user app
	ULONG dataLength = stackLocation->Parameters.Write.Length;

	//
	// here buffer with data from user app
	PVOID buffer = irp->AssociatedIrp.SystemBuffer;

	if (stackLocation && buffer && dataLength > 0)
	{
		PACKET packet;
		BINARYREADER br;

		//
		// Fill injection structure
		RmzBrInit(&br, buffer, dataLength);
		packet.flowId = RmzBrReadUInt64(&br);
		packet.source = RmzBrReadUInt32(&br);
		packet.dataSize = RmzBrReadUInt32(&br);
		packet.data = RmzBrReadBuffer(&br, packet.dataSize, TRUE);

		//
		// allocate mdl with our data
		mdl = IoAllocateMdl(packet.data, packet.dataSize, FALSE, FALSE, NULL);

		if (!mdl)
		{
			status = STATUS_NO_MEMORY;
			goto exit;
		}

		MmBuildMdlForNonPagedPool(mdl);

		//
		// allocate net buffer list using our mdl
		status = FwpsAllocateNetBufferAndNetBufferList(NBLPoolHandle, 0, 0, mdl, 0, packet.dataSize, &nbl);

		if (!CheckStatus(status, "FwpsAllocateNetBufferAndNetBufferList"))
		{
			goto exit;
		}

		//
		// inject net buffer list to flow
		status = FwpsStreamInjectAsync(InjectionHandle, NULL, 0,
			packet.flowId, CalloutStreamId, FWPS_LAYER_STREAM_V4,
			packet.source,
			nbl, packet.dataSize,
			completionFn, mdl);

		if (!CheckStatus(status, "FwpsStreamInjectAsync"))
		{
			goto exit;
		}
	}

exit:
	irp->IoStatus.Status = status;
	irp->IoStatus.Information = dataLength;
	IoCompleteRequest(irp, IO_NO_INCREMENT);

	return STATUS_SUCCESS;
}