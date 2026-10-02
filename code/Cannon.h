#ifndef CANNON_H
#define CANNON_H

// ============================================================================
// CannonCruise - Cannon Weapon Entity (Cannon.h)
// Original path: D:\Projects\CannonCruisePC\code\Cannon.h
// Tersine mühendislik ile Ghidra ve CC.asm analiz edilerek yeniden yazılmıştır.
// ============================================================================

#include "Cannonball.h"
#include "framework/core/eventhandler/ceventhandler.h"
#include <rwcore.h>
#include <rpworld.h>

/**
 * @class CCannon
 * @brief Represents a single cannon weapon mounted on a ship.
 *
 * Each cannon is bound to a 3-D clump that contains "Gun_Barrel" and
 * "Gun_Base" sub-frames for aiming animations. The Fire() method spawns
 * a cannonball projectile and plays the firing sound effect. Reload timing
 * is tracked internally via m_fReloadTimer.
 */
class CCannon : public RWS::CEventHandler
{
public:
    /** @brief Default constructor – zeroes all pointers and the reload timer. */
    CCannon();

    /** @brief Virtual destructor (does not own the clump). */
    virtual ~CCannon();

    /** @brief Message handler – currently unused but required by CEventHandler. */
    virtual void HandleEvents(const RWS::CMsg& msg);

    /**
     * @brief Binds the cannon to a model and records the owning entity's ID.
     * @param pClump   Pointer to the RpClump that contains barrel/base frames.
     * @param ownerID  Unique identifier of the ship that owns this cannon
     *                 (used to attribute projectile hits).
     */
    void Initialize(RpClump* pClump, RwUInt32 ownerID);

    /**
     * @brief Spawns a cannonball projectile.
     * @param type       The projectile variant (standard, explosive, etc.).
     * @param pDirection Normalised fire direction in world space.
     * @param speed      Initial projectile speed (default 50 units/sec).
     *
     * Determines the muzzle position from the barrel frame (or clump root
     * as fallback), scales the direction by speed to produce a velocity
     * vector, then fills an SCannonballSpawnData struct and plays the
     * "Audio_CannonFire" positional sound.
     */
    void Fire(ECannonballType type, const RwV3d* pDirection, RwReal speed = 50.0f);

    /**
     * @brief Adjusts the cannon's barrel elevation and base azimuth.
     * @param elevation  Vertical angle (radians, positive = up).
     * @param azimuth    Horizontal angle (radians, positive = clockwise).
     *
     * Implementation stub – barrel and base frame rotations will be applied
     * here once the full animation data is restored.
     */
    void Aim(RwReal elevation, RwReal azimuth);

private:
    RpClump* m_pClump;          ///< The model containing barrel/base frames (not owned).
    RwFrame* m_pGunBarrelFrame; ///< Cached frame for the gun barrel tip (muzzle position).
    RwFrame* m_pGunBaseFrame;   ///< Cached frame for the gun swivel base.
    RwUInt32 m_uOwnerID;        ///< ID of the entity that owns this cannon.
    RwReal m_fReloadTimer;      ///< Countdown timer preventing rapid re-firing.
};

#endif // CANNON_H
