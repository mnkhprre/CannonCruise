#pragma once

// ============================================================================
// CannonCruise - Event Proxy (eventproxy.h)
// Original path: D:\Projects\CannonCruisePC\code\eventproxy.h
// Tersine mühendislik ile Ghidra ve CC.asm analiz edilerek yeniden yazılmıştır.
// Binary ref: FUN_00473480 - FUN_00473600
// ============================================================================

#include <rwcore.h>
#include <rpworld.h>
#include "framework/core/attributehandler/cattributehandler.h"
#include "framework/core/eventhandler/ceventhandler.h"

namespace RWS
{

/**
 * @class CEventProxy
 * @brief Varlıklar arasında adlandırılmış olay ID'leri kullanarak olay yönlendirici.
 *
 * CEventProxy, bir giriş olayını alır ve bunu çıkış olayına dönüştürerek yeniden ateşler.
 * Bu mekanizma, seviye tasarımcılarının birbirini doğrudan bilmeyen varlıkları
 * RenderWare Studio editöründen bağlamasına olanak tanır. Ayrıca Fernando karakterine
 * özgü IDUNA01/02/03 diyalog olaylarını da işler.
 *
 * **Özellikler:**
 * - Giriş → Çıkış olay yönlendirme
 * - IDUNA01/02/03 — Fernando NPC diyalog tetikleyicileri
 * - `m_bEnabled` ile çalışma zamanında devre dışı bırakma
 *
 * **Binary referansları:**
 * - String tablosu: `0x00616e38` (IDUNA01/02/03 olay adları)
 * - Ctor: `FUN_00473480`
 * - HandleEvents: `FUN_00473580`
 */
class CEventProxy : public CAttributeHandler, public CEventHandler
{
public:
    RWS_MAKENEWCLASS(CEventProxy);
    RWS_DECLARE_CLASS(CEventProxy);

    /**
     * @brief Yapıcı — olay proxy'sini attribute paketi ile başlatır.
     * @param packet Giriş/çıkış olay adlarını içeren attribute paketi.
     */
    CEventProxy(const CAttributePacket& packet);

    /**
     * @brief Yıkıcı — olay kayıtlarını iptal eder.
     */
    virtual ~CEventProxy();

    /**
     * @brief Giriş/çıkış olay ID'lerini ve etkinleştirme durumunu günceller.
     * @param packet Güncel olay ID ayarlarını içeren paket.
     */
    virtual void HandleAttributes(const CAttributePacket& packet);

    /**
     * @brief Giriş olayını alır ve çıkış olayını ateşler.
     * @param msg İşlenecek giriş CMsg olayı.
     *
     * IDUNA01/02/03 mesajları özel Fernando diyalog mantığıyla işlenir.
     */
    virtual void HandleEvents(CMsg& msg);

private:
    CEventId m_eventInput;      ///< Dinlenecek giriş olayı ID'si
    CEventId m_eventOutput;     ///< Yönlendirilecek çıkış olayı ID'si
    CEventId m_eventIduna01;    ///< Fernando diyalog olayı "IDUNA01"
    CEventId m_eventIduna02;    ///< Fernando diyalog olayı "IDUNA02"
    CEventId m_eventIduna03;    ///< Fernando diyalog olayı "IDUNA03"
    RwBool m_bEnabled;          ///< FALSE ise olaylar yönlendirilmez
};

} // namespace RWS
