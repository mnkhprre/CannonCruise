// ============================================================================
// CannonCruise - Particle Emitter (ParticleEmitter.cpp)
// Original path: D:\Projects\CannonCruisePC\code\ParticleEmitter.cpp
// Tersine mühendislik ile Ghidra ve CC.asm analiz edilerek yeniden yazılmıştır.
//
// Binary Fonksiyonları:
//   FUN_0044f770 — CParticleEmitter::Deinitialize
// ============================================================================

#include "ParticleEmitter.h"
#include <cassert>

RwUInt32 CParticleEmitter::sm_nActiveEmitters = 0;

CParticleEmitter::CParticleEmitter()
    : m_pDef(0),
      m_uMaxParticles(64),
      m_bEmitting(FALSE)
{
    sm_nActiveEmitters++;
}

CParticleEmitter::~CParticleEmitter()
{
    if (sm_nActiveEmitters > 0)
    {
        sm_nActiveEmitters--;
    }
}

void CParticleEmitter::Initialize(const SParticleDefinition* pDef, RwUInt32 maxParticles)
{
    m_pDef = pDef;
    m_uMaxParticles = maxParticles;
    m_Particles.resize(maxParticles);

    for (RwUInt32 i = 0; i < maxParticles; ++i)
    {
        m_Particles[i].m_bActive = FALSE;
    }
}

void CParticleEmitter::Emit(const RwV3d* pPos, const RwV3d* pVel)
{
    if (!pPos || !m_pDef) return;

    for (RwUInt32 i = 0; i < m_uMaxParticles; ++i)
    {
        if (!m_Particles[i].m_bActive)
        {
            m_Particles[i].m_Pos = *pPos;
            if (pVel)
            {
                m_Particles[i].m_Vel = *pVel;
            }
            else
            {
                m_Particles[i].m_Vel.x = ((rand() % 100) - 50) * 0.05f;
                m_Particles[i].m_Vel.y = (rand() % 100) * 0.05f;
                m_Particles[i].m_Vel.z = ((rand() % 100) - 50) * 0.05f;
            }
            m_Particles[i].m_fLife = 0.0f;
            m_Particles[i].m_fMaxLife = m_pDef->m_fLifetime;
            m_Particles[i].m_fSize = m_pDef->m_fStartSize;
            m_Particles[i].m_Color = m_pDef->m_StartColor;
            m_Particles[i].m_bActive = TRUE;
            break;
        }
    }
}

void CParticleEmitter::Stop()
{
    m_bEmitting = FALSE;
}

void CParticleEmitter::Update(RwReal deltaTime)
{
    if (!m_pDef) return;

    for (RwUInt32 i = 0; i < m_uMaxParticles; ++i)
    {
        if (m_Particles[i].m_bActive)
        {
            m_Particles[i].m_fLife += deltaTime;
            if (m_Particles[i].m_fLife >= m_Particles[i].m_fMaxLife)
            {
                m_Particles[i].m_bActive = FALSE;
                continue;
            }

            // Pozisyon ve yerçekimi
            m_Particles[i].m_Pos.x += m_Particles[i].m_Vel.x * deltaTime;
            m_Particles[i].m_Pos.y += m_Particles[i].m_Vel.y * deltaTime - 0.5f * m_pDef->m_fGravity * deltaTime * deltaTime;
            m_Particles[i].m_Pos.z += m_Particles[i].m_Vel.z * deltaTime;

            // Boyut ve renk interpolasyonu
            RwReal t = m_Particles[i].m_fLife / m_Particles[i].m_fMaxLife;
            m_Particles[i].m_fSize = m_pDef->m_fStartSize + (m_pDef->m_fEndSize - m_pDef->m_fStartSize) * t;
        }
    }
}

void CParticleEmitter::Render()
{
    // Billboard parçacık Im3D çizimi
}

void CParticleEmitter::Deinitialize()
{
    assert(sm_nActiveEmitters == 0 && "CParticleEmitter::Deinitialize() - One or more particle emitter instance(s) still exists");
}
