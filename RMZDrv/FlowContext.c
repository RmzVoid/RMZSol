#include "FlowContext.h"

#define MAX_CONTEXTS 16

// increased on each connection
UINT64 currentFlowContext = 1;
const ULONG contextPoolTag = 'fubU';
const SIZE_T initialBufferSize = 4096;

// assume we have maximum 16 connections
RMZ_FLOW_CONTEXT Flows[MAX_CONTEXTS] = { 0 };
PKEVENT bufferReadyEvent[MAX_CONTEXTS];
PKWAIT_BLOCK waitBlock = NULL;
const ULONG waitBlockPoolTag = 'ptbW';
LARGE_INTEGER timeout = { 100 * 10 * 1000 * 10 };

void rmzInitFlows()
{
	for (int i = 0; i < MAX_CONTEXTS; i++)
	{
		bufferReadyEvent[i] = ExAllocatePoolWithTag(NonPagedPool, sizeof(KEVENT), waitBlockPoolTag);

		if (bufferReadyEvent[i])
			KeInitializeEvent(bufferReadyEvent[i], SynchronizationEvent, FALSE);
		else
			DbgPrint("Error allocating memory to bufferReadyEvent[%d]", i);
	}

	waitBlock = ExAllocatePoolWithTag(NonPagedPool, sizeof(KWAIT_BLOCK) * MAX_CONTEXTS, waitBlockPoolTag);
}

// signals dispatcher about data appeared in buffer
void rmzSignalBufferReady(PRMZ_FLOW_CONTEXT context)
{
	KeSetEvent(bufferReadyEvent[context->position], IO_NO_INCREMENT, FALSE);
}

// waits for data appeared in buffer
PRMZ_FLOW_CONTEXT rmzWaitForBufferReady()
{
	DbgPrint("Just before KeWaitForMultipleObjects\r\n");

	NTSTATUS status = KeWaitForMultipleObjects(MAX_CONTEXTS, bufferReadyEvent, WaitAny, Executive, KernelMode, FALSE, NULL, waitBlock);

	DbgPrint("WaitForBuffer ready status: %u\r\n", status);

	if (status >= STATUS_WAIT_0 && status <= STATUS_WAIT_63)
		return &Flows[status];

	return NULL;
}

PRMZ_FLOW_CONTEXT rmzAllocateFlowContext(
	UINT64 flowId, UINT16 layerId, UINT32 calloutId,
	UINT32 localAddress, UINT16 localPort,
	UINT32 remoteAddress, UINT16 remotePort)
{
	for (int i = 0; i < MAX_CONTEXTS; i++)
		if (!Flows[i].occupied)
		{
			Flows[i].position = i;
			Flows[i].occupied = TRUE;
			Flows[i].associated = FALSE;
			Flows[i].flowId = flowId;
			Flows[i].layerId = layerId;
			Flows[i].calloutId = calloutId;
			Flows[i].localAddress = localAddress;
			Flows[i].localPort = localPort;
			Flows[i].remoteAddress = remoteAddress;
			Flows[i].remotePort = remotePort;
			Flows[i].buffer.dataSize = 0;
			Flows[i].buffer.size = initialBufferSize;
			Flows[i].buffer.data = ExAllocatePoolWithTag(NonPagedPool, initialBufferSize, contextPoolTag);
			Flows[i].flowContext = currentFlowContext++;
			KeInitializeMutex(&Flows[i].buffer.lock, 0);
			return &Flows[i];
		}

	return NULL;
}

void rmzFreeFlowContext(UINT64 flowContext)
{
	for (int i = 0; i < MAX_CONTEXTS; i++)
		if (Flows[i].flowContext == flowContext)
		{
			Flows[i].occupied = FALSE;
			//
			// magic here
			KeReleaseMutex(&Flows[i].buffer.lock, TRUE);
			KeWaitForSingleObject(&Flows[i].buffer.lock, Executive, KernelMode, FALSE, 0);
		}
}

NTSTATUS rmzAssociateFlowContext(PRMZ_FLOW_CONTEXT context)
{
	// TODO: first remove context, then associate
	if (context->associated)
		return STATUS_OBJECT_NAME_EXISTS;

	NTSTATUS status = FwpsFlowAssociateContext(context->flowId, context->layerId, context->calloutId, context->flowContext);

	if (NT_SUCCESS(status))
		context->associated = TRUE;

	return status;
}

NTSTATUS rmzRemoveFlowContext(PRMZ_FLOW_CONTEXT context)
{
	if (context->associated)
		return FwpsFlowRemoveContext(context->flowId, context->layerId, context->calloutId);

	return STATUS_UNSUCCESSFUL;
}

