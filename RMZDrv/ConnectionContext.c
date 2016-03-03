#include "ConnectionContext.h"
#include "Util.h"

ULONG tag = 'CCtg';
PACKET_QUEUE gQueue;
LONG64 serial;
LARGE_INTEGER waitQueueTimeout;

//
// This list must have ability to
// remove entry at any position,
// thus we can't use interlocked
// funtions on it
FLOW_LIST gFlowList;

//
//
//
void RmzInitQueue()
{
	serial = 0;
	KeInitializeSpinLock(&gQueue.lock);
	KeInitializeEvent(&gQueue.event, NotificationEvent, FALSE);
	InitializeListHead(&gQueue.packets);
	waitQueueTimeout.QuadPart = -200 * 1000 * 10;
}

//
// Adds streamData to queue
//
void RmzQueuePacket(UINT64 flowId, SOURCE source, FWPS_STREAM_DATA* stream)
{
	//
	// copy stream data to new NBL
	NET_BUFFER_LIST* nbl = NULL;
	if (stream)
		FwpsCloneStreamData0(stream, NULL, NULL, 0, &nbl);

	//
	// allocate memory
	PPACKET packet = ExAllocatePoolWithTag(NonPagedPool, sizeof(PACKET), tag);
	if (!packet) return;

	if (stream)
	{
		packet->stream = ExAllocatePoolWithTag(NonPagedPool, sizeof(FWPS_STREAM_DATA), tag);
		if (!packet->stream)
		{
			ExFreePoolWithTag(packet, tag);
			return;
		}

		FWPS_STREAM_DATA* ns = packet->stream;

		ns->dataLength = stream->dataLength;
		ns->netBufferListChain = nbl;
		ns->dataOffset.netBuffer = nbl->FirstNetBuffer;
		ns->dataOffset.mdl = NET_BUFFER_CURRENT_MDL(nbl->FirstNetBuffer);
		ns->dataOffset.mdlOffset = NET_BUFFER_CURRENT_MDL_OFFSET(nbl->FirstNetBuffer);
		ns->dataOffset.netBufferList = nbl;
		ns->dataOffset.netBufferOffset = nbl->FirstNetBuffer->DataOffset;
		ns->dataOffset.streamDataOffset = stream->dataOffset.streamDataOffset;
		ns->flags = stream->flags;
	}
	else
		packet->stream = NULL;

	//
	// fill structure
	packet->flowId = flowId;
	packet->serial = InterlockedIncrement64(&serial);
	packet->source = source;

	//
	// insert initialized packet to queue
	ExInterlockedInsertTailList(&gQueue.packets, &packet->list, &gQueue.lock);

	//
	// fire event to notify dispatcher
	RmzNotifyQueueNotEmpty();
}

void RmzFreePacket(PPACKET packet)
{
	//
	// assume packet never equal NULL
	if (packet->stream)
	{
		FwpsFreeCloneNetBufferList(packet->stream->netBufferListChain, 0);
		ExFreePoolWithTag(packet->stream, tag);
	}

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
	//
	// here we wait then packet arrive
	// just this happen we clear event
	// while I dont understand how IRP works
	// I set timeout to not block dispatcher forever
	NTSTATUS status = KeWaitForSingleObject(&gQueue.event, Executive, KernelMode, FALSE, &waitQueueTimeout);
	KeClearEvent(&gQueue.event);
	return STATUS_WAIT_0 == status;
}

BOOL RmzIsQueueEmpty()
{
	return gQueue.packets.Flink == &gQueue.packets;
}

void RmzFreeQueue()
{
	PPACKET packet = RmzPopPacket();

	while (packet)
	{
		RmzFreePacket(packet);
		packet = RmzPopPacket();
	}
}

void RmzInitFlowList()
{
	KeInitializeSpinLock(&gFlowList.lock);
	InitializeListHead(&gFlowList.flows);
}

PFLOW RmzAddFlow(UINT64 flowId, UINT32 calloutId)
{
	PFLOW flow = (PFLOW)ExAllocatePoolWithTag(NonPagedPool, sizeof(FLOW), tag);

	if (flow == NULL)
		return NULL;

	flow->flowId = flowId;
	flow->calloutId = calloutId;
		
	NTSTATUS status = FwpsFlowAssociateContext(flowId, FWPS_LAYER_STREAM_V4, calloutId, (UINT64)flow);

	if (!CheckStatus(status, "FwpsFlowAssociateContext"))
	{
		ExFreePoolWithTag(flow, tag);
		return NULL;
	}

	//
	// insert flow into the list
	KLOCK_QUEUE_HANDLE queueHandle;
	KeAcquireInStackQueuedSpinLock(&gFlowList.lock, &queueHandle);
	InsertTailList(&gFlowList.flows, &flow->list);
	KeReleaseInStackQueuedSpinLock(&queueHandle);

	return flow;
}

void RmzRemoveFlow(PFLOW flow)
{
	//
	// remove entry
	KLOCK_QUEUE_HANDLE queueHandle;
	KeAcquireInStackQueuedSpinLock(&gFlowList.lock, &queueHandle);
	RemoveEntryList((PLIST_ENTRY)flow);
	KeReleaseInStackQueuedSpinLock(&queueHandle);

	//
	// free memory
	ExFreePoolWithTag(flow, tag);
}

void RmzDeassociateFlows()
{
	//
	// lock list
	KLOCK_QUEUE_HANDLE queueHandle;
	KeAcquireInStackQueuedSpinLock(&gFlowList.lock, &queueHandle);

	PLIST_ENTRY head = &gFlowList.flows;
	PLIST_ENTRY entry = head->Flink;

	//
	// while not end of list reached
	// list reached then forward link points
	// to list's head
	while (entry->Flink != head)
	{
		PFLOW flow = CONTAINING_RECORD(entry, FLOW, list);
		FwpsFlowRemoveContext(flow->flowId, FWPS_LAYER_STREAM_V4, flow->calloutId);

		entry = entry->Flink;
	}

	KeReleaseInStackQueuedSpinLock(&queueHandle);
}