#ifndef RWS_AUDIO_H
#define RWS_AUDIO_H

// ============================================================================
// CannonCruise - RenderWare Studio Framework (Audio.h)
// Original path: D:\Projects\CannonCruisePC\code\framework\toolkits\Audio\Audio.h
// Tersine mühendislik ile Ghidra ve CC.asm analiz edilerek yeniden yazılmıştır.
// ============================================================================

#include <rwcore.h>

namespace RWS
{
    namespace Audio
    {
        // Ses alt sistemi başlatma & kapatma
        RwBool Initialize();
        void Shutdown();

        // Genel ses seviyesi kontrolü
        void SetMasterVolume(RwReal volume);
        RwReal GetMasterVolume();

        // Efekt & Müzik seviyesi
        void SetEffectsVolume(RwReal volume);
        RwReal GetEffectsVolume();
        void SetMusicVolume(RwReal volume);
        RwReal GetMusicVolume();

        // 3D Dinleyici konumu güncelleme
        void SetListenerPosition(const RwV3d* pPos, const RwV3d* pForward, const RwV3d* pUp);
    }
}

#endif // RWS_AUDIO_H
