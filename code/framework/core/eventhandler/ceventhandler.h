#ifndef RWS_CEVENTHANDLER_H
#define RWS_CEVENTHANDLER_H

// ============================================================================
// CannonCruise - RenderWare Studio Framework (ceventhandler.h)
// Original path: D:\Projects\CannonCruisePC\code\framework\core\eventhandler\ceventhandler.h
// Tersine mühendislik ile Ghidra ve CC.asm analiz edilerek yeniden yazılmıştır.
// ============================================================================

#include <rwcore.h>
#include <vector>
#include <string>

namespace RWS
{
    // Orijinal sabitler (assert mesajlarından elde edilmiştir)
    #define CEVENT_HANDLER_REGISTERED_MSG_DEFAULT_BLOCK_SIZE 128
    #define CREGISTERED_MSGS_DEFAULT_BLOCK_SIZE 128

    typedef RwUInt32 CEventId;

    struct CMsg
    {
        CEventId m_Id;
        const void* m_pData;
        RwUInt32 m_Priority;

        CMsg(CEventId id = 0, const void* pData = 0, RwUInt32 priority = 0)
            : m_Id(id), m_pData(pData), m_Priority(priority)
        {
        }
    };

    class CEventHandler
    {
    public:
        CEventHandler();
        virtual ~CEventHandler();

        // Olay işleme sanal fonksiyonu
        virtual void HandleEvents(const CMsg& msg);

        // Olay bağlama / bağlantı kesme
        void LinkMsg(CEventId eventId, const char* pFormat = 0, RwUInt32 priority = 0);
        void UnLinkMsg(CEventId eventId);
        void UnLinkAllProcess();

        // Statik mesaj gönderme & olay yönetimi
        static void SendMsg(const CMsg& msg);
        static CEventId RegisterMsg(const char* pEventName, const char* pFormat = 0);
        static void Initialize();
        static void Shutdown();

    protected:
        struct EventLink
        {
            CEventId m_EventId;
            RwUInt32 m_Priority;
            std::string m_Format;
        };

        std::vector<EventLink> m_LinkedEvents;
    };
}

#endif // RWS_CEVENTHANDLER_H
