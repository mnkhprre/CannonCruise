// ============================================================================
// CannonCruise - RenderWare Studio Framework (cmaterialevent.cpp)
// Original path: D:\Projects\CannonCruisePC\code\framework\toolkits\world\cmaterialevent.cpp
// Tersine mühendislik ile Ghidra ve CC.asm analiz edilerek yeniden yazılmıştır.
//
// Binary Fonksiyonları:
//   FUN_0042b090 — CMaterialEvent::PluginAttach
//   FUN_0042b310 — CMaterialEvent::GetMaterialData
// ============================================================================

#include "cmaterialevent.h"
#include <cassert>
#include <cstring>

namespace RWS
{
    // RenderWare Studio Material Plugin ID (0x01 = RWS material event ID)
    #define rwID_MATERIAL_EVENT_PLUGIN 0x01

    RwInt32 CMaterialEvent::sm_PluginOffset = -1;

    void* CMaterialEvent::MaterialConstructor(void* object, RwInt32 offsetInObject, RwInt32 sizeInObject)
    {
        MaterialEventData* pData = (MaterialEventData*)((RwUInt8*)object + offsetInObject);
        pData->m_EventId = 0;
        pData->m_MaterialIndex = 0;
        memset(pData->m_Tag, 0, sizeof(pData->m_Tag));
        return object;
    }

    void* CMaterialEvent::MaterialDestructor(void* object, RwInt32 offsetInObject, RwInt32 sizeInObject)
    {
        return object;
    }

    void* CMaterialEvent::MaterialCopy(void* dstObject, const void* srcObject, RwInt32 offsetInObject, RwInt32 sizeInObject)
    {
        MaterialEventData* pDst = (MaterialEventData*)((RwUInt8*)dstObject + offsetInObject);
        const MaterialEventData* pSrc = (const MaterialEventData*)((const RwUInt8*)srcObject + offsetInObject);
        *pDst = *pSrc;
        return dstObject;
    }

    // ========================================================================
    // CMaterialEvent::PluginAttach (FUN_0042b090)
    // ASM: 0042b090 — RpMaterialRegisterPlugin ile eklentiyi kaydet
    // ========================================================================
    RwBool CMaterialEvent::PluginAttach()
    {
        assert(sm_PluginOffset == -1 && "Failed PRE-condition: !iCMaterialEventPlugin_Offset");

        sm_PluginOffset = RpMaterialRegisterPlugin(
            sizeof(MaterialEventData),
            rwID_MATERIAL_EVENT_PLUGIN,
            MaterialConstructor,
            MaterialDestructor,
            MaterialCopy
        );

        assert(sm_PluginOffset != -1 && "Failed POST-condition: iCMaterialEventPlugin_Offset");
        return (sm_PluginOffset != -1);
    }

    // ========================================================================
    // CMaterialEvent::GetMaterialData (FUN_0042b310)
    // ASM: 0042b310 — Materyaldeki eklenti verisini al
    // ========================================================================
    MaterialEventData* CMaterialEvent::GetMaterialData(RpMaterial* pMaterial)
    {
        if (!pMaterial || sm_PluginOffset == -1) return 0;
        assert(sm_PluginOffset != -1 && "Failed PRE-condition: iCMaterialEventPlugin_Offset");

        return (MaterialEventData*)((RwUInt8*)pMaterial + sm_PluginOffset);
    }

    void CMaterialEvent::SetMaterialEventId(RpMaterial* pMaterial, RwUInt32 eventId)
    {
        MaterialEventData* pData = GetMaterialData(pMaterial);
        if (pData)
        {
            pData->m_EventId = eventId;
        }
    }
}
