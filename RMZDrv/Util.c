#include "util.h"

LPCSTR fwpValueToStr(_In_ FWP_VALUE* value, _In_ LPSTR string, _In_ DWORD lenght)
{
	switch (value->type)
	{
	case FWP_EMPTY: 
		strcpy_s(string, lenght, "(empty)");
		break;

	case FWP_UINT8:
		sprintf_s(string, lenght, "%u", value->uint8);
		break;

	case FWP_UINT16:
		sprintf_s(string, lenght, "%hu", value->uint16);
		break;

	case FWP_UINT32:
		sprintf_s(string, lenght, "%u", value->uint32);
		break;

	case FWP_UINT64:
		sprintf_s(string, lenght, "%llu", value->uint64[0]);
		break;

	case FWP_INT8:
		sprintf_s(string, lenght, "%d", value->int8);
		break;

	case FWP_INT16:
		sprintf_s(string, lenght, "%hd", value->int16);
		break;

	case FWP_INT32:
		sprintf_s(string, lenght, "%d", value->int32);
		break;

	case FWP_INT64:
		sprintf_s(string, lenght, "%lld", value->int64[0]);
		break;

	case FWP_FLOAT:
		sprintf_s(string, lenght, "(float)" /*, value->float32 */);
		break;

	case FWP_DOUBLE:
		sprintf_s(string, lenght, "(double)" /*, &value->double64*/);
		break;

	case FWP_BYTE_ARRAY16_TYPE:
	{
		PUINT8 array = value->byteArray16->byteArray16;
		sprintf_s(string, lenght, "%02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X",
			array[0x0], array[0x1], array[0x2], array[0x3],
			array[0x4], array[0x5], array[0x6], array[0x7],
			array[0x8], array[0x9], array[0xA], array[0xB],
			array[0xC], array[0xD], array[0xE], array[0xF]
			);
		break;
	}

	case FWP_BYTE_BLOB_TYPE:
		sprintf_s(string, lenght, "UINT8[%u]", value->byteBlob->size);
		break;

	case FWP_SID:
		sprintf_s(string, lenght, "(sid)");
		break;

	case FWP_SECURITY_DESCRIPTOR_TYPE:
		sprintf_s(string, lenght, "(sd)");
		break;

	case FWP_TOKEN_INFORMATION_TYPE:
		sprintf_s(string, lenght, "(tokeninformation)");
		break;

	case FWP_TOKEN_ACCESS_INFORMATION_TYPE:
		sprintf_s(string, lenght, "(tokenaccessinformation)");
		break;

	case FWP_UNICODE_STRING_TYPE:
		sprintf_s(string, lenght, "%S", value->unicodeString);
		break;

	case FWP_BYTE_ARRAY6_TYPE:
	{
		PUINT8 array = value->byteArray6->byteArray6;
		sprintf_s(string, lenght, "%02X %02X %02X %02X %02X %02X",
			array[0x0], array[0x1], array[0x2], array[0x3], array[0x4], array[0x5]);
		break;
	}

	case FWP_SINGLE_DATA_TYPE_MAX:
		sprintf_s(string, lenght, "(singlemax)");
		break;

	case FWP_V4_ADDR_MASK:
		sprintf_s(string, lenght, "(v4 ip/mask)");
		break;

	case FWP_V6_ADDR_MASK:
		sprintf_s(string, lenght, "(v6 ip/mask)");
		break;

	case FWP_RANGE_TYPE:
		sprintf_s(string, lenght, "(range)");
		break;

	case FWP_DATA_TYPE_MAX:
		sprintf_s(string, lenght, "(datamax)");
		break;

	default:
		sprintf_s(string, lenght, "(unk type %u)", value->type);
	}

	return string;
}