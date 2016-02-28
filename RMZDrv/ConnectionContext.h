#pragma once

#define NDIS_SUPPORT_NDIS6 1

#include <basetsd.h>

#pragma warning(push)
#pragma warning(disable:4201)       // unnamed struct/union
#include <fwpsk.h>
#pragma warning(pop)

typedef struct _PACKET
{
	LIST_ENTRY list;
	UINT64 flowId;
	UINT64 serial;
	FWPS_STREAM_DATA* stream;
} PACKET, *PPACKET;

typedef struct _PACKET_QUEUE
{
	LIST_ENTRY packets;
	KSPIN_LOCK lock;
	KEVENT event;
} PACKET_QUEUE, *PPACKET_QUEUE;

void RmzInitQueue();
void RmzQueuePacket(UINT64 flowId, FWPS_STREAM_DATA* stream);
void RmzFreePacket(PPACKET packet);
PPACKET RmzPopPacket();

BOOL RmzWaitOnQueue();
void RmzNotifyQueueNotEmpty();