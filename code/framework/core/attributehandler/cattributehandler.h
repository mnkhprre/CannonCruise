#ifndef RWS_CATTRIBUTEHANDLER_H
#define RWS_CATTRIBUTEHANDLER_H

// ============================================================================
// CannonCruise - RenderWare Studio Framework (cattributehandler.h)
// Original path: D:\Projects\CannonCruisePC\code\framework\core\attributehandler\cattributehandler.h
// Tersine mühendislik ile Ghidra ve CC.asm analiz edilerek yeniden yazılmıştır.
// Binary ref: FUN_00402000 - FUN_00402400 (RWS::CAttributeHandler)
// ============================================================================

#include <rwcore.h>
#include <map>
#include <string>
#include <cassert>

#ifndef RWS_ASSERT
/// @brief RWS framework için assert makrosu
#define RWS_ASSERT(cond, msg) assert((cond) && (msg))
#endif

namespace RWS
{
    /**
     * @class CAttributePacket
     * @brief Bir nesnenin yapılandırma verilerini taşıyan ham bayt paketi.
     *
     * RenderWare Studio'nun binary varlık sisteminde nesnelerin başlatılması
     * bu paket yapısı aracılığıyla gerçekleşir. `CAttributeHandler::HandleAttributes()`
     * tarafından alınır ve `CAttributeCommandIterator` yardımıyla ayrıştırılır.
     */
    class CAttributePacket
    {
    public:
        /// @brief Boş paket yapıcısı
        CAttributePacket() : m_pData(0), m_DataSize(0) {}

        /**
         * @brief Ham veri ile paket oluşturur.
         * @param pData Ham paket verisi.
         * @param size  Veri boyutu (bayt).
         */
        CAttributePacket(const void* pData, RwUInt32 size) : m_pData(pData), m_DataSize(size) {}

        /// @return Ham paket verisine işaretçi
        const void* GetData() const { return m_pData; }
        /// @return Paket boyutu (bayt)
        RwUInt32    GetSize() const { return m_DataSize; }

    private:
        const void* m_pData;    ///< Ham veri işaretçisi
        RwUInt32    m_DataSize; ///< Veri boyutu
    };

    /**
     * @struct CAttributeCommandEntry
     * @brief Bir özellik komutunu ve verilerini tutan yapı.
     *
     * `CAttributeCommandIterator` tarafından üretilir; her komut
     * bir kimlik (`m_CommandId`) ve o komuta ait veri işaretçisi içerir.
     */
    struct CAttributeCommandEntry
    {
        RwUInt32    m_CommandId;     ///< Komut kimliği
        const void* m_pCommandData;  ///< Komut verisi işaretçisi

        /// @return Komut kimliği
        RwUInt32    GetCommandId()   const { return m_CommandId; }
        /// @return Komut verisi işaretçisi
        const void* GetCommandData() const { return m_pCommandData; }
    };

    /**
     * @class CAttributeCommandIterator
     * @brief `CAttributePacket` içindeki komutlar üzerinde yineleyici.
     *
     * `CAttributeHandler::HandleAttributes()` içinde paket komutlarını
     * ayrıştırmak için kullanılır:
     * ```cpp
     * for (CAttributeCommandIterator it(packet); !it.IsFinished(); ++it)
     * {
     *     if (it->GetCommandId() == CMD_SET_HEALTH)
     *         m_Health = *static_cast<const RwReal*>(it->GetCommandData());
     * }
     * ```
     */
    class CAttributeCommandIterator
    {
    public:
        /**
         * @brief Yineleyiciyi başlatır.
         * @param packet     Ayrıştırılacak paket.
         * @param classIndex Hedef sınıfın paket içindeki indeksi.
         */
        CAttributeCommandIterator(const CAttributePacket& packet, RwUInt32 classIndex = 0)
            : m_bFinished(true)
        {
            m_Current.m_CommandId = 0;
            m_Current.m_pCommandData = 0;
        }

        /// @return TRUE ise tüm komutlar işlendi
        RwBool IsFinished() const { return m_bFinished; }
        /// @brief Bir sonraki komuta geçer
        void operator++() { m_bFinished = true; }
        /// @return Mevcut komut girişine işaretçi
        const CAttributeCommandEntry* operator->() const { return &m_Current; }

    private:
        RwBool               m_bFinished; ///< Yineleyici bitişi
        CAttributeCommandEntry m_Current; ///< Mevcut komut girişi
    };

