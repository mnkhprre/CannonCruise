#ifndef WBCAMERA_H
#define WBCAMERA_H

// ============================================================================
// CannonCruise - WaveBreaker Third-Person Camera (WbCamera.h)
// Original path: D:\Projects\CannonCruisePC\code\WbCamera.h
// Reverse-engineered via Ghidra and CC.asm analysis.
// ============================================================================

#include "framework/core/eventhandler/ceventhandler.h"
#include <rwcore.h>

/**
 * @class CWbCamera
 * @brief Third-person naval chase camera controller ("WaveBreaker Camera").
 *
 * Smoothly follows the player's ship with spring damping, adjusts elevation
 * to remain above ocean wave swells, and aligns viewport orientation.
 */
class CWbCamera : public RWS::CEventHandler
{
public:
    /**
     * @brief Constructs a CWbCamera instance.
     * @see FUN_0042a100
     */
    CWbCamera();

    /**
     * @brief Destructor.
     */
    virtual ~CWbCamera();

    /**
     * @brief Handles camera event notifications.
     * @param msg Event message container.
     */
    virtual void HandleEvents(const RWS::CMsg& msg);

    /**
     * @brief Attaches camera tracking to a target frame (player ship).
     * @param pTargetFrame Target transform frame.
     */
    void SetTargetFrame(RwFrame* pTargetFrame);

    /**
     * @brief Updates camera tracking position and smooth damping.
     * @param deltaTime Elapsed frame time in seconds.
     * @see FUN_0042a820
     */
    void Update(RwReal deltaTime);

    /**
     * @brief Sets chase distance behind target vessel.
     * @param distance Distance in world units.
     */
    void SetDistance(RwReal distance) { m_fDistance = distance; }

    /**
     * @brief Sets camera height offset above water plane.
     * @param elevation Height offset in world units.
     */
    void SetElevation(RwReal elevation) { m_fElevation = elevation; }

    /**
     * @brief Retrieves active camera frame.
     * @return Pointer to RwCamera or NULL.
     */
    RwCamera* GetCamera() const { return m_pCamera; }

private:
    RwCamera* m_pCamera;      ///< Managed RenderWare camera
    RwFrame*  m_pTargetFrame; ///< Target frame being followed (ship frame)
    RwV3d     m_CurrentPos;   ///< Filtered camera world position
    RwV3d     m_LookAtPos;    ///< Target look-at point
    RwReal    m_fDistance;    ///< Follow distance behind vessel
    RwReal    m_fElevation;   ///< Height elevation above target
    RwReal    m_fDamping;     ///< Position smoothing lag factor
};

#endif // WBCAMERA_H
