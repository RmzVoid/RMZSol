#include "Driver.h"
#include "Util.h"
#include "FlowContext.h"
#include "NetBuffer.h"
#include "Irp.h"

NTSTATUS DriverEntry(_In_ PDRIVER_OBJECT driverObject, _In_ PUNICODE_STRING registryPath);

#ifdef ALLOC_PRAGMA
#pragma alloc_text (INIT, DriverEntry)
#endif

// forward declarations
void NTAPI ClassifyFnConnect(
	const FWPS_INCOMING_VALUES0* inFixedValues,
	const FWPS_INCOMING_METADATA_VALUES0* inMetaValues,
	void* layerData,
	const void* classifyContext,
	const FWPS_FILTER* filter,
	UINT64 flowContext,
	FWPS_CLASSIFY_OUT0* classifyOut);

void NTAPI ClassifyFnStream(
	const FWPS_INCOMING_VALUES0* inFixedValues,
	const FWPS_INCOMING_METADATA_VALUES0* inMetaValues,
	void* layerData,
	const void* classifyContext,
	const FWPS_FILTER* filter,
	UINT64 flowContext,
	FWPS_CLASSIFY_OUT0* classifyOut);

NTSTATUS NTAPI NotifyFn(
	FWPS_CALLOUT_NOTIFY_TYPE notifyType,
	const GUID* filterKey,
	FWPS_FILTER* filter);

void NTAPI FlowDeleteFn(
	UINT16 layerId,
	UINT32 calloutId,
	UINT64 flowContext);

void Unload(
	PDRIVER_OBJECT driverObject);

BOOL CheckStatus(
	NTSTATUS status,
	PCSTR message);

//
// Global variables
//
UINT32 CalloutConnectId;
UINT32 CalloutStreamId;
PDEVICE_OBJECT DeviceObject = NULL;
LPCWSTR wstrDeviceName = L"\\Device\\rmzdrv";
LPCWSTR wstrSymlinkName = L"\\??\\rmzdrv";
UNICODE_STRING deviceName;
UNICODE_STRING symlinkName;

//
// Entry point for dtiver
//
NTSTATUS DriverEntry(_In_ PDRIVER_OBJECT driverObject, _In_ PUNICODE_STRING registryPath)
{
    NTSTATUS status;
	FWPS_CALLOUT calloutConnect = { 0 };
	FWPS_CALLOUT calloutStream = { 0 };
	
	/* Specify unload handler */
	driverObject->DriverUnload = Unload;

	/* Dispatchers */
	driverObject->MajorFunction[IRP_MJ_CREATE] = rmzDispatchCreate;
	driverObject->MajorFunction[IRP_MJ_CLOSE] = rmzDispatchClose;
	driverObject->MajorFunction[IRP_MJ_READ] = rmzDispatchRead;
	driverObject->MajorFunction[IRP_MJ_WRITE] = rmzDispatchWrite;

	/* Give a name for our device */
	RtlUnicodeStringInit(&deviceName, wstrDeviceName);
	RtlUnicodeStringInit(&symlinkName, wstrSymlinkName);

	/* Create i/o device */
	status = IoCreateDevice(driverObject, 0, &deviceName, FILE_DEVICE_UNKNOWN, FILE_DEVICE_SECURE_OPEN, FALSE, &DeviceObject);

	if (!CheckStatus(status, "IoCreateDevice")) goto exit;

	DeviceObject->Flags |= DO_BUFFERED_IO;

	/* Create symbolic link, to allow open device as file from user space */
	status = IoCreateSymbolicLink(&symlinkName, &deviceName);

	CheckStatus(status, "IoCreateSymbolicLink");

	/* Register connect callout */
	calloutConnect.calloutKey = rmzCalloutConnectGuid;
	calloutConnect.classifyFn = ClassifyFnConnect;
	calloutConnect.notifyFn = NotifyFn;

	status = FwpsCalloutRegister(DeviceObject, &calloutConnect, &CalloutConnectId);

	if (!CheckStatus(status, "FwpsCalloutRegister(calloutConnect)")) goto exit;

	/* Register stream callout */
	calloutStream.calloutKey = rmzCalloutStreamGuid;
	calloutStream.classifyFn = ClassifyFnStream;
	calloutStream.notifyFn = NotifyFn;
	calloutStream.flowDeleteFn = FlowDeleteFn;
	calloutStream.flags = FWP_CALLOUT_FLAG_CONDITIONAL_ON_FLOW;

	status = FwpsCalloutRegister(DeviceObject, &calloutStream, &CalloutStreamId);

	if (!CheckStatus(status, "FwpsCalloutRegister(calloutStream)")) goto exit;

exit:
    return status;

	UNREFERENCED_PARAMETER(registryPath);
}

