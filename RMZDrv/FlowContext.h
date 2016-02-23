#pragma once

#define NDIS_SUPPORT_NDIS6 1

#include <basetsd.h>

#pragma warning(push)
#pragma warning(disable:4201)       // unnamed struct/union
#include <fwpsk.h>
#pragma warning(pop)

typedef struct _RMZ_FLOW_CONTEXT
{
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
