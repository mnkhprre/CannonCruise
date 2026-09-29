#pragma once
#include "Flotsam.h"

namespace RWS
{
    class CAmmoCrate : public CFlotsam
    {
    public:
        RWS_MAKENEWCLASS(CAmmoCrate);
        RWS_DECLARE_CLASS(CAmmoCrate);

        CAmmoCrate(const CAttributePacket& packet);
        virtual ~CAmmoCrate();

        virtual void HandleAttributes(const CAttributePacket& packet);
        virtual void HandleEvents(CMsg& msg);

        virtual void OnCollect();

    private:
        RwInt32 m_ammoType;                 // 0xac
        RwInt32 m_ammoCount;                // 0xb0
        CEventId m_eventAmmoCollected;      // 0xb4
    };
}
