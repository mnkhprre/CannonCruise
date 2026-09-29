#ifndef RWS_CMATERIALEVENT_H
#define RWS_CMATERIALEVENT_H

// ============================================================================
// CannonCruise - RenderWare Studio Framework (cmaterialevent.h)
// Original path: D:\Projects\CannonCruisePC\code\framework\toolkits\world\cmaterialevent.h
// Tersine mühendislik ile Ghidra ve CC.asm analiz edilerek yeniden yazılmıştır.
// ============================================================================

#include <rwcore.h>
#include <rpworld.h>
#include <string>

namespace RWS
{
    struct MaterialEventData
    {
        RwUInt32 m_EventId;
        RwUInt32 m_MaterialIndex;
        RwChar m_Tag[32];
    };

    class CMaterialEvent
    {
    public:
        // RenderWare Material eklentisini bağla
        static RwBool PluginAttach();

        // Materyal olay verisine erişim
        static MaterialEventData* GetMaterialData(RpMaterial* pMaterial);
        static void SetMaterialEventId(RpMaterial* pMaterial, RwUInt32 eventId);

    private:
        static RwInt32 sm_PluginOffset;

        // RenderWare eklenti geri çağrıları
        static void* MaterialConstructor(void* object, RwInt32 offsetInObject, RwInt32 sizeInObject);
        static void* MaterialDestructor(void* object, RwInt32 offsetInObject, RwInt32 sizeInObject);
        static void* MaterialCopy(void* dstObject, const void* srcObject, RwInt32 offsetInObject, RwInt32 sizeInObject);
    };
}

#endif // RWS_CMATERIALEVENT_H
