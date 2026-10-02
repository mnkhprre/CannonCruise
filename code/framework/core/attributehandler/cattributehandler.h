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
#include <cassert>

#ifndef RWS_ASSERT
#define RWS_ASSERT(cond, msg) assert((cond) && (msg))
#endif

namespace RWS

{
    class CAttributePacket
    {
    public:
        CAttributePacket() : m_pData(0), m_DataSize(0) {}
        CAttributePacket(const void* pData, RwUInt32 size) : m_pData(pData), m_DataSize(size) {}

        const void* GetData() const { return m_pData; }
        RwUInt32 GetSize() const { return m_DataSize; }

    private:
        const void* m_pData;
        RwUInt32 m_DataSize;
    };

    struct CAttributeCommandEntry
    {
        RwUInt32 m_CommandId;
        const void* m_pCommandData;

        RwUInt32 GetCommandId() const { return m_CommandId; }
        const void* GetCommandData() const { return m_pCommandData; }
    };

    class CAttributeCommandIterator
    {
    public:
        CAttributeCommandIterator(const CAttributePacket& packet, RwUInt32 classIndex = 0)
            : m_bFinished(true)
        {
            m_Current.m_CommandId = 0;
            m_Current.m_pCommandData = 0;
        }

        RwBool IsFinished() const { return m_bFinished; }
        void operator++() { m_bFinished = true; }
        const CAttributeCommandEntry* operator->() const { return &m_Current; }

    private:
        RwBool m_bFinished;
        CAttributeCommandEntry m_Current;
    };

    class CAttributeHandler
    {
    public:
        struct EntityInstance
        {
            RwUInt32 m_InstanceId;
            void* m_pObject;
            RwUInt32 m_Flags;
        };

        CAttributeHandler() {}
        CAttributeHandler(const CAttributePacket& packet) {}
        virtual ~CAttributeHandler() {}

        virtual void HandleAttributes(const CAttributePacket& packet) {}

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
        static CAttributeHandler* sm_pInstance;

        // Instance ID -> Entity mapping (g_ptheMap / DAT_00635650)
        std::map<RwUInt32, EntityInstance> m_InstanceMap;
    };
}

#include "cclassfactory.h"

#endif // RWS_CATTRIBUTEHANDLER_H