// TODO: check for possible errors
void rmzRemoveAllFlowContexts()
{
	NTSTATUS status = STATUS_SUCCESS;

	for (int i = 0; i < MAX_CONTEXTS; i++)
		status = rmzRemoveFlowContext(&Flows[i]);
}

void rmzPrintContext(PRMZ_FLOW_CONTEXT context)
{
	DbgPrint("Flow:\r\n");
	DbgPrint("   occupied: %s\r\n", context->occupied ? "true" : "false");
	DbgPrint("   associated: %s\r\n", context->associated ? "true" : "false");
	DbgPrint("   flow id: %llu\r\n", context->flowId);
	DbgPrint("   layer id: %hu\r\n", context->layerId);
	DbgPrint("   callout id: %u\r\n", context->calloutId);
	DbgPrint("   context: %llu\r\n", context->flowContext);
	DbgPrint("   local addr: %u.%u.%u.%u:%hu\r\n",
		(context->localAddress & 0xFF000000) >> 24,
		(context->localAddress & 0x00FF0000) >> 16,
		(context->localAddress & 0x0000FF00) >> 8,
		(context->localAddress & 0x000000FF),
		context->localPort);
	DbgPrint("   remote addr: %u.%u.%u.%u:%hu\r\n",
		(context->remoteAddress & 0xFF000000) >> 24,
		(context->remoteAddress & 0x00FF0000) >> 16,
		(context->remoteAddress & 0x0000FF00) >> 8,
		(context->remoteAddress & 0x000000FF),
		context->remotePort);
}

PRMZ_FLOW_CONTEXT rmzGetFlowContext(UINT64 flowContext)
{
	for (int i = 0; i < MAX_CONTEXTS; i++)
	{
		if (Flows[i].flowContext == flowContext)
			return &Flows[i];
	}

	return NULL;
}

PVOID rmzPrepareDataBuffer(PRMZ_FLOW_CONTEXT context, SIZE_T size)
{
	//
	// Check if buffer enought size to add data
	SIZE_T sizeNeeded = context->buffer.dataSize + size;

	//
	// Expand buffer
	if (sizeNeeded > context->buffer.size)
	{
		PUINT8 newBuffer = ExAllocatePoolWithTag(NonPagedPool, sizeNeeded * 2, contextPoolTag);

		if (!newBuffer)
		{
			DbgPrint("Not enought memory for data. Allocated %lld, needed %lld, tried to allocate %lld", context->buffer.size, sizeNeeded, sizeNeeded * 2);
			return NULL;
		}

		RtlCopyMemory(newBuffer, context->buffer.data, context->buffer.dataSize);
		ExFreePoolWithTag(context->buffer.data, contextPoolTag);
		context->buffer.data = newBuffer;
		context->buffer.size = sizeNeeded * 2;
	}

	return context->buffer.data + context->buffer.dataSize;
}

// get context by flow context id
PRMZ_FLOW_CONTEXT getFlowContext(UINT64 flowContext)
{
	for (int i = 0; i < MAX_CONTEXTS; i++)
	{
		if (Flows[i].flowContext == flowContext)
			return &Flows[i];
	}

	return NULL;
}

// lock data buffer
void rmzLockDataBuffer(PRMZ_FLOW_CONTEXT context)
{
	KeWaitForSingleObject(&context->buffer.lock, Executive, KernelMode, FALSE, NULL);
}

// unlock data buffer
void rmzUnlockDataBuffer(PRMZ_FLOW_CONTEXT context)
{
	KeReleaseMutex(&context->buffer.lock, FALSE);
}

// move first N bytes from buffer to specified location
SIZE_T rmzMoveBufferData(PRMZ_FLOW_CONTEXT context, PVOID buffer, SIZE_T bufferSize, SIZE_T* bytesMoved)
{
	SIZE_T bytesToMove = bufferSize > context->buffer.dataSize ? context->buffer.dataSize : bufferSize;

	DbgPrint("Moving data to user buffer\r\n");
	DbgPrint(" User buffer size: %lld\r\n", bufferSize);
	DbgPrint(" Flow data size: %lld\r\n", context->buffer.dataSize);
	DbgPrint(" Bytes to move: %lld\r\n", bytesToMove);

	RtlCopyMemory(buffer, context->buffer.data, bytesToMove);

	//
	// if moved not all content of buffer
	if (bytesToMove < context->buffer.dataSize)
	{
		// move remainig data to begin of buffer
		RtlMoveMemory(context->buffer.data, context->buffer.data + bytesToMove, context->buffer.dataSize - bytesToMove);
	}

	context->buffer.dataSize -= bytesToMove;

	DbgPrint(" New flow data size: %lld\r\n", context->buffer.dataSize);

	*bytesMoved = bytesToMove;

	// return remain data size
	return context->buffer.dataSize;
}
