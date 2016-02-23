#include "NetBuffer.h"

void rmzPrintNetBufferList(_In_ PNET_BUFFER_LIST nbl)
{
	PNET_BUFFER_LIST next = nbl;
	int nblNumber = 0;

	while (next != NULL)
	{
		DbgPrint("NET_BUFFER_LIST[%d]\r\n", nblNumber);

		DbgPrint("   Flags: 0x%X\r\n", next->Flags);
		DbgPrint("   NblFlags: 0x%X\r\n", next->NblFlags);
		DbgPrint("   Status: %u\r\n", next->Status);
		DbgPrint("   Clones: %u\r\n", next->ChildRefCount);
		DbgPrint("   Cloned: %s\r\n", next->ParentNetBufferList == NULL ? "false" : "true");

		rmzPrintNetBufferListContext(next->Context);
		rmzPrintNetBuffer(next->FirstNetBuffer);

		next = next->Next;
		nblNumber++;
	}
}

void rmzPrintNetBufferListContext(_In_ PNET_BUFFER_LIST_CONTEXT nblc)
{
	PNET_BUFFER_LIST_CONTEXT next = nblc;
	int nblcNumber = 0;

	while (next != NULL)
	{
		DbgPrint("NET_BUFFER_LIST_CONTEXT[%d]\r\n", nblcNumber);

		DbgPrint("   Size: %u\r\n", next->Size);
		DbgPrint("   Offset: %u\r\n", next->Offset);
		DbgPrint("   ContextData: ");
		for (int i = next->Offset; i < next->Size; ++i)
			DbgPrint("%02X ", next->ContextData[i]);
		DbgPrint("\r\n");
		
		next = next->Next;
		nblcNumber++;
	}
}

void rmzPrintNetBuffer(_In_ PNET_BUFFER nb)
{
	PNET_BUFFER next = nb;
	int nbNumber = 0;

	while (next != NULL)
	{
		DbgPrint("NET_BUFFER[%d]\r\n", nbNumber);

		DbgPrint("   ChecksumBias: %hu\r\n", next->ChecksumBias);
		DbgPrint("   CurrentMdlOffset: %lu\r\n", next->CurrentMdlOffset);
		DbgPrint("   DataLength: %lu\r\n", next->DataLength);
		DbgPrint("   DataOffset: %lu\r\n", next->DataOffset);

		rmzPrintMDL(next->CurrentMdl);

		next = next->Next;
		nbNumber++;
	}
}

void rmzPrintMDL(_In_ PMDL mdl)
{
	PMDL next = mdl;
	int mdlNumber = 0;
	PUINT8 startData = NULL;

	while (next != NULL)
	{
		DbgPrint("MDL[%d]\r\n", mdlNumber);

		DbgPrint("   ByteCount: %lu\r\n", mdl->ByteCount);
		DbgPrint("   ByteOffset: %lu\r\n", mdl->ByteOffset);
		DbgPrint("   MdlFlags: 0x%hd\r\n", mdl->MdlFlags);
		DbgPrint("   Size: 0x%hd\r\n", mdl->Size);
		DbgPrint("   MappedSystemVa: %p\r\n", mdl->MappedSystemVa);
		DbgPrint("   StartVa: %p\r\n", mdl->StartVa);

		startData = (PUINT8)(next->StartVa) + next->ByteOffset;
		DbgPrint("   Data: ");
		for (ULONG i = 0; i < next->ByteCount; ++i)
			DbgPrint("%02X ", startData[i]);
		DbgPrint("\r\n");

		next = next->Next;
		mdlNumber++;
	}
}