    /**
     * @class CAttributeHandler
     * @brief Nesne yapılandırma özelliklerini işleyen temel sınıf.
     *
     * CannonCruise'daki tüm yapılandırılabilir oyun nesneleri (CShip, CCannon,
     * CSpeakManager vb.) bu sınıftan türeyerek RenderWare Studio'nun
     * binary varlık sistemiyle uyumlu hale gelir.
     *
     * **Kullanım akışı:**
     * 1. `CAttributeHandler::Initialize()` uygulama başında çağrılır.
     * 2. Varlık paketleri `ProcessPacket()` ile işlenir; yeni nesne örnekleri
     *    `CClassFactory` aracılığıyla oluşturulur ve `RegisterInstance()` ile kaydedilir.
     * 3. Her nesne `HandleAttributes()` override ederek kendi özelliklerini alır.
     *
     * **Binary referansları:**
     * - Instance: `FUN_00402000`
     * - ProcessPacket: `FUN_00402100`
     * - FindInstance: `FUN_004021c0`
     * - RegisterInstance: `FUN_00402240`
     */
    class CAttributeHandler
    {
    public:
        /**
         * @struct EntityInstance
         * @brief Kayıtlı bir nesne örneğini tanımlayan yapı.
         */
        struct EntityInstance
        {
            RwUInt32 m_InstanceId; ///< Benzersiz örnek kimliği
            void*    m_pObject;    ///< Nesne işaretçisi
            RwUInt32 m_Flags;      ///< Örnek bayrakları
        };

        /// @brief Varsayılan yapıcı
        CAttributeHandler();
        /**
         * @brief Özellik paketi ile yapıcı (CClassFactory tarafından çağrılır).
         * @param packet Başlangıç özellik paketi.
         */
        CAttributeHandler(const CAttributePacket& packet);
        /// @brief Sanal yıkıcı
        virtual ~CAttributeHandler();

        /**
         * @brief Özellik paketini işleyen sanal fonksiyon.
         * @param packet İşlenecek özellik paketi.
         *
         * Türetilmiş sınıflar bu fonksiyonu override ederek
         * `CAttributeCommandIterator` ile paket komutlarını ayrıştırır.
         */
        virtual void HandleAttributes(const CAttributePacket& packet) {}

        /// @return Tekil CAttributeHandler örneğine işaretçi
        static CAttributeHandler* Instance();
        /// @brief Özellik sistemini başlatır
        static void Initialize();
        /// @brief Özellik sistemini kapatır
        static void Shutdown();

        /**
         * @brief Ham paket verisini işler ve nesneleri oluşturur/yapılandırır.
         * @param pPacketData Ham paket verisi.
         * @param dataSize    Veri boyutu (bayt).
         */
        static void ProcessPacket(const RwUInt8* pPacketData, RwUInt32 dataSize);

        /**
         * @brief Kayıtlı bir nesne örneğini kimliğiyle arar.
         * @param instanceId Aranan örnek kimliği.
         * @return void* — Nesne işaretçisi veya nullptr.
         */
        static void* FindInstance(RwUInt32 instanceId);

        /**
         * @brief Bir nesne örneğini sisteme kaydeder.
         * @param instanceId Örnek kimliği.
         * @param pObject    Nesne işaretçisi.
         */
        static void RegisterInstance(RwUInt32 instanceId, void* pObject);

        /**
         * @brief Bir nesne örneğini sistemden kaldırır.
         * @param instanceId Kaldırılacak örnek kimliği.
         */
        static void UnregisterInstance(RwUInt32 instanceId);

        /**
         * @brief Paketin bir nesne oluşturma paketi olup olmadığını kontrol eder.
         * @param pPacket Ham paket verisi.
         * @return TRUE ise paket yeni bir nesne oluşturur.
         */
        static RwBool IsInstanceCreationPacket(const RwUInt8* pPacket);
        /**
         * @brief Paketin bir bitiş yığını olup olmadığını kontrol eder.
         * @param pPacket Ham paket verisi.
         * @return TRUE ise bu paketin ardından yeni bir yığın başlamaz.
         */
        static RwBool IsEndChunk(const RwUInt8* pPacket);
        /**
         * @brief Tüm paketlerin işlenip işlenmediğini kontrol eder.
         * @param pPacket Ham paket verisi.
         * @return TRUE ise işlem tamamlandı.
         */
        static RwBool IsFinished(const RwUInt8* pPacket);

    private:
        static CAttributeHandler* sm_pInstance; ///< Tekil örnek işaretçisi

        /// @brief Örnek kimliği → EntityInstance eşleştirme tablosu (g_ptheMap / DAT_00635650)
        std::map<RwUInt32, EntityInstance> m_InstanceMap;
    };
}

#include "cclassfactory.h"

#endif // RWS_CATTRIBUTEHANDLER_H


