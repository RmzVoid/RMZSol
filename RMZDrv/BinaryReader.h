#include "Driver.h"

typedef struct _BINARYREADER
{
	UINT32 currentPosition;
	PUINT8 buffer;
	UINT32 bufferSize;
} BINARYREADER, *PBINARYREADER;

void RmzBrInit(PBINARYREADER br, PVOID buffer, UINT32 size);
UINT64 RmzBrReadUInt64(PBINARYREADER br);
UINT32 RmzBrReadUInt32(PBINARYREADER br);
UINT16 RmzBrReadUInt16(PBINARYREADER br);
UINT8 RmzBrReadUInt8(PBINARYREADER br);
PVOID RmzBrReadBuffer(PBINARYREADER br, UINT32 length, BOOL copy);
