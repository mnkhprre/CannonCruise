#ifndef PARTICLESCRIPT_H
#define PARTICLESCRIPT_H

// ============================================================================
// CannonCruise - Particle Script Controller (ParticleScript.h)
// Original path: D:\Projects\CannonCruisePC\code\ParticleScript.h
// Tersine mühendislik ile Ghidra ve CC.asm analiz edilerek yeniden yazılmıştır.
// ============================================================================

#include "ParticleEmitter.h"
#include <rwcore.h>
#include <vector>

class CParticleScript {
public:
  CParticleScript();
  ~CParticleScript();

  void TriggerEffect(const char *pEffectName, const RwV3d *pPos);
  void Update(RwReal deltaTime);

  static void Deinitialize();
  static RwUInt32 GetActiveScriptCount() { return sm_nActiveScripts; }

private:
  std::vector<CParticleEmitter *> m_Emitters;
  static RwUInt32 sm_nActiveScripts;
};

#endif // PARTICLESCRIPT_H
