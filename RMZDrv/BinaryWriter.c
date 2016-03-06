#include "BinaryWriter.h"

void RmzBwInit(PBINARYWRITER bw, PVOID buffer, UINT32 size)
{
	bw->buffer = buffer;
	bw->currentPosition = 0;
	bw->bufferSize = size;
};

void RmzBwWriteUInt64(PBINARYWRITER bw, UINT64 val)
{
	NT_ASSERT((bw->currentPosition + sizeof(UINT64)) <= bw->bufferSize);

	RtlCopyMemory(&bw->buffer[bw->currentPosition], &val, sizeof(UINT64));
	bw->currentPosition += sizeof(UINT64);
}

void RmzBwWriteUInt32(PBINARYWRITER bw, UINT32 val)
{
	NT_ASSERT((bw->currentPosition + sizeof(UINT32)) <= bw->bufferSize);

	RtlCopyMemory(&bw->buffer[bw->currentPosition], &val, sizeof(UINT32));
	bw->currentPosition += sizeof(UINT32);
}

void RmzBwWriteUInt16(PBINARYWRITER bw, UINT16 val)
{
	NT_ASSERT((bw->currentPosition + sizeof(UINT16)) <= bw->bufferSize);

	RtlCopyMemory(&bw->buffer[bw->currentPosition], &val, sizeof(UINT16));
	bw->currentPosition += sizeof(UINT16);
}

void RmzBwWriteUInt8(PBINARYWRITER bw, UINT8 val)
{
	NT_ASSERT((bw->currentPosition + sizeof(UINT8)) <= bw->bufferSize);

	RtlCopyMemory(&bw->buffer[bw->currentPosition], &val, sizeof(UINT8));
	bw->currentPosition += sizeof(UINT8);
}

void RmzBwWriteBuffer(PBINARYWRITER bw, UINT32 length, PVOID source)
{
	NT_ASSERT((bw->currentPosition + length) <= bw->bufferSize);

	RtlCopyMemory(&bw->buffer[bw->currentPosition], source, length);

	bw->currentPosition += length;
}