#pragma once

#define NDIS_SUPPORT_NDIS6 1

#include <basetsd.h>

#pragma warning(push)
#pragma warning(disable:4201)       // unnamed struct/union
#include <fwpsk.h>
#pragma warning(pop)

typedef struct _RMZ_FLOW_CONTEXT
{
	UINT32 position;
	BOOL occupied;
	BOOL associated;
	UINT64 flowContext;
	UINT64 flowId;
	UINT16 layerId;
	UINT32 calloutId;
	UINT32 localAddress;
	UINT32 remoteAddress;
	UINT16 localPort;
	UINT16 remotePort;
	struct
	{
		PUINT8 data;	// buffer will be sended to user app
		SIZE_T size;	// whole buffer size
		SIZE_T dataSize;	// data size
		KMUTEX lock;	// lock the buffer
	} buffer;
} RMZ_FLOW_CONTEXT, *PRMZ_FLOW_CONTEXT;

// called then need to create context
PRMZ_FLOW_CONTEXT
rmzAllocateFlowContext(
	UINT64 flowId, UINT16 layerId, UINT32 calloutId,
	UINT32 localAddress, UINT16 localPort,
	UINT32 remoteAddress, UINT16 remotePort);

// called then flow should be freed (in flowDeleteFn)
void
rmzFreeFlowContext(UINT64 flowContext);

// associates flow with context
NTSTATUS
rmzAssociateFlowContext(PRMZ_FLOW_CONTEXT context);

// remove flow-context assciation
NTSTATUS
rmzRemoveFlowContext(PRMZ_FLOW_CONTEXT context);

// remove all assciations, usually in Unload routine
void
rmzRemoveAllFlowContexts();

// print flow info to debugger
void
rmzPrintContext(PRMZ_FLOW_CONTEXT context);

// get context by flow context id
PRMZ_FLOW_CONTEXT
rmzGetFlowContext(UINT64 flowContext);

// lock data buffer
void
rmzLockDataBuffer(PRMZ_FLOW_CONTEXT context);

// unlock data buffer
void
rmzUnlockDataBuffer(PRMZ_FLOW_CONTEXT context);

// allocated buffer for future data and return pointer there data can be stored
PVOID
rmzPrepareDataBuffer(PRMZ_FLOW_CONTEXT context, SIZE_T size);

// move first N bytes from buffer to specified location, return count of remain bytes of data
SIZE_T
rmzMoveBufferData(PRMZ_FLOW_CONTEXT context, PVOID buffer, SIZE_T bufferSize, SIZE_T* bytesMoved);

// signals dispatcher about data appeared in buffer
void
rmzSignalBufferReady(PRMZ_FLOW_CONTEXT context);

// waits for data appeared in buffer
PRMZ_FLOW_CONTEXT
rmzWaitForBufferReady();

// initialize flow contexts stuff
void
rmzInitFlows();