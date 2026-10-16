#ifndef HAVOK_H
#define HAVOK_H

// ============================================================================
// CannonCruise - Havok Physics 2.2.1 Shim (havok.h)
//
// Umbrella header — single #include provides all Havok interfaces.
//
// Version: Havok 2.2.1 (E:\nightly_builds\Havok_2_2_1_Branch)
//   - Binary: "** Havok libs built with version [" / "Havok - Build (%d)"
//   - Source: hk221\source\hkbase, hk221\source\hkdyd
//   - Naming: hkWorld / hkEntity / hkRigidBody (no hkp prefix — Havok 4+)
//
// Status: SHIM — REAL implementation (havokimpl.cpp, Phase 4.6h).
//   Naming/inheritance faithful to Havok 2.2.1 (binary fidelity); underlying
//   engine is Bullet 3.25 (MIT). hkWorld uses tested PhysicsBackend
//   world → existing 22 test paths unaffected.
//   Task: havokshim_test — naming, shape types, free fall,
//   static body, contact, phantom overlap, impulse.
// ============================================================================

#include "hkBaseTypes.h"
#include "hkMemory.h"
#include "hkClass.h"
#include "hkWorld.h"
#include "hkEntity.h"
#include "hkPhantom.h"
#include "hkShape.h"
#include "hkConstraint.h"
#include "hkCollisionFilter.h"

#endif // HAVOK_H
