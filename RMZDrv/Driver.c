#include "Driver.h"
#include "Util.h"
#include "NetBuffer.h"
#include "Irp.h"
#include "ConnectionContext.h"

DRIVER_INITIALIZE DriverEntry;
DRIVER_UNLOAD DriverUnload;

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

//
// Global variables
//
UINT32 CalloutConnectId;
UINT32 CalloutStreamId;
HANDLE InjectionHandle;
NDIS_HANDLE NBLPoolHandle; 
ULONG NBLPoolTag = 'tlbN';
PDEVICE_OBJECT DeviceObject = NULL;
LPCWSTR wstrDeviceName = L"\\Device\\rmzdrv";
LPCWSTR wstrSymlinkName = L"\\??\\rmzdrv";
UNICODE_STRING deviceName = { 0 };
UNICODE_STRING symlinkName = { 0 };
LONG AppStarted = FALSE;

//
// Entry point for dtiver
//
NTSTATUS DriverEntry(_In_ PDRIVER_OBJECT driverObject, _In_ PUNICODE_STRING registryPath)
{
    NTSTATUS status;
	FWPS_CALLOUT calloutConnect = { 0 };
	FWPS_CALLOUT calloutStream = { 0 };
	
	/* Specify unload handler */
	driverObject->DriverUnload = DriverUnload;

	/* Dispatchers */
	driverObject->MajorFunction[IRP_MJ_CREATE] = rmzDispatchCreate;
	driverObject->MajorFunction[IRP_MJ_CLOSE] = rmzDispatchClose;
	driverObject->MajorFunction[IRP_MJ_READ] = rmzDispatchRead;
	driverObject->MajorFunction[IRP_MJ_WRITE] = rmzDispatchWrite;

	/* Give a name for our device */
	RtlInitUnicodeString(&deviceName, wstrDeviceName);
	RtlInitUnicodeString(&symlinkName, wstrSymlinkName);

	/* Create i/o device */
	status = IoCreateDevice(driverObject, 0, &deviceName, FILE_DEVICE_UNKNOWN, FILE_DEVICE_SECURE_OPEN, FALSE, &DeviceObject);

	if (!CheckStatus(status, "IoCreateDevice") || !DeviceObject) goto exit;

	DeviceObject->Flags |= DO_BUFFERED_IO;

	/* Init connection context */
	RmzInitQueue();

	/* Init flows list */
	RmzInitFlowList();

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

	/* Register injection handle */
	status = FwpsInjectionHandleCreate(AF_INET, FWPS_INJECTION_TYPE_STREAM, &InjectionHandle);

	if (!CheckStatus(status, "FwpsInjectionHandleCreate")) goto exit;

	/* Allocate net buffer list */
	NET_BUFFER_LIST_POOL_PARAMETERS nblPoolParameters = { 0 };
	nblPoolParameters.Header.Revision = NET_BUFFER_LIST_POOL_PARAMETERS_REVISION_1;
	nblPoolParameters.Header.Type = NDIS_OBJECT_TYPE_DEFAULT;
	nblPoolParameters.Header.Size = sizeof(nblPoolParameters);
	nblPoolParameters.ProtocolId = NDIS_PROTOCOL_ID_TCP_IP;
	nblPoolParameters.fAllocateNetBuffer = TRUE;
	nblPoolParameters.PoolTag = NBLPoolTag;
	nblPoolParameters.DataSize = 0;
	NBLPoolHandle = NdisAllocateNetBufferListPool(NULL, &nblPoolParameters);

	if (NBLPoolHandle == NULL) { DbgPrint("NdisAllocateNetBufferListPool failed"); goto exit; }

exit:
    return status;
	
	UNREFERENCED_PARAMETER(registryPath);
}

