#ifndef RWS_CMATERIALEVENT_H
#define RWS_CMATERIALEVENT_H

// ============================================================================
// CannonCruise - RenderWare Studio Framework (cmaterialevent.h)
// Original path: D:\Projects\CannonCruisePC\code\framework\toolkits\world\cmaterialevent.h
// Tersine mühendislik ile Ghidra ve CC.asm analiz edilerek yeniden yazılmıştır.
// Binary ref: FUN_0040a000 - FUN_0040a200 (RWS::CMaterialEvent)
// ============================================================================

#include <rwcore.h>
#include <rpworld.h>
#include <string>

namespace RWS
{
    /**
     * @struct MaterialEventData
     * @brief Materyallere eklenen (plugin verisi) çarpışma olayı yapısı.
     *
     * Dünyadaki zemin (su, kum, taş) veya duvar gibi materyallere özellik
     * atamak için kullanılır. Top mermisi vb. çarpıştığında materyale özgü
     * partikül efektleri veya sesler çıkarmak için `m_EventId` kontrol edilir.
     */
    struct MaterialEventData
    {
        RwUInt32 m_EventId;       ///< Olay kimliği (Örn: EVENT_HIT_WATER)
        RwUInt32 m_MaterialIndex; ///< Materyalin modeli içindeki dizini
        RwChar m_Tag[32];         ///< Materyale atanan özel etiket/isim
    };

    /**
     * @class CMaterialEvent
     * @brief RenderWare RpMaterial nesnelerine olay (event) verisi ekleyen eklenti yöneticisi.
     *
     * `RpMaterialRegisterPlugin()` kullanılarak materyal yapısına ekstra
     * alan tahsis edilir. Bu sayede fizik motorundan gelen materyal bilgisi,
     * RWS olay sistemine (CEventHandler) bağlanır.
     *
     * **Binary referansları:**
     * - PluginAttach: `FUN_0040a000`
     * - GetMaterialData: `FUN_0040a080`
     * - SetMaterialEventId: `FUN_0040a0f0`
     */
    class CMaterialEvent
    {
    public:
        /**
         * @brief RenderWare moturuna materyal olay eklentisini (plugin) kaydeder.
         * @return TRUE ise eklenti başarıyla kaydedildi.
         *
         * `RwEngineInit()` çağrısından sonra, fakat objeler yüklenmeden
         * önce (genellikle `AttachPlugins` içinde) çağrılmalıdır.
         */
        static RwBool PluginAttach();

        /**
         * @brief Belirtilen materyalin olay verisine (plugin alanı) işaretçi döndürür.
         * @param pMaterial Hedef RenderWare materyali.
         * @return MaterialEventData* — Materyalin olay verisi.
         */
        static MaterialEventData* GetMaterialData(RpMaterial* pMaterial);

        /**
         * @brief Materyale bir çarpışma/etkileşim olay kimliği atar.
         * @param pMaterial Hedef materyal.
         * @param eventId   Atanacak `CEventId` (Örn: iMsgHit).
         */
        static void SetMaterialEventId(RpMaterial* pMaterial, RwUInt32 eventId);

    private:
        static RwInt32 sm_PluginOffset; ///< Plugin verisinin RpMaterial içindeki bayt ofseti

        // RenderWare eklenti geri çağrıları (callbacks)
        static void* MaterialConstructor(void* object, RwInt32 offsetInObject, RwInt32 sizeInObject);
        static void* MaterialDestructor(void* object, RwInt32 offsetInObject, RwInt32 sizeInObject);
        static void* MaterialCopy(void* dstObject, const void* srcObject, RwInt32 offsetInObject, RwInt32 sizeInObject);
    };
}

#endif // RWS_CMATERIALEVENT_H

