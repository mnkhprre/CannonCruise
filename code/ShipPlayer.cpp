// ============================================================================
// CannonCruise - Player Ship Entity (ShipPlayer.cpp)
// Original path: D:\Projects\CannonCruisePC\code\ShipPlayer.cpp
// Tersine muhendislik ile Ghidra ve CC_ida.asm analiz edilerek yeniden yazilmistir.
//
// Binary Fonksiyonlari:
//   sub_4AE6C0 - CClassFactory::Register("CShipPlayer", 0x488)
//   sub_4AE700 - CShipPlayer constructor & event registration
//   sub_4AF3C0 - CShipPlayer per-frame update & physics
// ============================================================================

#include "ShipPlayer.h"
#include <cstdio>
#include <cstring>

// Register CShipPlayer with RenderWare Studio's reflection class factory (sub_4AE6C0)
RWS_REGISTER_CLASS(CShipPlayer, []() -> void* { return new CShipPlayer(); }, sizeof(CShipPlayer));

CShipPlayer *CShipPlayer::sm_pPlayerShip = nullptr;

/**
 * @brief Constructs player ship instance and wires incoming messages.
 * @see sub_4AE700
 */
CShipPlayer::CShipPlayer()
    : m_iStaticScore(0),
      m_iDynamicScore(0),
      m_bStormActive(FALSE),
      m_bRespawning(FALSE),
      m_fRespawnTimer(0.0f)
{
    sm_pPlayerShip = this;
    std::printf("CShipPlayer: instantiated (player vessel registered)\n");
}

/**
 * @brief Destructor. Releases active player ship pointer.
 */
CShipPlayer::~CShipPlayer()
{
    if (sm_pPlayerShip == this)
    {
        sm_pPlayerShip = nullptr;
    }
}

/**
 * @brief Handles incoming event messages (sub_4AE700 / sub_4AF3C0).
 */
void CShipPlayer::HandleEvents(const RWS::CMsg &msg)
{
    // Once taban CShip olaylarini islet (physics tick vb.)
    CShip::HandleEvents(msg);

    if (msg.m_Id == 0) return;

    // Skor ve hava durumu olaylari
    // Gercek entegrasyonda RegisterMsg id'leri ile karsilastirilir
    if (msg.m_pData != nullptr)
    {
        const char *pStr = static_cast<const char *>(msg.m_pData);
        if (std::strcmp(pStr, "iMsgStormStart") == 0)
        {
            m_bStormActive = TRUE;
            std::printf("CShipPlayer: storm started\n");
        }
        else if (std::strcmp(pStr, "iMsgStormStop") == 0)
        {
            m_bStormActive = FALSE;
            std::printf("CShipPlayer: storm stopped\n");
        }
        else if (std::strcmp(pStr, "iMsgFadeOutBeforeRespawn") == 0)
        {
            m_bRespawning = TRUE;
            m_fRespawnTimer = 1.5f;
            std::printf("CShipPlayer: respawn sequence began\n");
        }
        else if (std::strcmp(pStr, "iMsgFadeInAfterRespawn") == 0)
        {
            m_bRespawning = FALSE;
            std::printf("CShipPlayer: respawn sequence completed\n");
        }
    }
}

/**
 * @brief Per-frame update for player ship mechanics.
 */
void CShipPlayer::UpdatePlayer(RwReal deltaTime)
{
    if (m_bRespawning)
    {
        m_fRespawnTimer -= deltaTime;
        if (m_fRespawnTimer <= 0.0f)
        {
            m_bRespawning = FALSE;
        }
        return;
    }

    // Gemi fizigini ve hareketini guncelle
    CShip::Update(deltaTime);
}

/**
 * @brief Respawns vessel at given coordinates and restores hull integrity.
 */
void CShipPlayer::Respawn(const RwV3d &spawnPos)
{
    m_fHealth = m_fMaxHealth;
    m_fCurrentSpeed = 0.0f;
    m_fTargetSpeed = 0.0f;
    m_fRudderAngle = 0.0f;
    m_bSinking = FALSE;
    m_bRespawning = FALSE;

    std::printf("CShipPlayer: respawned at (%.2f, %.2f, %.2f)\n",
                spawnPos.x, spawnPos.y, spawnPos.z);
}