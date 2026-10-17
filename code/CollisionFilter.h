#ifndef COLLISIONFILTER_H
#define COLLISIONFILTER_H

// ============================================================================
// CannonCruise - Collision Filter (CollisionFilter.h)
// Original path: D:\Projects\CannonCruisePC\code\CollisionFilter.h
// Reverse-engineered via Ghidra and CC.asm analysis.
// ============================================================================

#include <rwcore.h>

enum ECollisionFilterEntityType
{
    CFE_WORLD_STATIC = 0,
    CFE_PLAYER_SHIP,
    CFE_ENEMY_SHIP,
    CFE_CANNONBALL,
    CFE_MINE,
    CFE_DEBRIS,
    CFE_FISH,
    CFE_TRIGGER,
    CFE_TYPE_COUNT
};

class CCollisionFilter
{
public:
    static RwBool ShouldCollide(RwUInt32 infoA, RwUInt32 infoB);
    static void Initialize();

private:
    static RwBool sm_CollisionMatrix[CFE_TYPE_COUNT][CFE_TYPE_COUNT];
};

#endif // COLLISIONFILTER_H
