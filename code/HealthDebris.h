#pragma once
#include "Debris.h"

namespace RWS
{
    class CHealthDebris : public CDebris
    {
    public:
        RWS_MAKENEWCLASS(CHealthDebris);
        RWS_DECLARE_CLASS(CHealthDebris);

        CHealthDebris(const CAttributePacket& packet);
        virtual ~CHealthDebris();

        virtual void HandleAttributes(const CAttributePacket& packet);
        virtual void HandleEvents(CMsg& msg);

        virtual void OnCollect();

    private:
        RwReal m_healthAmount;              // 0xb4
        CEventId m_eventHealthRestored;     // 0xb8
    };
}
