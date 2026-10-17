// ============================================================================
// CannonCruise - Collision Filter (CollisionFilter.cpp)
// Original path: D:\Projects\CannonCruisePC\code\CollisionFilter.cpp
// Reverse-engineered via Ghidra and CC.asm analysis.
//
// Binary functions:
//   FUN_00434cb0 — CCollisionFilter::ShouldCollide
// ============================================================================

#include "CollisionFilter.h"
#include <cassert>
#include <cstring>

RwBool CCollisionFilter::sm_CollisionMatrix[CFE_TYPE_COUNT][CFE_TYPE_COUNT];

void CCollisionFilter::Initialize()
{
    memset(sm_CollisionMatrix, 0, sizeof(sm_CollisionMatrix));

    // Collision Rules Matrix
    // Player ship collides with static world, enemy ships, mines, debris and triggers
    sm_CollisionMatrix[CFE_PLAYER_SHIP][CFE_WORLD_STATIC] = TRUE;
    sm_CollisionMatrix[CFE_PLAYER_SHIP][CFE_ENEMY_SHIP] = TRUE;
    sm_CollisionMatrix[CFE_PLAYER_SHIP][CFE_CANNONBALL] = TRUE;
    sm_CollisionMatrix[CFE_PLAYER_SHIP][CFE_MINE] = TRUE;
    sm_CollisionMatrix[CFE_PLAYER_SHIP][CFE_DEBRIS] = TRUE;
    sm_CollisionMatrix[CFE_PLAYER_SHIP][CFE_FISH] = TRUE;
    sm_CollisionMatrix[CFE_PLAYER_SHIP][CFE_TRIGGER] = TRUE;

    // Enemy ship rules
    sm_CollisionMatrix[CFE_ENEMY_SHIP][CFE_WORLD_STATIC] = TRUE;
    sm_CollisionMatrix[CFE_ENEMY_SHIP][CFE_PLAYER_SHIP] = TRUE;
    sm_CollisionMatrix[CFE_ENEMY_SHIP][CFE_CANNONBALL] = TRUE;

    // Cannonball rules
    sm_CollisionMatrix[CFE_CANNONBALL][CFE_WORLD_STATIC] = TRUE;
    sm_CollisionMatrix[CFE_CANNONBALL][CFE_PLAYER_SHIP] = TRUE;
    sm_CollisionMatrix[CFE_CANNONBALL][CFE_ENEMY_SHIP] = TRUE;

    // Simetrik matrisi doldur
    for (int i = 0; i < CFE_TYPE_COUNT; ++i)
    {
        for (int j = 0; j < CFE_TYPE_COUNT; ++j)
        {
            if (sm_CollisionMatrix[i][j])
            {
                sm_CollisionMatrix[j][i] = TRUE;
            }
        }
    }
}

// ============================================================================
// CCollisionFilter::ShouldCollide (FUN_00434cb0)
// ASM: 00434cb0 — Filter collision between two layers
// ============================================================================
RwBool CCollisionFilter::ShouldCollide(RwUInt32 infoA, RwUInt32 infoB)
{
    assert(infoA < CFE_TYPE_COUNT && "infoA < CFE_TYPE_COUNT Failed");
    assert(infoB < CFE_TYPE_COUNT && "Illegal ECollisionFilterEntityType !");

    return sm_CollisionMatrix[infoA][infoB];
}
