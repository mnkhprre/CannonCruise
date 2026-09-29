// ============================================================================
// CannonCruise - Profiler Behavior (ProfilerBehaviour.cpp)
// Original path: D:\Projects\CannonCruisePC\code\ProfilerBehaviour.cpp
// Tersine mühendislik ile Ghidra ve CC.asm analiz edilerek yeniden yazılmıştır.
//
// Binary Fonksiyonları:
//   FUN_00499a90 — CProfilerBehaviour constructor & registration
//   FUN_0049a2e0 — CProfilerBehaviour::EnableItem
//   FUN_0049a5e0 — CProfilerBehaviour::DisableItem
// ============================================================================

#include "ProfilerBehaviour.h"
#include "framework/core/attributehandler/cclassfactory.h"
#include <cassert>

#define MAX_PROFILER_ITEMS 32

RWS_REGISTER_CLASS(CProfilerBehaviour, []() -> void* { return new CProfilerBehaviour(); }, sizeof(CProfilerBehaviour));

CProfilerBehaviour::CProfilerBehaviour()
    : m_uProfileFlags(0)
{
}

CProfilerBehaviour::~CProfilerBehaviour()
{
}

void CProfilerBehaviour::HandleEvents(const RWS::CMsg& msg)
{
}

void CProfilerBehaviour::EnableItem(RwUInt32 index)
{
    assert(index < MAX_PROFILER_ITEMS && "Too high index when accessing EnableItem");
    m_uProfileFlags |= (1 << index);
}

void CProfilerBehaviour::DisableItem(RwUInt32 index)
{
    assert(index < MAX_PROFILER_ITEMS && "Too high index when accessing DisableItem");
    m_uProfileFlags &= ~(1 << index);
}
