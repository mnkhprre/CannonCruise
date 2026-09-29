#include "eventproxy.h"
#include "framework/core/attributehandler/cclassfactory.h"
#include "framework/core/attributehandler/csystemcommands.h"

namespace RWS
{
    RWS_REGISTER_CLASS(CEventProxy);

    CEventProxy::CEventProxy(const CAttributePacket& packet)
        : CAttributeHandler(packet)
        , CEventHandler(0)
        , m_bEnabled(TRUE)
    {
        m_eventIduna01.SetId("IDUNA01");
        m_eventIduna02.SetId("IDUNA02");
        m_eventIduna03.SetId("IDUNA03");
    }

    CEventProxy::~CEventProxy()
    {
        UnregisterForMessage(m_eventInput);
        UnregisterForMessage(m_eventIduna01);
        UnregisterForMessage(m_eventIduna02);
        UnregisterForMessage(m_eventIduna03);
    }

    void CEventProxy::HandleAttributes(const CAttributePacket& packet)
    {
        CAttributeHandler::HandleAttributes(packet);

        CAttributeCommandIterator it(packet, RWS_CLASS_INDEX(CEventProxy));
        while (!it.IsFinished())
        {
            switch (it->GetCommandId())
            {
            case 0:
                {
                    UnregisterForMessage(m_eventInput);
                    const RwChar* name = reinterpret_cast<const RwChar*>(it->GetCommandData());
                    m_eventInput.SetId(name);
                    RegisterForMessage(m_eventInput);
                }
                break;
            case 1:
                {
                    const RwChar* name = reinterpret_cast<const RwChar*>(it->GetCommandData());
                    m_eventOutput.SetId(name);
                }
                break;
            case 2:
                m_bEnabled = *reinterpret_cast<const RwInt32*>(it->GetCommandData()) != 0;
                break;
            default:
                break;
            }
            ++it;
        }
    }

    void CEventProxy::HandleEvents(CMsg& msg)
    {
        if (!m_bEnabled)
            return;

        if (msg.Id == m_eventInput)
        {
            CMsg outMsg(m_eventOutput, msg.pData);
            SendMsg(outMsg);
        }
        else if (msg.Id == m_eventIduna01 || msg.Id == m_eventIduna02 || msg.Id == m_eventIduna03)
        {
            CMsg outMsg(m_eventOutput, msg.pData);
            SendMsg(outMsg);
        }
    }
}
