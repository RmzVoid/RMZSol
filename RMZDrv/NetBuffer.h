#pragma once

#include <stdio.h>

#include "driver.h"

void rmzPrintNetBufferList(_In_ PNET_BUFFER_LIST nbl);
void rmzPrintNetBufferListContext(_In_ PNET_BUFFER_LIST_CONTEXT nblc);
void rmzPrintNetBuffer(_In_ PNET_BUFFER nb);
void rmzPrintMDL(_In_ PMDL mdl);
