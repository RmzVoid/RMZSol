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
	FWPM_CALLOUT callout = { 0 };
	FWPM_FILTER filter = { 0 };

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
		// Add callout
		//
		callout.displayData.name = L"RMZControl callout";
		callout.displayData.description = L"RMZControl callout description";
		callout.calloutKey = rmzCalloutGuid;
		callout.providerKey = (GUID*)&rmzProviderGuid;
		callout.applicableLayer = FWPM_LAYER_ALE_AUTH_CONNECT_V4;

		status = FwpmCalloutAdd(engine, &callout, NULL, &callout.calloutId);

		if (!CheckError(status, L"FwpmCalloutAdd")) throw status;

		//
		// Add filter
		//
		const int numConditions = 2;
		FWPM_FILTER_CONDITION condition[numConditions] = { { 0 }, { 0 } };
		condition[0].fieldKey = FWPM_CONDITION_IP_PROTOCOL;
		condition[0].matchType = FWP_MATCH_EQUAL;
		condition[0].conditionValue.type = FWP_UINT8;
		condition[0].conditionValue.uint8 = IPPROTO_TCP;

		condition[1].fieldKey = FWPM_CONDITION_IP_REMOTE_PORT;
		condition[1].matchType = FWP_MATCH_EQUAL;
		condition[1].conditionValue.type = FWP_UINT16;
		condition[1].conditionValue.uint16 = 7777;

		filter.displayData.name = L"RMZControl filter";
		filter.displayData.description = L"RMZControl filter description";
		filter.filterKey = rmzFilterGuid;
		filter.numFilterConditions = numConditions;
		filter.filterCondition = condition;
		filter.providerKey = (GUID*)&rmzProviderGuid;
		filter.layerKey = FWPM_LAYER_ALE_AUTH_CONNECT_V4;
		filter.subLayerKey = rmzSublayerGuid;
		filter.weight.type = FWP_EMPTY;
		filter.action.type = FWP_ACTION_CALLOUT_UNKNOWN;
		filter.action.calloutKey = rmzCalloutGuid;

		status = FwpmFilterAdd(engine, &filter, NULL, &filter.filterId);

		if (!CheckError(status, L"FwpmFilterAdd")) throw status;

		//
		// Pause console
		//
		wprintf(L"Press any key to exit");
		while (!_kbhit()){}
	}
	catch (DWORD status)
	{
		wprintf(L"Catched exception 0x%X\r\n", status);
	}

	FwpmFilterDeleteByKey(engine, &rmzFilterGuid);
	FwpmCalloutDeleteByKey(engine, &rmzCalloutGuid);
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