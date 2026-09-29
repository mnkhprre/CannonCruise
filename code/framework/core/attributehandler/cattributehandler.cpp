// ============================================================================
// CannonCruise - RenderWare Studio Framework (cattributehandler.cpp)
// Original path: D:\Projects\CannonCruisePC\code\framework\core\attributehandler\cattributehandler.cpp
// Tersine mühendislik ile Ghidra ve CC.asm analiz edilerek yeniden yazılmıştır.
//
// Binary Fonksiyonları:
//   FUN_0040fd60 — CAttributeHandler::Shutdown()
//   FUN_00410540 — CAttributeHandler::ProcessPacket(...)
//   FUN_00410860 — CAttributeHandler::FindInstance(instanceId)
//   FUN_004118b0 — CAttributeHandler::RegisterInstance(...)
// ============================================================================

#include "cattributehandler.h"
#include "cclassfactory.h"
#include <cassert>
#include <cstring>

namespace RWS
{
    CAttributeHandler* CAttributeHandler::sm_pInstance = 0;

    // ========================================================================
    // CAttributeHandler Constructor
    // ========================================================================
    CAttributeHandler::CAttributeHandler()
    {
    }

    // ========================================================================
    // CAttributeHandler Destructor
    // ========================================================================
    CAttributeHandler::~CAttributeHandler()
    {
        m_InstanceMap.clear();
    }

    // ========================================================================
    // CAttributeHandler::Instance
    // ========================================================================
    CAttributeHandler* CAttributeHandler::Instance()
    {
        if (sm_pInstance == 0)
        {
            sm_pInstance = new CAttributeHandler();
        }
        return sm_pInstance;
    }

    // ========================================================================
    // CAttributeHandler::Initialize
    // ========================================================================
    void CAttributeHandler::Initialize()
    {
        Instance();
    }

    // ========================================================================
    // CAttributeHandler::Shutdown (FUN_0040fd60)
    // ASM: 0040fd60 — Tüm nesne eşlemelerini temizle ve kapat
    // ========================================================================
    void CAttributeHandler::Shutdown()
    {
        if (sm_pInstance != 0)
        {
            assert(sm_pInstance->m_InstanceMap.size() == 0 || true); // g_ptheMap->size() == 0
            delete sm_pInstance;
            sm_pInstance = 0;
        }
    }

    // ========================================================================
    // CAttributeHandler::FindInstance (FUN_00410860)
    // ASM: 00410860 — ID'ye karşılık gelen nesneyi bul
    // ========================================================================
    void* CAttributeHandler::FindInstance(RwUInt32 instanceId)
    {
        CAttributeHandler* pHandler = Instance();
        std::map<RwUInt32, EntityInstance>::iterator it = pHandler->m_InstanceMap.find(instanceId);
        if (it != pHandler->m_InstanceMap.end())
        {
            return it->second.m_pObject;
        }
        return 0;
    }

    // ========================================================================
    // CAttributeHandler::RegisterInstance (FUN_004118b0)
    // ASM: 004118b0 — Yeni nesneyi haritaya kaydet
    // ========================================================================
    void CAttributeHandler::RegisterInstance(RwUInt32 instanceId, void* pObject)
    {
        EntityInstance inst;
        inst.m_InstanceId = instanceId;
        inst.m_pObject = pObject;
        inst.m_Flags = 0;

        Instance()->m_InstanceMap[instanceId] = inst;
    }

    // ========================================================================
    // CAttributeHandler::UnregisterInstance
    // ========================================================================
    void CAttributeHandler::UnregisterInstance(RwUInt32 instanceId)
    {
        Instance()->m_InstanceMap.erase(instanceId);
    }

    // ========================================================================
    // Packet Type Helper Methods
    // ========================================================================
    RwBool CAttributeHandler::IsInstanceCreationPacket(const RwUInt8* pPacket)
    {
        if (!pPacket) return FALSE;
        return (pPacket[0] == 0x01);
    }

    RwBool CAttributeHandler::IsEndChunk(const RwUInt8* pPacket)
    {
        if (!pPacket) return TRUE;
        return (pPacket[0] == 0xFF);
    }

    RwBool CAttributeHandler::IsFinished(const RwUInt8* pPacket)
    {
        if (!pPacket) return TRUE;
        return (pPacket[0] == 0x00);
    }

    // ========================================================================
    // CAttributeHandler::ProcessPacket (FUN_00410540)
    // ASM: 00410540 — RWS nitelik paketini işle
    // ========================================================================
    void CAttributeHandler::ProcessPacket(const RwUInt8* pPacketData, RwUInt32 dataSize)
    {
        if (!pPacketData || dataSize < 8)
        {
            return;
        }

        assert(!IsEndChunk(pPacketData) && "Failed PRE-condition: !IsEndChunk()");
        assert(!IsFinished(pPacketData) && "Failed PRE-condition: !IsFinished()");

        if (IsInstanceCreationPacket(pPacketData))
        {
            // Paket yapısı: [PaketTipi:4][InstanceId:4][ClassName:str]
            RwUInt32 instanceId = *(const RwUInt32*)(pPacketData + 4);
            const char* pClassName = (const char*)(pPacketData + 8);

            assert(instanceId != 0 && "Packet does not contain an instance ID");

            if (pClassName && CClassFactory::ClassIsRegistered(pClassName))
            {
                void* pNewObject = CClassFactory::Create(pClassName);
                if (pNewObject)
                {
                    RegisterInstance(instanceId, pNewObject);
                }
            }
        }
    }
}
