#include <stdafx.h>
#include "GfxState.h"

uint32_t CHleGfxState::SegmentedToPhysical(uint32_t segaddr)
{
	uint32_t segment = (segaddr >> 24) & 0x0F;
	uint32_t offset = segaddr & 0x00FFFFFF;
	return m_Segments[segment] + offset;
}

uint32_t CHleGfxState::SegmentedToVirtual(uint32_t segaddr)
{
	return SegmentedToPhysical(segaddr) | 0x80000000;
}
