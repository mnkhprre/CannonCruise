#pragma once

// ============================================================================
// CannonCruise - Base Actor (Actor.h)
// Original path: D:\Projects\CannonCruisePC\code\Actor.h
// Reverse-engineered via Ghidra and CC.asm analysis.
// Binary ref: FUN_00401000 - FUN_00401200 (CActor vtable region)
// ============================================================================

#include "AnimController.h"
#include "framework/core/attributehandler/cattributehandler.h"
#include "framework/core/eventhandler/ceventhandler.h"
#include <rpworld.h>
#include <rwcore.h>

namespace RWS {

/**
 * @class CActor
 * @brief Base actor class for RenderWare Studio-based game objects.
 *
 * CActor, derives from `CAttributeHandler` and `CEventHandler` and
 * manages RpClump (3D mesh) and CAnimController (animation).
 * Soldier, Bird, Fish, FernandoActor and all scene entities derive from this class.
 *
 * **Memory layout (binary offset):**
 * - `+0x00` vtable pointer
 * - `+0x04` CAttributeHandler base
 * - `+0x14` CEventHandler base
 * - `+0x24` m_pAnimController
 * - `+0x28` m_pClump
 * - `+0x2C` m_bAddedToWorld
 *
 * **Binary references:**
 * - Ctor: `FUN_00401000`
 * - Dtor: `FUN_00401080`
 * - HandleAttributes: `FUN_00401100`
 * - AddToWorld/RemoveFromWorld: `FUN_00401140`/`FUN_00401180`
 */
class CActor : public CAttributeHandler, public CEventHandler {
public:
  RWS_MAKENEWCLASS(CActor);
  RWS_DECLARE_CLASS(CActor);

  /**
   * @brief Constructor — initializes object with RWS attribute packet.
   * @param packet Attribute packet carrying first clump and transform data.
   *
   * CAnimController is created; if clump data extracted from packet,
   * animation target is assigned. Subscribes to `iMsgRunningTick` message.
   */
  CActor(const CAttributePacket &packet);

  /**
   * @brief Destructor — releases animation controller and clump.
   *
   * Cancels `iMsgRunningTick` registration, calls `RemoveFromWorld()`,
   * then frees CAnimController and RpClump memory.
   */
  virtual ~CActor();

  /**
   * @brief Processes RWS attribute packet; updates clump and animation.
   * @param packet Packet carrying current transform/animation data.
   */
  virtual void HandleAttributes(const CAttributePacket &packet);

  /**
   * @brief Handles incoming event message (e.g. clump change).
   * @param msg CMsg event to process.
   */
  virtual void HandleEvents(CMsg &msg);

  /**
   * @brief Adds clump to RpWorld scene (with duplicate-add protection).
   */
  void AddToWorld();

  /**
   * @brief Removes clump from RpWorld scene.
   */
  void RemoveFromWorld();

  /**
   * @brief Returns this actor's 3D mesh.
   * @return RpClump* — valid mesh pointer or nullptr.
   */
  RpClump *GetClump() const { return m_pClump; }

  /**
   * @brief Returns animation controller.
   * @return CAnimController* — animation controller pointer.
   */
  CAnimController *GetAnimController() const { return m_pAnimController; }

  /**
   * @brief Returns whether actor is added to world.
   * @return RwBool — TRUE if in RpWorld.
   */
  RwBool IsAddedToWorld() const { return m_bAddedToWorld; }

protected:
  CAnimController *m_pAnimController; ///< Animation controller (+0x24)
  RpClump *m_pClump;                  ///< 3D model mesh (+0x28)
  RwBool m_bAddedToWorld;             ///< Added-to-world flag (+0x2C)
};

} // namespace RWS
