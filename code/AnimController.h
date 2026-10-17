#pragma once

// ============================================================================
// CannonCruise - Animation Controller (AnimController.h)
// Original path: D:\Projects\CannonCruisePC\code\AnimController.h
// Reverse-engineered via Ghidra and CC.asm analysis.
// Binary ref: FUN_00401300 - FUN_00401600 (CAnimController methods)
// ============================================================================

#include <rwcore.h>
#include <rpworld.h>
#include <rphanim.h>
#include "framework/core/attributehandler/cattributehandler.h"
#include <vector>

namespace RWS
{

/**
 * @class CAnimController
 * @brief RenderWare HAnimHierarchy-based animation playback and blending controller.
 *
 * This controller, included in classes like CActor, manages RpHAnimAnimation
 * objects, sets speed/blend values, and updates every frame.
 * Blending multiple animations simultaneously is supported.
 *
 * **Memory layout:**
 * - `m_anims`        — animation list
 * - `m_blends`       — blend list
 * - `m_pHierarchy`   — RW HAnimHierarchy pointer
 * - `m_currentAnim`  — active animation index
 * - `m_currentTime`  — animation timer (seconds)
 *
 * **Binary references:**
 * - Ctor: `FUN_00401300`
 * - SetTarget: `FUN_00401350`
 * - AddAnimation: `FUN_00401400`
 * - Update: `FUN_00401580`
 */
class CAnimController
{
public:
    /**
     * @brief Animation record — playback speed, flags, and ID.
     */
    struct AnimEntry
    {
        RpHAnimAnimation* pAnim;  ///< RW animation source
        RwReal speed;             ///< Playback speed multiplier (1.0 = normal)
        RwInt32 flags;            ///< Animation flags (loop, one-shot, etc.)
        RwUInt32 id;              ///< Animation ID assigned by game
    };

    /**
     * @brief Blend record — hierarchy, animation index, and weight.
     */
    struct BlendEntry
    {
        RpHAnimHierarchy* pHierarchy; ///< Target hierarchy
        RwInt32 animIndex;            ///< Index of blended animation in list
        RwReal weight;                ///< Blend weight (0.0–1.0)
    };

    /**
     * @brief Constructor — initializes empty controller.
     */
    CAnimController();

    /**
     * @brief Destructor — releases all animation resources.
     */
    virtual ~CAnimController();

    /**
     * @brief Returns whether animation target is set.
     * @return RwBool — TRUE if m_pHierarchy valid.
     */
    RwBool HasTarget() const { return m_pHierarchy != 0; }

    /**
     * @brief Assigns an RpClump as animation target.
     * @param pClump 3D model to apply animation to.
     */
    void SetTarget(RpClump* pClump);

    /**
     * @brief Adds animation source to list and returns its index.
     * @param animId Animation ID assigned by game.
     * @return RwInt32 — index of added AnimEntry.
     */
    RwInt32 AddAnimation(RwInt32 animId);

    /**
     * @brief Returns animation pointer by index.
     * @param index Index in AnimEntry list.
     * @return RpHAnimAnimation* — animation pointer or nullptr.
     */
    RpHAnimAnimation* GetAnimation(RwInt32 index);

    /**
     * @brief Searches and returns animation by ID value.
     * @param animId Animation ID to search for.
     * @return RpHAnimAnimation* — found animation or nullptr.
     */
    RpHAnimAnimation* FindAnimation(RwInt32 animId);

    /**
     * @brief Processes animation data from RWS attribute packet.
     * @param packet Packet containing animation ID or speed value.
     */
    void HandleAttributes(const CAttributePacket& packet);

    /**
     * @brief Loads animation resource by name and adds to list.
     * @param name Source file name (e.g. "run.anm").
     */
    void AddAnimResource(const RwChar* name);

    /**
     * @brief Sets playback speed for a specific animation.
     * @param index Index in AnimEntry list.
     * @param speed New speed multiplier.
     */
    void SetAnimSpeed(RwInt32 index, RwReal speed);

    /**
     * @brief Called every frame; updates animation time and hierarchy.
     * @param deltaTime Time elapsed since last frame (seconds).
     */
    void Update(RwReal deltaTime);

private:
    std::vector<AnimEntry> m_anims;    ///< Loaded animation list
    std::vector<BlendEntry> m_blends;  ///< Active blend list
    RpHAnimHierarchy* m_pHierarchy;    ///< RW animation hierarchy
    RwInt32 m_currentAnim;             ///< Active animation index
    RwReal m_currentTime;              ///< Current animation time (seconds)
};

} // namespace RWS
