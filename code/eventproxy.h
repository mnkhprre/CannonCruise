#pragma once
#include <rwcore.h>
#include <rpworld.h>
#include "framework/core/attributehandler/cattributehandler.h"
#include "framework/core/eventhandler/ceventhandler.h"

namespace RWS
{
    /**
     * CEventProxy - Routes events between entities using named event IDs.
     * Binary ref: strings at 0x00616e38, IDUNA01/02/03 event strings.
     * Xrefs: FUN_00473480 - FUN_00473600
     */
    class CEventProxy : public CAttributeHandler, public CEventHandler
    {
    public:
        RWS_MAKENEWCLASS(CEventProxy);
        RWS_DECLARE_CLASS(CEventProxy);

        CEventProxy(const CAttributePacket& packet);
        virtual ~CEventProxy();

        virtual void HandleAttributes(const CAttributePacket& packet);
        virtual void HandleEvents(CMsg& msg);

    private:
        CEventId m_eventInput;
        CEventId m_eventOutput;
        CEventId m_eventIduna01;    // "IDUNA01"
        CEventId m_eventIduna02;    // "IDUNA02"
        CEventId m_eventIduna03;    // "IDUNA03"
        RwBool m_bEnabled;
    };
}