void DriverUnload(PDRIVER_OBJECT driverObject)
{
	NTSTATUS status;

	//
	// Destroy injection
	status = FwpsInjectionHandleDestroy0(InjectionHandle);
	CheckStatus(status, "FwpsInjectionHandleDestroy");

	//
	// Unregister connect callout
	status = FwpsCalloutUnregisterById(CalloutConnectId);
	CheckStatus(status, "FwpsCalloutUnregisterById(CalloutConnectId)");

	//
	// Unregister stream callout
	status = FwpsCalloutUnregisterById(CalloutStreamId);

	if (status == STATUS_DEVICE_BUSY)
	{
		//
		// Here we must only deassociate flows
		// all memory deallocation should be done
		// in FlowDeleteFn
		RmzDeassociateFlows();

		//
		// Try to unregister callout again
		status = FwpsCalloutUnregisterById(CalloutStreamId);
	}

	CheckStatus(status, "FwpsCalloutUnregisterById(CalloutStreamId)");

	//
	// Delete symbolic link to driver
	status = IoDeleteSymbolicLink(&symlinkName);
	CheckStatus(status, "IoDeleteSymbolicLink");

	//
	// Remove remaining packets from queue
	RmzFreeQueue();

	//
	// Delete device
	IoDeleteDevice(DeviceObject);

	UNREFERENCED_PARAMETER(driverObject);
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
	UNREFERENCED_PARAMETER(layerData);
	UNREFERENCED_PARAMETER(classifyContext);
	UNREFERENCED_PARAMETER(flowContext);

	if (AppStarted)
	{
		if (inFixedValues->layerId == FWPS_LAYER_ALE_FLOW_ESTABLISHED_V4)
		{
			RmzAddFlow(inMetaValues->flowHandle, CalloutStreamId);
			RmzQueuePacket(inMetaValues->flowHandle, NEWCONNECTION, NULL);
		}
	}

	if (filter->flags & FWPS_FILTER_FLAG_CLEAR_ACTION_RIGHT)
		classifyOut->rights &= ~FWPS_RIGHT_ACTION_WRITE;

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
	UNREFERENCED_PARAMETER(filter);
	UNREFERENCED_PARAMETER(classifyContext);
	UNREFERENCED_PARAMETER(flowContext);

	if (filter->flags & FWPS_FILTER_FLAG_CLEAR_ACTION_RIGHT)
		classifyOut->rights &= ~FWPS_RIGHT_ACTION_WRITE;

	if (!AppStarted)
	{
		classifyOut->actionType = FWP_ACTION_PERMIT;
		return;
	}

	if (layerData == NULL)
		return;

	FWPS_STREAM_CALLOUT_IO_PACKET* packet = layerData;
	FWPS_STREAM_DATA* streamData = packet->streamData;

	//
	// check if packet was injected before, just permit it
	FWPS_PACKET_INJECTION_STATE injectionState = FwpsQueryPacketInjectionState(InjectionHandle, streamData->netBufferListChain, NULL);

	if (injectionState == FWPS_PACKET_PREVIOUSLY_INJECTED_BY_SELF || injectionState == FWPS_PACKET_INJECTED_BY_SELF)
	{
		classifyOut->actionType = FWP_ACTION_PERMIT;
		return;
	}

	if (inFixedValues->layerId == FWPS_LAYER_STREAM_V4)
	{
		if (streamData->flags & FWPS_STREAM_FLAG_RECEIVE_DISCONNECT || streamData->flags & FWPS_STREAM_FLAG_SEND_DISCONNECT)
		{
			classifyOut->actionType = FWP_ACTION_PERMIT;
			RmzQueuePacket(inMetaValues->flowHandle, DISCONNECT, NULL);
		}
		else if (streamData->flags & FWPS_STREAM_FLAG_RECEIVE_EXPEDITED || streamData->flags & FWPS_STREAM_FLAG_SEND_EXPEDITED)
		{
			classifyOut->actionType = FWP_ACTION_PERMIT;
		}
		else
		{
			classifyOut->actionType = FWP_ACTION_BLOCK;

			SOURCE source = FWPS_STREAM_FLAG_RECEIVE;

			if (streamData->flags & FWPS_STREAM_FLAG_SEND)
				source = FWPS_STREAM_FLAG_SEND;

			RmzQueuePacket(inMetaValues->flowHandle, source, streamData);
		}
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
		DbgPrint("Filter added %llu\r\n", filter->filterId);
		break;

	case FWPS_CALLOUT_NOTIFY_DELETE_FILTER:
		DbgPrint("Filter deleted %llu\r\n", filter->filterId);
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

	PFLOW flow = (PFLOW)flowContext;
	RmzRemoveFlow(flow);

	DbgPrint("FlowDeleteFn %llu\r\n", flow->flowId);
}
