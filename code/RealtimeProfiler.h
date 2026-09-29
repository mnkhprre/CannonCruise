#ifndef REALTIMEPROFILER_H
#define REALTIMEPROFILER_H

// ============================================================================
// CannonCruise - Realtime Profiler (RealtimeProfiler.h)
// Original path: D:\Projects\CannonCruisePC\code\RealtimeProfiler.h
// Tersine mühendislik ile Ghidra ve CC.asm analiz edilerek yeniden yazılmıştır.
// ============================================================================

#include "Singleton.h"
#include <rwcore.h>

#define PROFILER_MAX_STACK 32

struct SProfileSample
{
    const char* m_pName;
    RwUInt32 m_uStartTime;
    RwUInt32 m_uTotalTime;
};

class CRealtimeProfiler : public CSingleton<CRealtimeProfiler>
{
public:
    CRealtimeProfiler();
    virtual ~CRealtimeProfiler();

    void BeginSample(const char* pName);
    void EndSample();
    void Reset();

private:
    SProfileSample m_Samples[PROFILER_MAX_STACK];
    RwUInt32 m_iStackIndex;
};

#endif // REALTIMEPROFILER_H