void Unload(PDRIVER_OBJECT driverObject)
{
	NTSTATUS status;

	// unregister connect callout
	status = FwpsCalloutUnregisterById(CalloutConnectId);
	CheckStatus(status, "FwpsCalloutUnregisterById(CalloutConnectId)");

	// unregister stream callout
	status = FwpsCalloutUnregisterById(CalloutStreamId);

	if (status == STATUS_DEVICE_BUSY)
	{
		DbgPrint("STATUS_DEVICE_BUSY, removing all contexts first\r\n");
		// removing associations force calling flowDeleteFn, there context data actually frees
		rmzRemoveAllFlowContexts();
		// try to unregister again
		status = FwpsCalloutUnregisterById(CalloutStreamId);
	}

	CheckStatus(status, "FwpsCalloutUnregisterById(CalloutStreamId)");

	status = IoDeleteSymbolicLink(&symlinkName);

	CheckStatus(status, "IoDeleteSymbolicLink");

	IoDeleteDevice(DeviceObject);

	UNREFERENCED_PARAMETER(driverObject);
}

BOOL CheckStatus(NTSTATUS status, PCSTR message)
{
	if (!NT_SUCCESS(status))
	{
		DbgPrint( "%s failed: 0x%X\r\n", message, status);
		return FALSE;
	}
	else
	{
		DbgPrint("%s success\r\n", message);
		return TRUE;
	}
}

void NTAPI ClassifyFnConnect(
	const FWPS_INCOMING_VALUES0* inFixedValues,
	const FWPS_INCOMING_METADATA_VALUES0* inMetaValues,
	void* layerData,
	const void* classifyContext,
	const FWPS_FILTER* filter,
	UINT64 flowContext,
	FWPS_CLASSIFY_OUT0* classifyOut)
{
	NTSTATUS status;
	PRMZ_FLOW_CONTEXT flow;

	UNREFERENCED_PARAMETER(layerData);
	UNREFERENCED_PARAMETER(classifyContext);

	DbgPrint("I am in 'CONNECT' callout\r\n");

	if (inFixedValues->layerId == FWPS_LAYER_ALE_FLOW_ESTABLISHED_V4)
	{
		DbgPrint("Flow established v4 layer\r\n");

		DbgPrint("Flow context %d\r\n", flowContext);
		DbgPrint("Raw context %d\r\n", filter->context);
		DbgPrint("Values count %d\r\n", inFixedValues->valueCount);


		if (FWPS_IS_METADATA_FIELD_PRESENT(inMetaValues, FWPS_METADATA_FIELD_FLOW_HANDLE))
		{
			DbgPrint("Flow handle field present %d\r\n", inMetaValues->flowHandle);

			flow = rmzAllocateFlowContext(
				inMetaValues->flowHandle,
				FWPS_LAYER_STREAM_V4,
				CalloutStreamId,
				inFixedValues->incomingValue[FWPS_FIELD_ALE_FLOW_ESTABLISHED_V4_IP_LOCAL_ADDRESS].value.uint32,
				inFixedValues->incomingValue[FWPS_FIELD_ALE_FLOW_ESTABLISHED_V4_IP_LOCAL_PORT].value.uint16,
				inFixedValues->incomingValue[FWPS_FIELD_ALE_FLOW_ESTABLISHED_V4_IP_REMOTE_ADDRESS].value.uint32,
				inFixedValues->incomingValue[FWPS_FIELD_ALE_FLOW_ESTABLISHED_V4_IP_REMOTE_PORT].value.uint16
				);

			status = rmzAssociateFlowContext(flow);
			CheckStatus(status, "FwpsFlowAssociateContext");
			rmzPrintContext(flow);
		}
	}

	if (filter->flags & FWPS_FILTER_FLAG_CLEAR_ACTION_RIGHT)
	{
		classifyOut->rights &= ~FWPS_RIGHT_ACTION_WRITE;
	}

	classifyOut->actionType = FWP_ACTION_PERMIT;
}

