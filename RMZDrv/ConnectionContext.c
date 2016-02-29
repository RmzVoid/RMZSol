#include "ConnectionContext.h"
#include "Util.h"

ULONG tag = 'CCtg';
PACKET_QUEUE gQueue;
LONG64 serial;

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

void RmzInitFlowList()
{
	KeInitializeSpinLock(&gFlowList.lock);
	InitializeListHead(&gFlowList.flows);
}

void RmzFreeFlowList()
{
	//
	// remove entry from list
	KLOCK_QUEUE_HANDLE queueHandle;
	KeAcquireInStackQueuedSpinLock(&gFlowList.lock, &queueHandle);

	PLIST_ENTRY head = &gFlowList.flows;

	while (!IsListEmpty(head))
	{
		PFLOW flow = (PFLOW)RemoveHeadList(head);

		// deassociate flow
		FwpsFlowRemoveContext(flow->flowId, FWPS_LAYER_STREAM_V4, flow->calloutId);

		// free memory
		ExFreePoolWithTag(flow, tag);
	}

	KeReleaseInStackQueuedSpinLock(&queueHandle);
}

BOOL RmzWaitOnQueue()
{
	return STATUS_WAIT_0 == KeWaitForSingleObject(&gQueue.event, Executive, KernelMode, FALSE, NULL);
}

PFLOW RmzAssociateFlow(UINT64 flowId, UINT32 calloutId)
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

void RmzDeassociateFlow(PFLOW flow)
{
	//
	// remove entry from list
	KLOCK_QUEUE_HANDLE queueHandle;
	KeAcquireInStackQueuedSpinLock(&gFlowList.lock, &queueHandle);
	RemoveEntryList((PLIST_ENTRY)flow);
	KeReleaseInStackQueuedSpinLock(&queueHandle);

	// deassciate flow
	FwpsFlowRemoveContext(flow->flowId, FWPS_LAYER_STREAM_V4, flow->calloutId);

	// free memory
	ExFreePoolWithTag(flow, tag);
}