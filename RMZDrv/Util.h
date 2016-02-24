#pragma once

#include <stdio.h>

#include "driver.h"

LPCSTR rmzFwpValueToStr(_In_ FWP_VALUE* value, _In_ LPSTR string, _In_ DWORD lenght);
void rmzPrintIpAddr(UINT32 address);