#ifndef COLLISIONGEOMETRY_H
#define COLLISIONGEOMETRY_H

// ============================================================================
// CannonCruise - Collision Geometry Generator (CollisionGeometry.h)
// Original path: D:\Projects\CannonCruisePC\code\CollisionGeometry.h
// Tersine mühendislik ile Ghidra ve CC.asm analiz edilerek yeniden yazılmıştır.
// ============================================================================

#include <rwcore.h>
#include <rpworld.h>

class CCollisionGeometry
{
public:
    // RenderWare Clump geometrisinden Havok çarpışma şekli oluştur
    static void* CreateShapeFromClump(RpClump* pClump, RwBool bConvex = TRUE);
    static void* CreateBoxShape(const RwV3d* pHalfExtents);
    static void* CreateSphereShape(RwReal radius);
    static void DestroyShape(void* pShape);
};

#endif // COLLISIONGEOMETRY_H
