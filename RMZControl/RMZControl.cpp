#include <fwptypes.h>
#include <fwpmu.h>
#include <stdio.h>

#include "..\RMZCommon\Common.h"

BOOL CheckError(DWORD status, LPCWSTR message);

int main()
{
	//
	// Create session
	//
	FWPM_SESSION session = { 0 };
	session.displayData.name = L"RMZControl session";
	session.displayData.description = L"RMZControl session description";
	session.flags = FWPM_SESSION_FLAG_DYNAMIC;

	HANDLE engine;

	DWORD status = FwpmEngineOpen(NULL, RPC_C_AUTHN_DEFAULT, NULL, &session, &engine);

	if (!CheckError(status, L"FwpmEngineOpen"))
		return status;
	
	// Add provider
	FWPM_PROVIDER provider = { 0 };
	provider.displayData.name = L"RMZControl provider";
	provider.displayData.description = L"RMZControl provider description";
	provider.providerKey = rmzProviderGuid;

	status = FwpmProviderAdd(engine, &provider, NULL);

	if (!CheckError(status, L"FwpmProviderAdd")) goto exit;

	//
	// Add sublayer
	//
	FWPM_SUBLAYER sublayer = { 0 };
	sublayer.displayData.name = L"RMZControl sublayer";
	sublayer.displayData.description = L"RMZControl sublayer description";
exit:

	FwpmProviderDeleteByKey(engine, &rmzProviderGuid);
	FwpmEngineClose(engine);

	return 0;
}

BOOL CheckError(DWORD status, LPCWSTR message)
{
	if (status != ERROR_SUCCESS)
	{
		wprintf(L"%s failed: %d", message, status);
		return FALSE;
	}
	else
	{
		wprintf(L"%s success", message);
		return TRUE;
	}
}