#ifndef RWS_CEVENTHANDLER_H
#define RWS_CEVENTHANDLER_H

// ============================================================================
// CannonCruise - RenderWare Studio Framework (ceventhandler.h)
// Original path: D:\Projects\CannonCruisePC\code\framework\core\eventhandler\ceventhandler.h
// Tersine mühendislik ile Ghidra ve CC.asm analiz edilerek yeniden yazılmıştır.
// Binary ref: FUN_00401000 - FUN_004013e0 (RWS::CEventHandler)
// ============================================================================

#include <rwcore.h>
#include <vector>
#include <string>

/**
 * @namespace RWS
 * @brief RenderWare Studio framework'ünün tüm çekirdek sınıflarını barındıran ad alanı.
 *
 * CannonCruise'un tüm oyun nesneleri bu ad alanındaki `CEventHandler` ve
 * `CAttributeHandler` gibi temel sınıflardan türer. Olay tabanlı mesajlaşma
 * (`SendMsg` / `HandleEvents`) oyun mantığının ana iletişim mekanizmasıdır.
 */
namespace RWS
{
    /// Kayıtlı olay bağlantıları için varsayılan blok boyutu
    #define CEVENT_HANDLER_REGISTERED_MSG_DEFAULT_BLOCK_SIZE 128
    /// Kayıtlı mesajlar için varsayılan blok boyutu
    #define CREGISTERED_MSGS_DEFAULT_BLOCK_SIZE 128

    /**
     * @typedef CEventId
     * @brief Olay kimliği türü — 32-bit işaretsiz tam sayı.
     *
     * `RegisterMsg()` tarafından döndürülen benzersiz olay kimliği.
     * Sabit sistem olayları (iMsgRunningTick vb.) doğrudan tanımlanır;
     * oyun özgü olaylar çalışma zamanında `RegisterMsg()` ile alınır.
     */
    typedef RwUInt32 CEventId;

    /// Oyun döngüsünün her karesinde yayımlanan tick olayı
    const CEventId iMsgRunningTick    = 0x0001;
    /// Havok fizik adımı için yayımlanan güncelleme olayı
    const CEventId iMsgPhysicsUpdate  = 0x0002;

    /**
     * @struct CMsg
     * @brief Olay tabanlı mesajlaşma sisteminde iletilen mesaj yapısı.
     *
     * `SendMsg()` tarafından tüm kayıtlı `CEventHandler` örneklerine iletilir.
     * `m_pData` / `pData` alanı, gönderilen olaya özel veriye işaret eden
     * genel (void*) bir göstericidir; alıcı uygun türe dönüştürür.
     *
     * **Kullanım örneği:**
     * ```cpp
     * CMsg msg(iMsgRunningTick, nullptr, 0);
     * CEventHandler::SendMsg(msg);
     * ```
     */
    struct CMsg
    {
        union
        {
            CEventId m_Id;  ///< Olay kimliği (C++ stili erişim)
            CEventId Id;    ///< Olay kimliği (eski stil erişim — geriye uyumluluk)
        };
        union
        {
            const void* m_pData; ///< Olaya özgü veri işaretçisi (C++ stili)
            const void* pData;   ///< Olaya özgü veri işaretçisi (eski stil)
        };
        RwUInt32 m_Priority; ///< Mesaj önceliği (yüksek öncelik önce işlenir)

        /**
         * @brief CMsg yapıcısı.
         * @param id       Olay kimliği.
         * @param pData    Olaya özgü veri (nullptr olabilir).
         * @param priority Mesaj önceliği (varsayılan: 0).
         */
        CMsg(CEventId id = 0, const void* pData = 0, RwUInt32 priority = 0)
            : m_Id(id), m_pData(pData), m_Priority(priority)
        {
        }
    };

    /**
     * @class CEventHandler
     * @brief RenderWare Studio olay sistemi için temel sınıf.
     *
     * CannonCruise'daki tüm oyun davranış sınıfları (CSpeakManager, CAudioManager,
     * CScene, vb.) doğrudan veya dolaylı olarak `CEventHandler`'dan türer.
     * Bu sınıf, olaylara abone olmayı (`LinkMsg`), abonelikten çıkmayı (`UnLinkMsg`)
     * ve olayları işlemeyi (`HandleEvents`) sağlar.
     *
     * **Yaşam döngüsü:**
     * 1. Yapıcı çağrılır.
     * 2. `LinkMsg()` ile ilgilenilen olaylara abone olunur.
     * 3. `HandleEvents()` override edilerek gelen olaylar işlenir.
     * 4. Yıkıcı çağrıldığında `UnLinkAllProcess()` otomatik tetiklenir.
     *
     * **Binary referansları:**
     * - Dtor: `FUN_00401000`
     * - HandleEvents: `FUN_00401060` (sanal, varsayılan boş)
     * - LinkMsg: `FUN_004010c0`
     * - UnLinkMsg: `FUN_00401180`
     * - SendMsg: `FUN_004012a0` (statik)
     * - RegisterMsg: `FUN_00401300` (statik)
     */
    class CEventHandler
    {
    public:
        /**
         * @brief Varsayılan yapıcı.
         */
        CEventHandler();

