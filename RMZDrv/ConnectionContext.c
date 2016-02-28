#include "ConnectionContext.h"
#include "Util.h"

ULONG tag = 'CCtg';
PACKET_QUEUE gQueue;
LONG64 serial;

//
//
//
void RmzInitQueue()
{
	serial = 0;
	KeInitializeSpinLock(&gQueue.lock);
	KeInitializeEvent(&gQueue.event, NotificationEvent, FALSE);
	InitializeListHead(&gQueue.packets);
}

//
// Adds streamData to queue
//
void RmzQueuePacket(UINT64 flowId, FWPS_STREAM_DATA* stream)
{
	//
	// copy stream data to new NBL
	NET_BUFFER_LIST* nbl = NULL;
	FwpsCloneStreamData0(stream, NULL, NULL, 0, &nbl);

	//
	// allocate memory
	PPACKET packet = ExAllocatePoolWithTag(NonPagedPool, sizeof(PACKET), tag);
	if (!packet) return;

	packet->stream = ExAllocatePoolWithTag(NonPagedPool, sizeof(FWPS_STREAM_DATA), tag);
	if (!packet->stream)
	{
		ExFreePoolWithTag(packet, tag);
		return;
	}

	//
	// fill structure
	FWPS_STREAM_DATA* ns = packet->stream;

	packet->flowId = flowId;
	packet->serial = InterlockedIncrement64(&serial);

	ns->dataLength = stream->dataLength;
	ns->netBufferListChain = nbl;
	ns->dataOffset.netBuffer = nbl->FirstNetBuffer;
	ns->dataOffset.mdl = NET_BUFFER_CURRENT_MDL(nbl->FirstNetBuffer);
	ns->dataOffset.mdlOffset = NET_BUFFER_CURRENT_MDL_OFFSET(nbl->FirstNetBuffer);
	ns->dataOffset.netBufferList = nbl;
	ns->dataOffset.netBufferOffset = nbl->FirstNetBuffer->DataOffset;
	ns->dataOffset.streamDataOffset = stream->dataOffset.streamDataOffset;
	ns->flags = stream->flags;

	//
	// insert initialized packet to queue
	ExInterlockedInsertTailList(&gQueue.packets, &packet->list, &gQueue.lock);

	//
	// fire event to notify dispatcher
	RmzNotifyQueueNotEmpty();
}

void RmzFreePacket(PPACKET packet)
{
	FwpsFreeCloneNetBufferList(packet->stream->netBufferListChain, 0);
	ExFreePoolWithTag(packet->stream, tag);
	ExFreePoolWithTag(packet, tag);
}

PPACKET RmzPopPacket()
{
	return (PPACKET)ExInterlockedRemoveHeadList(&gQueue.packets, &gQueue.lock);
}

void RmzNotifyQueueNotEmpty()
{
	KeSetEvent(&gQueue.event, IO_NO_INCREMENT, FALSE);
}

BOOL RmzWaitOnQueue()
{
	return STATUS_WAIT_0 == KeWaitForSingleObject(&gQueue.event, Executive, KernelMode, FALSE, NULL);
}