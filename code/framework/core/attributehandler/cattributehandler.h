#ifndef RWS_CATTRIBUTEHANDLER_H
#define RWS_CATTRIBUTEHANDLER_H

// ============================================================================
// CannonCruise - RenderWare Studio Framework (cattributehandler.h)
// Original path: D:\Projects\CannonCruisePC\code\framework\core\attributehandler\cattributehandler.cpp
// Tersine mühendislik ile Ghidra ve CC.asm analiz edilerek yeniden yazılmıştır.
// ============================================================================

#include <rwcore.h>
#include <map>
#include <string>

namespace RWS
{
    class CAttributeHandler
    {
    public:
        struct EntityInstance
        {
            RwUInt32 m_InstanceId;
            void* m_pObject;
            RwUInt32 m_Flags;
        };

        static CAttributeHandler* Instance();
        static void Initialize();
        static void Shutdown();

        static void ProcessPacket(const RwUInt8* pPacketData, RwUInt32 dataSize);
        static void* FindInstance(RwUInt32 instanceId);
        static void RegisterInstance(RwUInt32 instanceId, void* pObject);
        static void UnregisterInstance(RwUInt32 instanceId);

        static RwBool IsInstanceCreationPacket(const RwUInt8* pPacket);
        static RwBool IsEndChunk(const RwUInt8* pPacket);
        static RwBool IsFinished(const RwUInt8* pPacket);

    private:
        CAttributeHandler();
        ~CAttributeHandler();

        static CAttributeHandler* sm_pInstance;

        // Instance ID -> Entity mapping (g_ptheMap / DAT_00635650)
        std::map<RwUInt32, EntityInstance> m_InstanceMap;
    };
}

#endif // RWS_CATTRIBUTEHANDLER_H
