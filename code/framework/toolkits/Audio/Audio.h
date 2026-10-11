#ifndef RWS_AUDIO_H
#define RWS_AUDIO_H

// ============================================================================
// CannonCruise - RenderWare Studio Framework (Audio.h)
// Original path: D:\Projects\CannonCruisePC\code\framework\toolkits\Audio\Audio.h
// Tersine mühendislik ile Ghidra ve CC.asm analiz edilerek yeniden yazılmıştır.
// Binary ref: FUN_00407000 - FUN_004072a0 (RWS::Audio)
// ============================================================================

#include <rwcore.h>

namespace RWS
{
    /**
     * @namespace RWS::Audio
     * @brief RenderWare Studio ses altyapısı yardımcı fonksiyonları.
     *
     * CannonCruise'un ses sistemi `CAudioManager` aracılığıyla çalışır;
     * bu namespace ise alttaki ses motorunun (DirectSound / Miles Sound System)
     * başlatılması ve seviye kontrolü için düz fonksiyonlar sunar.
     *
     * **Ses hiyerarşisi:**
     * - Master Volume → genel çıkış seviyesi
     * - Effects Volume → SFX kanalları (top sesi, dalga vb.)
     * - Music Volume → arka plan müziği
     *
     * **3D ses:** `SetListenerPosition()` her karede kameranın konumuyla çağrılmalıdır.
     */
    namespace Audio
    {
        /**
         * @brief Ses alt sistemini başlatır.
         * @return TRUE ise başlatma başarılı.
         *
         * `CAudioManager` oluşturulmadan önce çağrılmalıdır.
         */
        RwBool Initialize();

        /**
         * @brief Ses alt sistemini kapatır, tüm kaynakları serbest bırakır.
         */
        void Shutdown();

        /**
         * @brief Genel ses seviyesini ayarlar.
         * @param volume [0.0, 1.0] aralığında ses seviyesi.
         */
        void SetMasterVolume(RwReal volume);

        /**
         * @brief Genel ses seviyesini döndürür.
         * @return RwReal — Geçerli master volume değeri.
         */
        RwReal GetMasterVolume();

        /**
         * @brief Efekt ses seviyesini ayarlar.
         * @param volume [0.0, 1.0] aralığında ses efekti seviyesi.
         */
        void SetEffectsVolume(RwReal volume);

        /**
         * @brief Efekt ses seviyesini döndürür.
         * @return RwReal — Geçerli effects volume değeri.
         */
        RwReal GetEffectsVolume();

        /**
         * @brief Müzik ses seviyesini ayarlar.
         * @param volume [0.0, 1.0] aralığında müzik seviyesi.
         */
        void SetMusicVolume(RwReal volume);

        /**
         * @brief Müzik ses seviyesini döndürür.
         * @return RwReal — Geçerli music volume değeri.
         */
        RwReal GetMusicVolume();

        /**
         * @brief 3D ses dinleyicisinin konumunu günceller.
         * @param pPos     Dinleyici (kamera) dünya konumu.
         * @param pForward Dinleyicinin baktığı yön (normalize edilmiş).
         * @param pUp      Dinleyicinin yukarı yönü (normalize edilmiş).
         *
         * Her karede kamera güncellemesinin hemen ardından çağrılmalıdır.
         */
        void SetListenerPosition(const RwV3d* pPos, const RwV3d* pForward, const RwV3d* pUp);
    }
}

#endif // RWS_AUDIO_H
