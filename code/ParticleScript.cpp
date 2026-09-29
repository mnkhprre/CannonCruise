// ============================================================================
// CannonCruise - Particle Script Controller (ParticleScript.cpp)
// Original path: D:\Projects\CannonCruisePC\code\ParticleScript.cpp
// Tersine mühendislik ile Ghidra ve CC.asm analiz edilerek yeniden yazılmıştır.
//
// Binary Fonksiyonları:
//   FUN_00454d30 — CParticleScript::Deinitialize
// ============================================================================

#include "ParticleScript.h"
#include <cassert>

RwUInt32 CParticleScript::sm_nActiveScripts = 0;

CParticleScript::CParticleScript()
{
    sm_nActiveScripts++;
}

CParticleScript::~CParticleScript()
{
    for (size_t i = 0; i < m_Emitters.size(); ++i)
    {
        delete m_Emitters[i];
    }
    m_Emitters.clear();

    if (sm_nActiveScripts > 0)
    {
        sm_nActiveScripts--;
    }
}

void CParticleScript::TriggerEffect(const char* pEffectName, const RwV3d* pPos)
{
    if (!pEffectName || !pPos) return;

    // Belirli bir parçacık şablonunu tetikle
}

void CParticleScript::Update(RwReal deltaTime)
{
    for (size_t i = 0; i < m_Emitters.size(); ++i)
    {
        if (m_Emitters[i])
        {
            m_Emitters[i]->Update(deltaTime);
        }
    }
}

void CParticleScript::Deinitialize()
{
    assert(sm_nActiveScripts == 0 && "CParticleScript::Deinitialize() - One or more particle script instance(s) still exists");
}
