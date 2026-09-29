#ifndef PARTICLEBANK_H
#define PARTICLEBANK_H

// ============================================================================
// CannonCruise - Particle Bank Resource (ParticleBank.h)
// Original path: D:\Projects\CannonCruisePC\code\ParticleBank.h
// Tersine mühendislik ile Ghidra ve CC.asm analiz edilerek yeniden yazılmıştır.
// ============================================================================

#include <rwcore.h>
#include <string>
#include <vector>

struct SParticleDefinition {
  char m_szName[32];
  RwReal m_fLifetime;
  RwReal m_fStartSize;
  RwReal m_fEndSize;
  RwRGBA m_StartColor;
  RwRGBA m_EndColor;
  RwReal m_fSpeed;
  RwReal m_fGravity;
  RwTexture *m_pTexture;
};

class CParticleBank {
public:
  CParticleBank();
  ~CParticleBank();

  RwBool Load(const char *pFilename);
  void Unload();

  const SParticleDefinition *FindDefinition(const char *pName) const;

private:
  std::vector<SParticleDefinition> m_Definitions;
};

#endif // PARTICLEBANK_H
