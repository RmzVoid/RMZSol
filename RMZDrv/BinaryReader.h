#include "Driver.h"

typedef struct _BINARYREADER
{
	UINT64 currentPosition;
	PUINT8 buffer;
	UINT64 bufferSize;
} BINARYREADER, *PBINARYREADER;

void RmzBrInit(PBINARYREADER br, PVOID buffer, UINT64 size);
UINT64 RmzBrReadUInt64(PBINARYREADER br);
UINT32 RmzBrReadUInt32(PBINARYREADER br);
UINT16 RmzBrReadUInt16(PBINARYREADER br);
UINT8 RmzBrReadUInt8(PBINARYREADER br);
PVOID RmzBrReadBuffer(PBINARYREADER br, UINT64 length, BOOL copy);
