#ifndef SPEAKMANAGER_H
#define SPEAKMANAGER_H

// ============================================================================
// CannonCruise - Speech Manager (SpeakManager.h)
// Original path: D:\Projects\CannonCruisePC\code\SpeakManager.h
// Tersine mühendislik ile Ghidra ve CC.asm analiz edilerek yeniden yazılmıştır.
// ============================================================================

#include "Singleton.h"
#include "framework/core/eventhandler/ceventhandler.h"
#include "SpeakStreamDefs.h"
#include <rwcore.h>
#include <string>

class CSpeakManager : public CSingleton<CSpeakManager>, public RWS::CEventHandler
{
public:
    CSpeakManager();
    virtual ~CSpeakManager();

    virtual void HandleEvents(const RWS::CMsg& msg);

    RwBool IsSpeakValid(RwInt32 iCategory, RwInt32 iIndex) const;
    void PlaySpeak(RwInt32 iCategory, RwInt32 iIndex, const RwV3d* pPos = 0);
    void StopSpeak();

private:
    std::string m_SpeaksPath;
    RwInt32 m_iCurrentCategory;
    RwInt32 m_iCurrentIndex;
};

#endif // SPEAKMANAGER_H