        /**
         * @brief Öncelik parametreli yapıcı (geriye uyumluluk).
         * @param priority Başlangıç önceliği (kullanılmayabilir).
         */
        CEventHandler(RwUInt32);

        /**
         * @brief Yıkıcı — `UnLinkAllProcess()` çağrılarak tüm abonelikler iptal edilir.
         */
        virtual ~CEventHandler();

        /**
         * @brief Gelen olayı işleyen sanal fonksiyon.
         * @param msg İşlenecek mesaj.
         *
         * Türetilmiş sınıflar bu fonksiyonu override ederek kendi olay mantığını uygular.
         * Bilinmeyen olaylar için temel sınıf uygulaması hiçbir şey yapmaz.
         */
        virtual void HandleEvents(const CMsg& msg);

        /**
         * @brief Değiştirilebilir mesaj referansı alan HandleEvents aşırı yüklemesi.
         * @param msg İşlenecek mesaj.
         */
        virtual void HandleEvents(CMsg& msg) { HandleEvents((const CMsg&)msg); }

        /**
         * @brief Belirtilen olaya abone olur.
         * @param eventId  Abone olunacak olay kimliği.
         * @param pFormat  Verinin formatını tanımlayan isteğe bağlı format dizesi.
         * @param priority Olay önceliği (yüksek değer = önce işlenir).
         */
        void LinkMsg(CEventId eventId, const char* pFormat = 0, RwUInt32 priority = 0);

        /**
         * @brief Belirtilen olaydan aboneliği kaldırır.
         * @param eventId Aboneliği kaldırılacak olay kimliği.
         */
        void UnLinkMsg(CEventId eventId);

        /**
         * @brief Tüm olay aboneliklerini iptal eder.
         *
         * Yıkıcı tarafından otomatik çağrılır; ayrıca elle de çağrılabilir.
         */
        void UnLinkAllProcess();

        /**
         * @brief `LinkMsg()` için uyumluluk sarmalayıcısı.
         * @param eventId Abone olunacak olay kimliği.
         */
        void RegisterForMessage(CEventId eventId) { LinkMsg(eventId); }

        /**
         * @brief `UnLinkMsg()` için uyumluluk sarmalayıcısı.
         * @param eventId Aboneliği kaldırılacak olay kimliği.
         */
        void UnregisterForMessage(CEventId eventId) { UnLinkMsg(eventId); }

        /**
         * @brief Bir mesajı tüm abone `CEventHandler` örneklerine iletir.
         * @param msg Gönderilecek mesaj.
         *
         * Mesaj, `m_Priority` değerine göre sıralanmış abonelere iletilir.
         */
        static void SendMsg(const CMsg& msg);

        /**
         * @brief Yeni bir olay adı kaydeder ve benzersiz kimlik döndürür.
         * @param pEventName Kaydedilecek olay adı (örn. "iMsgHit").
         * @param pFormat    İsteğe bağlı veri format dizesi.
         * @return CEventId — Yeni olayın benzersiz kimliği.
         */
        static CEventId RegisterMsg(const char* pEventName, const char* pFormat = 0);

        /**
         * @brief Daha önce kaydedilmiş bir olayı sistemden kaldırır.
         * @param eventId Kaldırılacak olay kimliği.
         *
         * Binary referansı: `loc_40CC60` — kayıtlı olay kaydını ve abonelik
         * bağlantılarını siler (RenderPipelineShutdown bunu çağırır).
         */
        static void UnRegisterMsg(CEventId eventId);

        /**
         * @brief Olay sistemini başlatır (uygulama başlangıcında bir kez çağrılır).
         */
        static void Initialize();

        /**
         * @brief Olay sistemini kapatır ve tüm kayıtlı olayları temizler.
         */
        static void Shutdown();

    protected:
        /**
         * @struct EventLink
         * @brief Bu nesnenin abone olduğu bir olayı tanımlayan iç yapı.
         */
        struct EventLink
        {
            CEventId   m_EventId;  ///< Abone olunan olay kimliği
            RwUInt32   m_Priority; ///< İşlem önceliği
            std::string m_Format;  ///< Veri format dizesi
        };

        std::vector<EventLink> m_LinkedEvents; ///< Bu nesnenin tüm olay abonelikleri
    };
}

#endif // RWS_CEVENTHANDLER_H
