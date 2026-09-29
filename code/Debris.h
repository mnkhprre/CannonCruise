#pragma once
#include "Flotsam.h"

namespace RWS
{
    class CDebris : public CFlotsam
    {
    public:
        RWS_MAKENEWCLASS(CDebris);
        RWS_DECLARE_CLASS(CDebris);

        CDebris(const CAttributePacket& packet);
        virtual ~CDebris();

        virtual void HandleAttributes(const CAttributePacket& packet);
        virtual void HandleEvents(CMsg& msg);

        virtual void Explode();

    protected:
        RwInt32 m_debrisPieces;             // 0xac
        RwInt32 m_explosionParticleId;      // 0xb0
    };
}
