#ifndef RWS_CRESOURCEMANAGER_H
#define RWS_CRESOURCEMANAGER_H

// ============================================================================
// CannonCruise - RenderWare Studio Framework (cresourcemanager.h)
// Original path: D:\Projects\CannonCruisePC\code\framework\core\resourcemanager\cresourcemanager.h
// Tersine mühendislik ile Ghidra ve CC.asm analiz edilerek yeniden yazılmıştır.
// Binary ref: FUN_00405000 - FUN_004052e0 (RWS::CResourceManager)
// ============================================================================

#include <rwcore.h>
#include <map>
#include <string>

namespace RWS
{
    /**
     * @struct CResource
     * @brief Kayıtlı bir oyun kaynağını (asset) tanımlayan veri yapısı.
     *
     * `CResourceManager::RegisterResource()` tarafından oluşturulur ve
     * kaynak tablosuna eklenir. Tip, ad, yol, ham veri işaretçisi,
     * referans sayacı ve bayrakları içerir.
     *
     * **Kaynak tipleri (m_Type):**
     * - `"CTextureResource"` — Doku kaynağı
     * - `"CWorldGeometryResource"` — BSP dünya geometrisi
     * - `"CPathFindingResource"` — Yol bulma haritası
     * - `"CAudioResource"` — Ses dosyası
     */
    struct CResource
    {
        std::string m_Type;     ///< Kaynak tipi dizesi (örn. "CTextureResource")
        std::string m_Name;     ///< İnsan okunabilir kaynak adı
        std::string m_Path;     ///< Dosya sistemi yolu
        void*       m_pData;    ///< Yüklenmiş kaynak verisi (tipe özel dönüşüm gerekir)
        RwUInt32    m_RefCount; ///< Referans sayacı (0 → serbest bırakılabilir)
        RwUInt32    m_Flags;    ///< Kaynak bayrakları (yüklü, kalıcı vb.)

        /// @brief Varsayılan yapıcı — tüm alanları sıfırlar
        CResource() : m_pData(0), m_RefCount(0), m_Flags(0) {}
    };

    /**
     * @class CResourceManager
     * @brief Oyun kaynaklarını (asset) merkezi olarak yönetir.
     *
     * CannonCruise'da dokular, dünya geometrileri ve ses kaynakları bu
     * yönetici aracılığıyla yüklenir ve paylaşılır. Referans sayacı
     * mekanizmasıyla aynı kaynağın birden fazla nesne tarafından kullanılmasına
     * izin verilir; sayaç sıfıra düştüğünde kaynak bellekten kaldırılır.
     *
     * **Kullanım akışı:**
     * 1. `Initialize()` — Uygulama başında
     * 2. `RegisterResource()` — Kaynak yüklendiğinde
     * 3. `FindResource()` — Kaynak kullanılacağında
     * 4. `UnRegisterResource()` / `UnRegisterAll()` — Sahne kapatılırken
     *
     * **Binary referansları:**
     * - Instance: `FUN_00405000`
     * - FindResource: `FUN_00405080`
     * - RegisterResource: `FUN_004050f0`
     * - UnRegisterResource: `FUN_00405180`
     */
    class CResourceManager
    {
    public:
        /// @return Tekil CResourceManager örneğine işaretçi
        static CResourceManager* Instance();
        /// @brief Kaynak yöneticisini başlatır
        static void Initialize();
        /// @brief Kaynak yöneticisini kapatır, tüm kaynakları serbest bırakır
        static void Shutdown();

        /**
         * @brief Belirtilen kimlik ve tiple kayıtlı kaynağı bulur.
         * @param pResId    Kaynak kimliği dizesi.
         * @param pStrType  Kaynak tipi dizesi (isteğe bağlı filtre).
         * @return void* — Kaynak verisi işaretçisi; bulunamazsa nullptr.
         */
        static void* FindResource(const char* pResId, const char* pStrType = 0);

        /**
         * @brief `FindResource()` için uyumluluk sarmalayıcısı.
         * @param pResId   Kaynak kimliği.
         * @param pStrType Kaynak tipi dizesi.
         * @param pSize    İsteğe bağlı boyut çıkış işaretçisi (kullanılmıyor).
         * @return void* — Kaynak verisi işaretçisi.
         */
        static void* Find(const char* pResId, const char* pStrType = 0, RwUInt32* pSize = 0)
        {
            return FindResource(pResId, pStrType);
        }

        /**
         * @brief Yeni bir kaynağı yöneticiye kaydeder.
         * @param pResId   Benzersiz kaynak kimliği.
         * @param pStrType Kaynak tipi dizesi.
         * @param pStrName İnsan okunabilir kaynak adı.
         * @param pData    Kaynak veri işaretçisi.
         * @return TRUE ise kayıt başarılı.
         */
        static RwBool RegisterResource(const char* pResId, const char* pStrType,
                                       const char* pStrName, void* pData);

        /**
         * @brief Belirtilen kaynağın kaydını kaldırır.
         * @param pResId Kaydı kaldırılacak kaynak kimliği.
         */
        static void UnRegisterResource(const char* pResId);

        /// @brief Tüm kayıtlı kaynakları kaldırır
        static void UnRegisterAll();

    private:
        CResourceManager();
        ~CResourceManager();

        static CResourceManager* sm_pInstance; ///< Tekil örnek işaretçisi

        std::map<std::string, CResource> m_Resources; ///< ResId → CResource eşleştirme tablosu
    };
}

#endif // RWS_CRESOURCEMANAGER_H
