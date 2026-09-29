// ============================================================================
// CannonCruise - RenderWare Studio Framework (ceventhandler.cpp)
// Original path: D:\Projects\CannonCruisePC\code\framework\core\eventhandler\ceventhandler.cpp
// Tersine mühendislik ile Ghidra ve CC.asm analiz edilerek yeniden yazılmıştır.
//
// Binary Fonksiyonları:
//   FUN_0040b090 — CEventHandler vtable setup
//   FUN_0040be80 — CEventHandler::LinkMsg(...)
//   FUN_0040c120 — CEventHandler::UnLinkMsg(...)
//   FUN_0040c6b0 — CEventHandler::SendMsg(...)
//   FUN_0040ca30 — CEventHandler::RegisterMsg(...)
// ============================================================================

#include "ceventhandler.h"
#include <cassert>
#include <map>
#include <vector>

namespace RWS
{
    // Global olay yöneticisi veri yapıları
    struct RegisteredEventInfo
    {
        std::string m_Name;
        std::string m_Format;
    };

    static std::map<CEventId, RegisteredEventInfo> s_RegisteredEvents;
    static std::map<CEventId, std::vector<CEventHandler*> > s_EventSubscribers;
    static CEventId s_NextEventId = 1000;
    static RwFreeList* s_pEventHandlerFreeList = 0;

    // ========================================================================
    // CEventHandler Constructor
    // ========================================================================
    CEventHandler::CEventHandler()
    {
    }

    // ========================================================================
    // CEventHandler Destructor
    // ========================================================================
    CEventHandler::~CEventHandler()
    {
        UnLinkAllProcess();
    }

    // ========================================================================
    // CEventHandler::HandleEvents
    // ========================================================================
    void CEventHandler::HandleEvents(const CMsg& msg)
    {
        // Alt sınıflar tarafından override edilir
    }

    // ========================================================================
    // CEventHandler::LinkMsg (FUN_0040be80)
    // ASM: 0040be80 — Bir olaya abone ol
    // ========================================================================
    void CEventHandler::LinkMsg(CEventId eventId, const char* pFormat, RwUInt32 priority)
    {
        // Ön koşul kontrolü
        for (size_t i = 0; i < m_LinkedEvents.size(); ++i)
        {
            if (m_LinkedEvents[i].m_EventId == eventId)
            {
                assert(m_LinkedEvents[i].m_Priority == priority &&
                       "link to an event, that this event handler is already linked to but with a different priority.");
                return;
            }
        }

        EventLink link;
        link.m_EventId = eventId;
        link.m_Priority = priority;
        if (pFormat)
        {
            link.m_Format = pFormat;
        }

        m_LinkedEvents.push_back(link);
        s_EventSubscribers[eventId].push_back(this);

        assert(m_LinkedEvents.size() <= CEVENT_HANDLER_REGISTERED_MSG_DEFAULT_BLOCK_SIZE &&
               "Linked more than messages. Please increase CEVENT_HANDLER_REGISTERED_MSG_DEFAULT_BLOCK_SIZE in file: ceventhandler.cpp");
    }

    // ========================================================================
    // CEventHandler::UnLinkMsg (FUN_0040c120)
    // ASM: 0040c120 — Bir olaya olan aboneliği kaldır
    // ========================================================================
    void CEventHandler::UnLinkMsg(CEventId eventId)
    {
        for (std::vector<EventLink>::iterator it = m_LinkedEvents.begin();
             it != m_LinkedEvents.end(); ++it)
        {
            if (it->m_EventId == eventId)
            {
                m_LinkedEvents.erase(it);
                break;
            }
        }

        std::map<CEventId, std::vector<CEventHandler*> >::iterator mapIt = s_EventSubscribers.find(eventId);
        if (mapIt != s_EventSubscribers.end())
        {
            std::vector<CEventHandler*>& list = mapIt->second;
            for (std::vector<CEventHandler*>::iterator subIt = list.begin();
                 subIt != list.end(); ++subIt)
            {
                if (*subIt == this)
                {
                    list.erase(subIt);
                    break;
                }
            }
        }
    }

    // ========================================================================
    // CEventHandler::UnLinkAllProcess
    // ========================================================================
    void CEventHandler::UnLinkAllProcess()
    {
        while (!m_LinkedEvents.empty())
        {
            UnLinkMsg(m_LinkedEvents.back().m_EventId);
        }
    }

    // ========================================================================
    // CEventHandler::SendMsg (FUN_0040c6b0)
    // ASM: 0040c6b0 — Olayı tüm abonelere dağıt
    // ========================================================================
    void CEventHandler::SendMsg(const CMsg& msg)
    {
        std::map<CEventId, std::vector<CEventHandler*> >::iterator mapIt = s_EventSubscribers.find(msg.m_Id);
        if (mapIt != s_EventSubscribers.end())
        {
            const std::vector<CEventHandler*>& subscribers = mapIt->second;
            for (size_t i = 0; i < subscribers.size(); ++i)
            {
                CEventHandler* pHandler = subscribers[i];
                if (pHandler)
                {
                    pHandler->HandleEvents(msg);
                }
            }
        }
    }

    // ========================================================================
    // CEventHandler::RegisterMsg (FUN_0040ca30)
    // ASM: 0040ca30 — Yeni bir olay adı kaydet
    // ========================================================================
    CEventId CEventHandler::RegisterMsg(const char* pEventName, const char* pFormat)
    {
        assert(pEventName != 0);

        // Olay adı zaten kayıtlı mı kontrol et
        for (std::map<CEventId, RegisteredEventInfo>::iterator it = s_RegisteredEvents.begin();
             it != s_RegisteredEvents.end(); ++it)
        {
            if (it->second.m_Name == pEventName)
            {
                return it->first;
            }
        }

        CEventId newId = s_NextEventId++;
        RegisteredEventInfo info;
        info.m_Name = pEventName;
        if (pFormat)
        {
            info.m_Format = pFormat;
        }

        s_RegisteredEvents[newId] = info;

        assert(s_RegisteredEvents.size() <= CREGISTERED_MSGS_DEFAULT_BLOCK_SIZE &&
               "Registered more than messages. Please increase CREGISTERED_MSGS_DEFAULT_BLOCK_SIZE in file: ceventhandler.cpp");

        return newId;
    }

    // ========================================================================
    // CEventHandler::Initialize / Shutdown
    // ========================================================================
    void CEventHandler::Initialize()
    {
        s_RegisteredEvents.clear();
        s_EventSubscribers.clear();
        s_NextEventId = 1000;
    }

    void CEventHandler::Shutdown()
    {
        s_RegisteredEvents.clear();
        s_EventSubscribers.clear();
    }
}
