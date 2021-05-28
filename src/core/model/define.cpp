
#include "define.h"

#include <windows.h>

int GetScanValueTypeSize(EScanValueType type)
{
	switch (type)
	{
	case EScanValueType_Byte:   return sizeof(BYTE);
	case EScanValueType_2_Byte: return sizeof(WORD);
	case EScanValueType_4_Byte: return sizeof(DWORD32);
	case EScanValueType_8_Byte: return sizeof(DWORD64);
	case EScanValueType_Float:  return sizeof(FLOAT);
	case EScanValueType_Double: return sizeof(DOUBLE);
	case EScanValueType_String: return sizeof(BYTE);
	case EScanValueType_All:    return sizeof(BYTE);
	}
	return sizeof(BYTE);
}

