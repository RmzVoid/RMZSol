#include <fwptypes.h>
#include <fwpmu.h>
#include <stdio.h>
#include <conio.h>

#include "..\RMZCommon\Common.h"

BOOL CheckError(DWORD status, LPCWSTR message);

int main()
{
	HANDLE engine;
	FWPM_SESSION session = { 0 };
	FWPM_PROVIDER provider = { 0 };
	FWPM_SUBLAYER sublayer = { 0 };
	FWPM_CALLOUT calloutConnect = { 0 };
	FWPM_CALLOUT calloutStream = { 0 };
	FWPM_FILTER filterConnect = { 0 };
	FWPM_FILTER filterStream = { 0 };

	try
	{
		//
		// Create session
		//
		session.displayData.name = L"RMZControl session";
		session.displayData.description = L"RMZControl session description";
		session.flags = FWPM_SESSION_FLAG_DYNAMIC;

		DWORD status = FwpmEngineOpen(NULL, RPC_C_AUTHN_DEFAULT, NULL, &session, &engine);

		if (!CheckError(status, L"FwpmEngineOpen"))
			return status;

		// Add provider
		provider.displayData.name = L"RMZControl provider";
		provider.displayData.description = L"RMZControl provider description";
		provider.providerKey = rmzProviderGuid;

		status = FwpmProviderAdd(engine, &provider, NULL);

		if (!CheckError(status, L"FwpmProviderAdd")) throw status;

		//
		// Add sublayer
		//
		sublayer.displayData.name = L"RMZControl sublayer";
		sublayer.displayData.description = L"RMZControl sublayer description";
		sublayer.subLayerKey = rmzSublayerGuid;
		sublayer.providerKey = (GUID*)&rmzProviderGuid;
		sublayer.weight = 0x01;

		status = FwpmSubLayerAdd(engine, &sublayer, NULL);

		if (!CheckError(status, L"FwpmSublayerAdd")) throw status;

		//
		// Add callouts
		//

		// connect callout
		calloutConnect.displayData.name = L"RMZControl connect callout";
		calloutConnect.displayData.description = L"RMZControl connect callout description";
		calloutConnect.calloutKey = rmzCalloutConnectGuid;
		calloutConnect.providerKey = (GUID*)&rmzProviderGuid;
		calloutConnect.applicableLayer = FWPM_LAYER_ALE_FLOW_ESTABLISHED_V4;

		status = FwpmCalloutAdd(engine, &calloutConnect, NULL, &calloutConnect.calloutId);

		if (!CheckError(status, L"FwpmCalloutAdd(calloutConnect)")) throw status;

		// stream callout
		calloutStream.displayData.name = L"RMZControl stream callout";
		calloutStream.displayData.description = L"RMZControl stream callout description";
		calloutStream.calloutKey = rmzCalloutStreamGuid;
		calloutStream.providerKey = (GUID*)&rmzProviderGuid;
		calloutStream.applicableLayer = FWPM_LAYER_STREAM_V4;

		status = FwpmCalloutAdd(engine, &calloutStream, NULL, &calloutStream.calloutId);

		if (!CheckError(status, L"FwpmCalloutAdd(calloutStream)")) throw status;

		//
		// Add filter
		//

		// connect filter
		FWPM_FILTER_CONDITION conditionConnect[1] = { { 0 } };
		conditionConnect[0].fieldKey = FWPM_CONDITION_IP_REMOTE_PORT;
		conditionConnect[0].matchType = FWP_MATCH_EQUAL;
		conditionConnect[0].conditionValue.type = FWP_UINT16;
		conditionConnect[0].conditionValue.uint16 = 7777;

		filterConnect.displayData.name = L"RMZControl connection filter";
		filterConnect.displayData.description = L"RMZControl connection filter description";
		filterConnect.filterKey = rmzFilterConnectGuid;
		filterConnect.numFilterConditions = 1;
		filterConnect.filterCondition = conditionConnect;
		filterConnect.providerKey = (GUID*)&rmzProviderGuid;
		filterConnect.layerKey = FWPM_LAYER_ALE_FLOW_ESTABLISHED_V4;
		filterConnect.subLayerKey = rmzSublayerGuid;
		filterConnect.weight.type = FWP_EMPTY;
		filterConnect.action.type = FWP_ACTION_CALLOUT_UNKNOWN;
		filterConnect.action.calloutKey = rmzCalloutConnectGuid;
		filterConnect.rawContext = context;

		status = FwpmFilterAdd(engine, &filterConnect, NULL, &filterConnect.filterId);

		if (!CheckError(status, L"FwpmFilterAdd(filterConnect)")) throw status;

		// stream filter
		FWPM_FILTER_CONDITION conditionStream[1] = { { 0 } };
		conditionStream[0].fieldKey = FWPM_CONDITION_IP_REMOTE_PORT;
		conditionStream[0].matchType = FWP_MATCH_EQUAL;
		conditionStream[0].conditionValue.type = FWP_UINT16;
		conditionStream[0].conditionValue.uint16 = 7777;

		filterStream.displayData.name = L"RMZControl stream filter";
		filterStream.displayData.description = L"RMZControl stream filter description";
		filterStream.filterKey = rmzFilterStreamGuid;
		filterStream.numFilterConditions = 1;
		filterStream.filterCondition = conditionStream;
		filterStream.providerKey = (GUID*)&rmzProviderGuid;
		filterStream.layerKey = FWPM_LAYER_STREAM_V4;
		filterStream.subLayerKey = rmzSublayerGuid;
		filterStream.weight.type = FWP_EMPTY;
		filterStream.action.type = FWP_ACTION_CALLOUT_UNKNOWN;
		filterStream.action.calloutKey = rmzCalloutStreamGuid;

		status = FwpmFilterAdd(engine, &filterStream, NULL, &filterConnect.filterId);
		
		if (!CheckError(status, L"FwpmFilterAdd(filterStream)")) throw status;
	}
	catch (DWORD status)
	{
		wprintf(L"Catched exception 0x%X\r\n", status);
	}

	//
	// Pause console
	//
	wprintf(L"Press any key to exit\r\n");
	while (!_kbhit()){}

	FwpmFilterDeleteByKey(engine, &rmzFilterStreamGuid);
	FwpmFilterDeleteByKey(engine, &rmzFilterConnectGuid);
	FwpmCalloutDeleteByKey(engine, &rmzCalloutStreamGuid);
	FwpmCalloutDeleteByKey(engine, &rmzCalloutConnectGuid);
	FwpmSubLayerDeleteByKey(engine, &rmzSublayerGuid);
	FwpmProviderDeleteByKey(engine, &rmzProviderGuid);
	FwpmEngineClose(engine);

	return 0;
}

BOOL CheckError(DWORD status, LPCWSTR message)
{
	if (status != ERROR_SUCCESS)
	{
		wprintf(L"%s failed: 0x%X\r\n", message, status, GetLastError);
		return FALSE;
	}
	else
	{
		wprintf(L"%s success\r\n", message);
		return TRUE;
	}
}