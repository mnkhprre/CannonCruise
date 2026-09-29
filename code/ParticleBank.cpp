// ============================================================================
// CannonCruise - Particle Bank Resource (ParticleBank.cpp)
// Original path: D:\Projects\CannonCruisePC\code\ParticleBank.cpp
// Tersine mühendislik ile Ghidra ve CC.asm analiz edilerek yeniden yazılmıştır.
//
// Binary Fonksiyonları:
//   FUN_0044d240 — CParticleBank::Load
// ============================================================================

#include "ParticleBank.h"
#include <cassert>
#include <cstdio>
#include <cstring>

#define PARTICLE_BANK_SIGNATURE "PALLERWa"

CParticleBank::CParticleBank()
{
}

CParticleBank::~CParticleBank()
{
    Unload();
}

// ============================================================================
// CParticleBank::Load (FUN_0044d240)
// ASM: 0044d240 — Parçacık bankası dosyasını oku ve başlığı doğrula
// ============================================================================
RwBool CParticleBank::Load(const char* pFilename)
{
    if (!pFilename) return FALSE;

    FILE* pFile = fopen(pFilename, "rb");
    if (!pFile) return FALSE;

    char szHeader[9];
    memset(szHeader, 0, sizeof(szHeader));
    fread(szHeader, 1, 8, pFile);

    if (strcmp(szHeader, PARTICLE_BANK_SIGNATURE) != 0)
    {
        assert(false && "CParticleBank::Load() - Particle bank file is not recognized");
        fclose(pFile);
        return FALSE;
    }

    RwUInt32 numDefs = 0;
    fread(&numDefs, sizeof(RwUInt32), 1, pFile);

    for (RwUInt32 i = 0; i < numDefs; ++i)
    {
        SParticleDefinition def;
        memset(&def, 0, sizeof(def));
        fread(&def, sizeof(SParticleDefinition), 1, pFile);
        m_Definitions.push_back(def);
    }

    fclose(pFile);
    return TRUE;
}

void CParticleBank::Unload()
{
    m_Definitions.clear();
}

const SParticleDefinition* CParticleBank::FindDefinition(const char* pName) const
{
    if (!pName) return 0;

    for (size_t i = 0; i < m_Definitions.size(); ++i)
    {
        if (strcmp(m_Definitions[i].m_szName, pName) == 0)
        {
            return &m_Definitions[i];
        }
    }
    return 0;
}
