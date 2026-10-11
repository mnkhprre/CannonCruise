#ifndef RWS_CSYSTEMCOMMANDS_H
#define RWS_CSYSTEMCOMMANDS_H

// ============================================================================
// CannonCruise - RenderWare Studio Framework (csystemcommands.h)
// Original path: D:\Projects\CannonCruisePC\code\framework\core\attributehandler\csystemcommands.h
// Tersine mühendislik ile Ghidra ve CC.asm analiz edilerek yeniden yazılmıştır.
// Binary ref: FUN_00403400 - FUN_004035a0 (RWS::CSystemCommands)
// ============================================================================

#include <rwcore.h>
#include <rpworld.h>

namespace RWS
{
    /**
     * @enum SystemCommandId
     * @brief Sistem düzeyindeki özellik komut kimlikleri.
     *
     * `CAttributeHandler::ProcessPacket()` tarafından üretilen sistem komutları
     * `CSystemCommands` işleyicilerine yönlendirilmeden önce bu kimliklerle tanımlanır.
     */
    enum SystemCommandId
    {
        CMD_LoadMatrix   = 0x01, ///< RwFrame dönüşüm matrisini yükle
        CMD_SetParent    = 0x02, ///< Çocuk çerçeveyi üst çerçeveye bağla
        CMD_SetFlags     = 0x03, ///< RpAtomic bayrağını ayarla
        CMD_AttachAtomic = 0x04  ///< Atomiki belirtilen çerçeveye ekle
    };

    /**
     * @struct CAttributeCommand
     * @brief Bir özellik paketi komutunu temsil eden yapı.
     *
     * `CSystemCommands` işleyicilerine geçirilen komut kaydıdır.
     * Komut kimliği, veri boyutu ve veri işaretçisi içerir.
     */
    struct CAttributeCommand
    {
        RwUInt32    m_CommandId; ///< Komut kimliği (SystemCommandId değeri)
        RwUInt32    m_DataSize;  ///< Komut verisi boyutu (bayt)
        const void* m_pData;    ///< Komut verisi işaretçisi

        /// @return Komut kimliği
        RwUInt32    GetCommandId() const { return m_CommandId; }
        /// @return Komut verisi işaretçisi
        const void* GetData()      const { return m_pData; }
        /// @return Komut verisi boyutu
        RwUInt32    GetDataSize()  const { return m_DataSize; }
    };

    /**
     * @class CSystemCommands
     * @brief Temel RenderWare nesneleri için sistem düzeyi özellik komut işleyicileri.
     *
     * `CAttributeHandler::ProcessPacket()` bir `CMD_LoadMatrix`, `CMD_SetParent` vb.
     * sistem komutu aldığında bu sınıfın statik metotları doğrudan çağrılır.
     * Oyun mantığına özgü komutlar ise `CAttributeHandler::HandleAttributes()`
     * sanal fonksiyonu aracılığıyla ilgili sınıfa iletilir.
     *
     * **Binary referansları:**
     * - HandleLoadMatrix: `FUN_00403400`
     * - HandleSetParent: `FUN_00403460`
     * - HandleAttachAtomic: `FUN_004034c0`
     * - HandleSetFlags: `FUN_00403520`
     */
    class CSystemCommands
    {
    public:
        /**
         * @brief RwFrame üzerine dönüşüm matrisini uygular.
         * @param pFrame   Hedef çerçeve.
         * @param attrCmd  Matris verisini içeren komut.
         */
        static void HandleLoadMatrix(RwFrame* pFrame, const CAttributeCommand& attrCmd);

        /**
         * @brief Çocuk çerçeveyi üst çerçeveye bağlar.
         * @param pChildFrame  Bağlanacak çocuk çerçeve.
         * @param pParentFrame Üst çerçeve.
         */
        static void HandleSetParent(RwFrame* pChildFrame, RwFrame* pParentFrame);

        /**
         * @brief Atomiki belirtilen çerçeveye ekler.
         * @param pAtomic Eklenecek RpAtomic nesnesi.
         * @param pFrame  Hedef RwFrame.
         */
        static void HandleAttachAtomic(RpAtomic* pAtomic, RwFrame* pFrame);

        /**
         * @brief RpAtomic bayrağını ayarlar.
         * @param pAtomic Hedef RpAtomic nesnesi.
         * @param flags   Ayarlanacak bayrak değeri.
         */
        static void HandleSetFlags(RpAtomic* pAtomic, RwUInt32 flags);

        /**
         * @brief Özellik paketinden RpClump işaretçisini çıkarır.
         * @param packet Ayrıştırılacak özellik paketi.
         * @return const void* — Clump verisi (CannonCruise'da stub, her zaman nullptr).
         */
        static const void* ExtractClump(const class CAttributePacket& packet) { return 0; }

        /**
         * @brief Mesajdan RpClump işaretçisini çıkarır.
         * @param msg Ayrıştırılacak mesaj.
         * @return const void* — Clump verisi (CannonCruise'da stub, her zaman nullptr).
         */
        static const void* ExtractClump(const struct CMsg& msg) { return 0; }
    };
}


#endif // RWS_CSYSTEMCOMMANDS_H
