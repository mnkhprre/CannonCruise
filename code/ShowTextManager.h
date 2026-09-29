#ifndef SHOWTEXTMANAGER_H
#define SHOWTEXTMANAGER_H

// ============================================================================
// CannonCruise - Show Text Manager (ShowTextManager.h)
// Original path: D:\Projects\CannonCruisePC\code\ShowTextManager.h
// Tersine mühendislik ile Ghidra ve CC.asm analiz edilerek yeniden yazılmıştır.
// ============================================================================

#include "Singleton.h"
#include "framework/core/eventhandler/ceventhandler.h"
#include <rwcore.h>
#include <vector>
#include <string>

class CShowTextManager : public CSingleton<CShowTextManager>, public RWS::CEventHandler
{
public:
    CShowTextManager();
    virtual ~CShowTextManager();

    virtual void HandleEvents(const RWS::CMsg& msg);

    void QueueText(const char* pText, RwReal duration);
    void FormatString(const char* pInput, RwUInt16* pOutputBuffer, RwUInt32 maxLen);
    void Update(RwReal deltaTime);

private:
    RwUInt16 m_sTempString[512];
    std::vector<std::string> m_TextQueue;
};

#endif // SHOWTEXTMANAGER_H