void NTAPI ClassifyFnStream(
	const FWPS_INCOMING_VALUES0* inFixedValues,
	const FWPS_INCOMING_METADATA_VALUES0* inMetaValues,
	void* layerData,
	const void* classifyContext,
	const FWPS_FILTER* filter,
	UINT64 flowContext,
	FWPS_CLASSIFY_OUT0* classifyOut)
{
	UNREFERENCED_PARAMETER(inMetaValues);
	UNREFERENCED_PARAMETER(flowContext);
	UNREFERENCED_PARAMETER(filter);
	UNREFERENCED_PARAMETER(classifyContext);

	FWPS_STREAM_CALLOUT_IO_PACKET* packet = layerData;
	FWPS_STREAM_DATA* streamData = packet->streamData;

	DbgPrint("I am in 'STREAM' callout\r\n");

	if (inFixedValues->layerId == FWPS_LAYER_STREAM_V4)
	{
		DbgPrint("Stream v4 layer\r\n");
		DbgPrint("Direction: %s\r\n", streamData->flags & FWPS_STREAM_FLAG_RECEIVE ? "inbound" : streamData->flags & FWPS_STREAM_FLAG_SEND ? "outbound" : "unknown");
		DbgPrint("Local address: ");
		rmzPrintIpAddr(inFixedValues->incomingValue[FWPS_FIELD_STREAM_V4_IP_LOCAL_ADDRESS].value.uint32);
		DbgPrint(":%hu\r\n", inFixedValues->incomingValue[FWPS_FIELD_STREAM_V4_IP_LOCAL_PORT].value.uint16);

		DbgPrint("Remote address: ");
		rmzPrintIpAddr(inFixedValues->incomingValue[FWPS_FIELD_STREAM_V4_IP_REMOTE_ADDRESS].value.uint32);
		DbgPrint(":%hu\r\n", inFixedValues->incomingValue[FWPS_FIELD_STREAM_V4_IP_REMOTE_PORT].value.uint16);

		if (layerData != NULL)
		{
			DbgPrint("FWPS_STREAM_CALLOUT_IO_PACKET:\r\n");
			DbgPrint("   countBytesEnforced: %u\r\n", packet->countBytesEnforced);
			DbgPrint("   countBytesRequired: %u\r\n", packet->countBytesRequired);
			DbgPrint("   countMissedBytes: %u\r\n", packet->missedBytes);
			DbgPrint("   streamAction: %u\r\n", packet->streamAction);
			DbgPrint("FWPS_STREAM_DATA:\r\n");
			DbgPrint("   dataLength: %u\r\n", streamData->dataLength);
			DbgPrint("   flags: 0x%X\r\n", streamData->flags);
			
			rmzPrintNetBufferList(packet->streamData->netBufferListChain);
		}
		
		classifyOut->actionType = FWP_ACTION_PERMIT;
	}
}

NTSTATUS NotifyFn(
	FWPS_CALLOUT_NOTIFY_TYPE notifyType,
	const GUID* filterKey,
	FWPS_FILTER* filter)
{
	UNREFERENCED_PARAMETER(filterKey);

	switch (notifyType)
	{
	case FWPS_CALLOUT_NOTIFY_ADD_FILTER:
		DbgPrint("Filter added %d\r\n", filter->filterId);
		break;

	case FWPS_CALLOUT_NOTIFY_DELETE_FILTER:
		DbgPrint("Filter deleted %d\r\n", filter->filterId);
		break;

	default:
		DbgPrint("Unknown notify type %d\r\n", notifyType);
	}

	return STATUS_SUCCESS;
}

void FlowDeleteFn(
	UINT16 layerId,
	UINT32 calloutId,
	UINT64 flowContext)
{
	// TODO: this funtion can be called from different layers and callouts, so need to count layer and callout ids
	UNREFERENCED_PARAMETER(layerId);
	UNREFERENCED_PARAMETER(calloutId);

	DbgPrint("FlowDeleteFn %d\r\n", flowContext);

	rmzFreeFlowContext(flowContext);
}
