// ============================================================================
// CannonCruise - WaveBreaker Third-Person Camera (WbCamera.cpp)
// Original path: D:\Projects\CannonCruisePC\code\WbCamera.cpp
// Tersine mühendislik ile Ghidra ve CC.asm analiz edilerek yeniden yazılmıştır.
//
// Binary Fonksiyonları:
//   FUN_0042a100 — CWbCamera constructor & registration
//   FUN_0042a820 — CWbCamera::Update
//   FUN_0042af00 — CWbCamera::SetTargetFrame
// ============================================================================

#include "WbCamera.h"
#include "framework/core/attributehandler/cclassfactory.h"
#include <cassert>
#include <cmath>

RWS_REGISTER_CLASS(CWbCamera, []() -> void* { return new CWbCamera(); }, sizeof(CWbCamera));

/**
 * @brief Constructs a new CWbCamera instance.
 *
 * Configures default follow distance (25 units), elevation (10 units), and damping factor.
 *
 * @see FUN_0042a100
 */
CWbCamera::CWbCamera()
    : m_pCamera(0),
      m_pTargetFrame(0),
      m_fDistance(25.0f),
      m_fElevation(10.0f),
      m_fDamping(5.0f)
{
    m_CurrentPos.x = 0.0f;
    m_CurrentPos.y = 10.0f;
    m_CurrentPos.z = -25.0f;

    m_LookAtPos.x = 0.0f;
    m_LookAtPos.y = 0.0f;
    m_LookAtPos.z = 0.0f;
}

/**
 * @brief Destructor.
 */
CWbCamera::~CWbCamera()
{
}

/**
 * @brief Handles camera event notifications.
 * @param msg Event message container.
 */
void CWbCamera::HandleEvents(const RWS::CMsg& msg)
{
}

/**
 * @brief Binds camera chase target to a ship/entity transformation frame.
 *
 * @param pTargetFrame Transform frame to track.
 * @see FUN_0042af00
 */
void CWbCamera::SetTargetFrame(RwFrame* pTargetFrame)
{
    m_pTargetFrame = pTargetFrame;
    if (m_pTargetFrame)
    {
        RwMatrix* pMatrix = RwFrameGetMatrix(m_pTargetFrame);
        if (pMatrix)
        {
            m_LookAtPos = pMatrix->pos;
            m_CurrentPos = pMatrix->pos;
            m_CurrentPos.y += m_fElevation;
            m_CurrentPos.z -= m_fDistance;
        }
    }
}

/**
 * @brief Smoothly interpolates camera position behind target vessel and points lens.
 *
 * Computes desired position based on ship heading vector, clamps elevation
 * to prevent dipping below water level, and applies camera matrix transform.
 *
 * @param deltaTime Elapsed frame time in seconds.
 * @see FUN_0042a820
 */
void CWbCamera::Update(RwReal deltaTime)
{
    if (!m_pTargetFrame) return;

    RwMatrix* pTargetMatrix = RwFrameGetMatrix(m_pTargetFrame);
    if (!pTargetMatrix) return;

    RwV3d targetPos = pTargetMatrix->pos;
    RwV3d forwardVec = pTargetMatrix->at;

    // Hedefin arkasında istenen kamera konumu
    RwV3d desiredPos;
    desiredPos.x = targetPos.x - forwardVec.x * m_fDistance;
    desiredPos.y = targetPos.y + m_fElevation;
    desiredPos.z = targetPos.z - forwardVec.z * m_fDistance;

    // Su seviyesinin altına inmemesi için Y kontrolü
    if (desiredPos.y < 2.0f)
    {
        desiredPos.y = 2.0f;
    }

    // Yumuşak kamera takibi (sönümleme)
    RwReal t = deltaTime * m_fDamping;
    if (t > 1.0f) t = 1.0f;

    m_CurrentPos.x += (desiredPos.x - m_CurrentPos.x) * t;
    m_CurrentPos.y += (desiredPos.y - m_CurrentPos.y) * t;
    m_CurrentPos.z += (desiredPos.z - m_CurrentPos.z) * t;

    m_LookAtPos.x += (targetPos.x - m_LookAtPos.x) * t;
    m_LookAtPos.y += (targetPos.y + 2.0f - m_LookAtPos.y) * t;
    m_LookAtPos.z += (targetPos.z - m_LookAtPos.z) * t;

    if (m_pCamera)
    {
        RwFrame* pCamFrame = RwCameraGetFrame(m_pCamera);
        if (pCamFrame)
        {
            RwFrameTranslate(pCamFrame, &m_CurrentPos, rwCOMBINEREPLACE);
        }
    }
}
