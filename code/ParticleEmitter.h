#ifndef PARTICLEEMITTER_H
#define PARTICLEEMITTER_H

// ============================================================================
// CannonCruise - Particle Emitter (ParticleEmitter.h)
// Original path: D:\Projects\CannonCruisePC\code\ParticleEmitter.h
// Tersine mühendislik ile Ghidra ve CC.asm analiz edilerek yeniden yazılmıştır.
// ============================================================================

#include "ParticleBank.h"
#include <rwcore.h>
#include <vector>

struct SParticle
{
    RwV3d m_Pos;
    RwV3d m_Vel;
    RwReal m_fLife;
    RwReal m_fMaxLife;
    RwReal m_fSize;
    RwRGBA m_Color;
    RwBool m_bActive;
};

class CParticleEmitter
{
public:
    CParticleEmitter();
    ~CParticleEmitter();

    void Initialize(const SParticleDefinition* pDef, RwUInt32 maxParticles = 64);
    void Update(RwReal deltaTime);
    void Render();

    void Emit(const RwV3d* pPos, const RwV3d* pVel = 0);
    void Stop();

    static void Deinitialize();
    static RwUInt32 GetActiveEmitterCount() { return sm_nActiveEmitters; }

private:
    const SParticleDefinition* m_pDef;
    std::vector<SParticle> m_Particles;
    RwUInt32 m_uMaxParticles;
    RwBool m_bEmitting;

    static RwUInt32 sm_nActiveEmitters;
};

#endif // PARTICLEEMITTER_H
