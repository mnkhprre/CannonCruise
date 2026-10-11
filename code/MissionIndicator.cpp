// ============================================================================
// CannonCruise - Mission Objective Indicator (MissionIndicator.cpp)
// Original path: D:\Projects\CannonCruisePC\code\MissionIndicator.cpp
// Tersine mühendislik ile Ghidra ve CC.asm analiz edilerek yeniden yazılmıştır.
//
// Binary Fonksiyonları:
//   FUN_00484eb0 — CMissionIndicator constructor & registration
//   FUN_00485320 — CMissionIndicator::Update
// ============================================================================

#include "MissionIndicator.h"
#include "framework/core/attributehandler/cclassfactory.h"
#include <cassert>
#include <cmath>

RWS_REGISTER_CLASS(CMissionIndicator, []() -> void* { return new CMissionIndicator(); }, sizeof(CMissionIndicator));

/**
 * @brief Constructs a new CMissionIndicator instance.
 *
 * Initializes target position, rotation variables, and marks indicator inactive.
 *
 * @see FUN_00484eb0
 */
CMissionIndicator::CMissionIndicator()
    : m_pClump(0),
      m_fHoverAngle(0.0f),
      m_fRotationYaw(0.0f),
      m_bActive(FALSE)
{
    m_TargetPos.x = 0.0f;
    m_TargetPos.y = 0.0f;
    m_TargetPos.z = 0.0f;
}

/**
 * @brief Destructor. Destroys RenderWare clump if allocated.
 */
CMissionIndicator::~CMissionIndicator()
{
    if (m_pClump)
    {
        RpClumpDestroy(m_pClump);
        m_pClump = 0;
    }
}

/**
 * @brief Handles incoming event messages (e.g. iMsgRenderWorldIndicator).
 * @param msg Event message container.
 */
void CMissionIndicator::HandleEvents(const RWS::CMsg& msg)
{
}

/**
 * @brief Sets the 3D world position coordinates for the marker.
 * @param pPos Pointer to target position vector (asserted non-null).
 */
void CMissionIndicator::SetTargetPosition(const RwV3d* pPos)
{
    assert(pPos != 0);
    m_TargetPos = *pPos;
    m_bActive = TRUE;
}

/**
 * @brief Ticks bobbing hover motion and continuous yaw rotation.
 *
 * @param deltaTime Elapsed frame time in seconds.
 * @see FUN_00485320
 */
void CMissionIndicator::Update(RwReal deltaTime)
{
    if (!m_bActive) return;

    // Hover salınımı ve dönüş güncellemesi
    m_fHoverAngle += deltaTime * 2.5f;
    if (m_fHoverAngle > 6.283185f)
    {
        m_fHoverAngle -= 6.283185f;
    }

    m_fRotationYaw += deltaTime * 1.8f;
    if (m_fRotationYaw > 6.283185f)
    {
        m_fRotationYaw -= 6.283185f;
    }

    if (m_pClump)
    {
        RwFrame* pFrame = RpClumpGetFrame(m_pClump);
        if (pFrame)
        {
            RwV3d pos = m_TargetPos;
            pos.y += 5.0f + sinf(m_fHoverAngle) * 0.75f;
            RwFrameTranslate(pFrame, &pos, rwCOMBINEREPLACE);
            RwFrameRotate(pFrame, &RwFrameGetMatrix(pFrame)->up, m_fRotationYaw * (180.0f / 3.14159265f), rwCOMBINEPOSTCONCAT);
        }
    }
}

/**
 * @brief Renders the 3D objective beacon clump.
 */
void CMissionIndicator::Render()
{
    if (!m_bActive || !m_pClump) return;

    RpClumpRender(m_pClump);
}
