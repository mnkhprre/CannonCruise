#ifndef GLOBALFUNCTIONS_H
#define GLOBALFUNCTIONS_H

// ============================================================================
// CannonCruise - Global Helper & Math Utilities (GlobalFunctions.h)
// Original path: D:\Projects\CannonCruisePC\code\GlobalFunctions.h
// Tersine mühendislik ile Ghidra ve CC.asm analiz edilerek yeniden yazılmıştır.
// ============================================================================

#include <rwcore.h>
#include <cmath>

/**
 * @brief Clamps a floating-point value to [minVal, maxVal] range.
 * @param val Input value.
 * @param minVal Lower bound.
 * @param maxVal Upper bound.
 * @return Clamped value.
 */
inline RwReal ClampFloat(RwReal val, RwReal minVal, RwReal maxVal)
{
    if (val < minVal) return minVal;
    if (val > maxVal) return maxVal;
    return val;
}

/**
 * @brief Linear interpolation between two scalar values.
 * @param a Start value.
 * @param b End value.
 * @param t Interpolation factor [0.0, 1.0].
 * @return Interpolated result.
 */
inline RwReal LerpFloat(RwReal a, RwReal b, RwReal t)
{
    return a + (b - a) * t;
}

/**
 * @brief Wraps an angle in radians into the range [-PI, PI].
 * @param angle Input angle in radians.
 * @return Normalized angle in [-PI, PI].
 */
inline RwReal WrapAngle(RwReal angle)
{
    const RwReal kTwoPi = 6.28318530718f;
    const RwReal kPi    = 3.14159265359f;

    while (angle > kPi)  angle -= kTwoPi;
    while (angle < -kPi) angle += kTwoPi;
    return angle;
}

/**
 * @brief Computes 3D Euclidean distance between two points.
 * @param a First 3D vector.
 * @param b Second 3D vector.
 * @return Distance between a and b.
 */
inline RwReal Vec3Distance(const RwV3d& a, const RwV3d& b)
{
    RwReal dx = a.x - b.x;
    RwReal dy = a.y - b.y;
    RwReal dz = a.z - b.z;
    return sqrtf(dx * dx + dy * dy + dz * dz);
}

/**
 * @brief Computes 2D horizontal Euclidean distance (X-Z plane).
 * @param a First 3D vector.
 * @param b Second 3D vector.
 * @param组织 b Second 3D vector.
 * @return Horizontal distance on X-Z plane.
 */
inline RwReal Vec3Distance2D(const RwV3d& a, const RwV3d& b)
{
    RwReal dx = a.x - b.x;
    RwReal dz = a.z - b.z;
    return sqrtf(dx * dx + dz * dz);
}

/**
 * @brief Normalizes a 3D vector in-place.
 * @param v Vector to normalize.
 * @return Length before normalization.
 */
inline RwReal Vec3Normalize(RwV3d* v)
{
    if (!v) return 0.0f;
    RwReal len = sqrtf(v->x * v->x + v->y * v->y + v->z * v->z);
    if (len > 0.00001f)
    {
        RwReal inv = 1.0f / len;
        v->x *= inv;
        v->y *= inv;
        v->z *= inv;
    }
    return len;
}

/**
 * @brief Generates a random float in range [minVal, maxVal].
 * @param minVal Lower bound.
 * @param maxVal Upper bound.
 * @return Random value.
 */
RwReal RandomRangeFloat(RwReal minVal, RwReal maxVal);

/**
 * @brief Generates a random integer in range [minVal, maxVal].
 * @param minVal Lower bound.
 * @param maxVal Upper bound.
 * @return Random integer.
 */
RwInt32 RandomRangeInt(RwInt32 minVal, RwInt32 maxVal);

#endif // GLOBALFUNCTIONS_H
