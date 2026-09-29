// ============================================================================
// CannonCruise - Realtime Profiler (RealtimeProfiler.cpp)
// Original path: D:\Projects\CannonCruisePC\code\RealtimeProfiler.cpp
// Tersine mühendislik ile Ghidra ve CC.asm analiz edilerek yeniden yazılmıştır.
//
// Binary Fonksiyonları:
//   FUN_0049c000 — CRealtimeProfiler constructor
//   FUN_0049cacf — CRealtimeProfiler::EndSample
// ============================================================================

#include "RealtimeProfiler.h"
#include <cassert>

CRealtimeProfiler::CRealtimeProfiler()
    : m_iStackIndex(0)
{
}

CRealtimeProfiler::~CRealtimeProfiler()
{
    assert(m_iStackIndex == 0 && "m_iStackIndex == 0 Failed");
}

void CRealtimeProfiler::BeginSample(const char* pName)
{
    if (m_iStackIndex < PROFILER_MAX_STACK)
    {
        m_Samples[m_iStackIndex].m_pName = pName;
        m_Samples[m_iStackIndex].m_uStartTime = 0; // clock/rdtsc
        m_iStackIndex++;
    }
}

void CRealtimeProfiler::EndSample()
{
    if (m_iStackIndex > 0)
    {
        m_iStackIndex--;
    }
}

void CRealtimeProfiler::Reset()
{
    m_iStackIndex = 0;
}
