#pragma once
#include <rwcore.h>
#include <rpworld.h>
#include <rphanim.h>
#include "framework/core/attributehandler/cattributehandler.h"
#include <vector>

namespace RWS
{
    class CAnimController
    {
    public:
        struct AnimEntry
        {
            RpHAnimAnimation* pAnim;
            RwReal speed;
            RwInt32 flags;
            RwUInt32 id;
        };

        struct BlendEntry
        {
            RpHAnimHierarchy* pHierarchy;
            RwInt32 animIndex;
            RwReal weight;
        };

        CAnimController();
        virtual ~CAnimController();

        RwBool HasTarget() const { return m_pHierarchy != 0; }
        void SetTarget(RpClump* pClump);
        RwInt32 AddAnimation(RwInt32 animId);
        RpHAnimAnimation* GetAnimation(RwInt32 index);
        RpHAnimAnimation* FindAnimation(RwInt32 animId);
        void HandleAttributes(const CAttributePacket& packet);
        void AddAnimResource(const RwChar* name);
        void SetAnimSpeed(RwInt32 index, RwReal speed);
        void Update(RwReal deltaTime);

    private:
        std::vector<AnimEntry> m_anims;
        std::vector<BlendEntry> m_blends;
        RpHAnimHierarchy* m_pHierarchy;
        RwInt32 m_currentAnim;
        RwReal m_currentTime;
    };
}
