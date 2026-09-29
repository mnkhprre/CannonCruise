#ifndef PERFORMANCEVISUALIZER_H
#define PERFORMANCEVISUALIZER_H

// ============================================================================
// CannonCruise - Performance Visualizer (PerformanceVisualizer.h)
// Original path: D:\Projects\CannonCruisePC\code\PerformanceVisualizer.h
// Tersine mühendislik ile Ghidra ve CC.asm analiz edilerek yeniden yazılmıştır.
// ============================================================================

#include "framework/core/eventhandler/ceventhandler.h"
#include <rwcore.h>

class CPerformanceVisualizer : public RWS::CEventHandler
{
public:
    CPerformanceVisualizer();
    virtual ~CPerformanceVisualizer();

    virtual void HandleEvents(const RWS::CMsg& msg);

    void Render();
    void SetEnabled(RwBool bEnable) { m_bEnabled = bEnable; }

private:
    RwBool m_bEnabled;
    void* m_pCharset;
};

#endif // PERFORMANCEVISUALIZER_H
