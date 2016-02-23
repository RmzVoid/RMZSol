#include "FlowContext.h"

#define MAX_FLOW_CONTEXT 16

// increased on each connection
UINT64 currentFlowContext = 1;

// assume we have maximum 16 connections
RMZ_FLOW_CONTEXT Flows[MAX_FLOW_CONTEXT] = { 0 };

PRMZ_FLOW_CONTEXT rmzAllocateFlowContext(UINT64 flowId, UINT16 layerId, UINT32 calloutId)
{
	for (int i = 0; i < MAX_FLOW_CONTEXT; i++)
		if (!Flows[i].occupied)
		{
			Flows[i].occupied = TRUE;
			Flows[i].associated = FALSE;
			Flows[i].flowId = flowId;
			Flows[i].layerId = layerId;
			Flows[i].calloutId = calloutId;
			Flows[i].flowContext = currentFlowContext++;
			return &Flows[i];
		}

	return NULL;
}

void rmzFreeFlowContext(UINT64 flowContext)
{
	for (int i = 0; i < MAX_FLOW_CONTEXT; i++)
		if (Flows[i].flowContext == flowContext)
			Flows[i].occupied = FALSE;
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

	for (int i = 0; i < MAX_FLOW_CONTEXT; i++)
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
}