#ifndef AUDIOMANAGER_H
#define AUDIOMANAGER_H

// ============================================================================
// CannonCruise - Audio Manager (AudioManager.h)
// Original path: D:\Projects\CannonCruisePC\code\AudioManager.h
// Tersine mühendislik ile Ghidra ve CC.asm analiz edilerek yeniden yazılmıştır.
// ============================================================================

#include "Singleton.h"
#include "framework/core/eventhandler/ceventhandler.h"
#include <rwcore.h>
#include <string>
#include <map>

class CAudioManager : public CSingleton<CAudioManager>, public RWS::CEventHandler
{
public:
    CAudioManager();
    virtual ~CAudioManager();

    virtual void HandleEvents(const RWS::CMsg& msg);

    // Ses Çalma & Durdurma
    void PlaySound(const char* pSoundName, const RwV3d* pPos = 0);
    void PlayStream(const char* pStreamName, RwBool bLoop = FALSE);
    void StopAllVoices();

    // Ses Grubu Seviyeleri (VolumeGroup1, 2, 3)
    void SetVolumeGroup(RwUInt32 groupIndex, RwReal volume);
    RwReal GetVolumeGroup(RwUInt32 groupIndex) const;

    // 3D Dinleyici Konumu
    void UpdateListenerPosition(const RwMatrix* pMatrix);

private:
    RwReal m_afVolumeGroups[3];
    RwMatrix m_ListenerMatrix;
    std::string m_SoundPath;
};

#endif // AUDIOMANAGER_H
