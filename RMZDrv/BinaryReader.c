#include "BinaryReader.h"

ULONG BRTag = 'gtrB';

void RmzBrInit(PBINARYREADER br, PVOID buffer, UINT64 size)
{
	br->buffer = buffer;
	br->currentPosition = 0;
	br->bufferSize = size;
};

UINT64 RmzBrReadUInt64(PBINARYREADER br)
{
	NT_ASSERT((br->currentPosition + sizeof(UINT64)) <= br->bufferSize);

	UINT64 result = *(PUINT64)(&br->buffer[br->currentPosition]);
	br->currentPosition += sizeof(UINT64);
	return result;
}

UINT32 RmzBrReadUInt32(PBINARYREADER br)
{
	NT_ASSERT((br->currentPosition + sizeof(UINT32)) <= br->bufferSize);

	UINT32 result = *(PUINT32)(&br->buffer[br->currentPosition]);
	br->currentPosition += sizeof(UINT32);
	return result;
}

UINT16 RmzBrReadUInt16(PBINARYREADER br)
{
	NT_ASSERT((br->currentPosition + sizeof(UINT16)) <= br->bufferSize);

	UINT16 result = *(PUINT16)(&br->buffer[br->currentPosition]);
	br->currentPosition += sizeof(UINT16);
	return result;
}

UINT8 RmzBrReadUInt8(PBINARYREADER br)
{
	NT_ASSERT((br->currentPosition + sizeof(UINT8)) <= br->bufferSize);

	UINT8 result = br->buffer[br->currentPosition];
	br->currentPosition += sizeof(UINT8);
	return result;
}

//
// Don't forget to free memory allocated for buffer
PVOID RmzBrReadBuffer(PBINARYREADER br, UINT64 length, BOOL copy)
{
	NT_ASSERT((br->currentPosition + length) <= br->bufferSize);

	PVOID buffer = NULL;
	//
	// copy buffer content to new location

	if (copy)
	{
		buffer = ExAllocatePoolWithTag(NonPagedPool, length, BRTag);
		if (buffer)
		{
			RtlCopyMemory(buffer, &br->buffer[br->currentPosition], length);

		}
	}
	else
		buffer = &br->buffer[br->currentPosition];

	br->currentPosition += length;

	return buffer;
}