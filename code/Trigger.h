#pragma once

// ============================================================================
// CannonCruise - Area Trigger (Trigger.h)
// Original path: D:\Projects\CannonCruisePC\code\Trigger.h
// Tersine mühendislik ile Ghidra ve CC.asm analiz edilerek yeniden yazılmıştır.
// Binary ref: FUN_004bf390 - FUN_004bfe50
// ============================================================================

#include <rwcore.h>
#include <rpworld.h>
#include "framework/core/attributehandler/cattributehandler.h"
#include "framework/core/eventhandler/ceventhandler.h"
#include "CollisionTrigger.h"
#include "SpeakManager.h"

namespace RWS
{

/**
 * @class CTrigger
 * @brief Varlıklar belirli bir alana girdiğinde veya çıktığında olay ateşleyen alan tetikleyicisi.
 *
 * CTrigger, dünya koordinatlarında bir konum ve yarıçap tanımlar; fizik sistemi aracılığıyla
 * varlıkların bu alana giriş/çıkışlarını algılar ve önceden tanımlanmış RWS olaylarını
 * ateşler. Ayrıca CSpeakManager entegrasyonu ile konuşma/diyalog tetikleme desteği sunar.
 *
 * **Özellikler:**
 * - Küresel tetikleme alanı (konum + yarıçap)
 * - Giriş (`OnEnter`) ve çıkış (`OnExit`) olayları
 * - Tek seferlik veya tekrarlanan tetikleme modu (`m_bOneShot`)
 * - CSpeakManager aracılığıyla sesli diyalog tetikleme
 *
 * **Binary referansları:**
 * - String tablosu: `0x0061947c`
 * - Ctor: `FUN_004bf390`
 * - HandleEvents: `FUN_004bfe50`
 */
class CTrigger : public CAttributeHandler, public CEventHandler
{
public:
    RWS_MAKENEWCLASS(CTrigger);
    RWS_DECLARE_CLASS(CTrigger);

    /**
     * @brief Yapıcı — attribute paketi ile tetikleyiciyi başlatır.
     * @param packet Konum, yarıçap ve olay ID verilerini içeren paket.
     */
    CTrigger(const CAttributePacket& packet);

    /**
     * @brief Yıkıcı — olay kayıtlarını temizler.
     */
    virtual ~CTrigger();

    /**
     * @brief Konum, yarıçap ve olay ID'lerini attribute paketinden okur.
     * @param packet Güncel ayarları içeren paket.
     */
    virtual void HandleAttributes(const CAttributePacket& packet);

    /**
     * @brief Gelen olayları işler; fizik çarpışma bildirimlerini değerlendirir.
     * @param msg İşlenecek CMsg olayı.
     */
    virtual void HandleEvents(CMsg& msg);

    /**
     * @brief Bir varlık tetikleme alanına girdiğinde çağrılır.
     *
     * `m_eventOnEnter` olayını ateşler ve eğer `m_speakId` geçerliyse
     * CSpeakManager üzerinden konuşma başlatır.
     */
    void OnEnter();

    /**
     * @brief Bir varlık tetikleme alanından çıktığında çağrılır.
     *
     * `m_eventOnExit` olayını ateşler.
     */
    void OnExit();

private:
    CEventId m_eventOnEnter;   ///< Giriş olayı ID'si
    CEventId m_eventOnExit;    ///< Çıkış olayı ID'si
    CEventId m_eventOnStay;    ///< İçeride kalma olayı ID'si (periyodik)

    RwV3d  m_triggerPos;       ///< Tetikleme alanının dünya konumu
    RwReal m_triggerRadius;    ///< Tetikleme alanının yarıçapı (metre)
    RwBool m_bOneShot;         ///< TRUE ise yalnızca bir kez tetiklenir
    RwBool m_bTriggered;       ///< Tek seferlik tetikleme gerçekleşti mi?
    RwBool m_bInsideTrigger;   ///< Şu anda tetikleme alanında varlık var mı?

    RwInt32 m_speakId;         ///< Tetiklendiğinde oynatılacak konuşma ID'si
};

} // namespace RWS
