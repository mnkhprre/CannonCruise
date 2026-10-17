#ifndef COLLISIONGEOMETRY_H
#define COLLISIONGEOMETRY_H

// ============================================================================
// CannonCruise - Collision Geometry Generator (CollisionGeometry.h)
// Original path: D:\Projects\CannonCruisePC\code\CollisionGeometry.h
// Reverse-engineered via Ghidra and CC.asm analysis.
// ============================================================================

#include <rwcore.h>
#include <rpworld.h>

class CCollisionGeometry
{
public:
    // Create Havok collision shape from RenderWare Clump geometry
    static void* CreateShapeFromClump(RpClump* pClump, RwBool bConvex = TRUE);
    static void* CreateBoxShape(const RwV3d* pHalfExtents);
    static void* CreateSphereShape(RwReal radius);
    static void DestroyShape(void* pShape);
};

#endif // COLLISIONGEOMETRY_H
