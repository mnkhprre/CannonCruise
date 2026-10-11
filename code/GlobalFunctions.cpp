// ============================================================================
// CannonCruise - Global Helper & Math Utilities (GlobalFunctions.cpp)
// Original path: D:\Projects\CannonCruisePC\code\GlobalFunctions.cpp
// Tersine mühendislik ile Ghidra ve CC.asm analiz edilerek yeniden yazılmıştır.
//
// Binary Fonksiyonları:
//   FUN_00401200 — RandomRangeFloat
//   FUN_00401260 — RandomRangeInt
// ============================================================================

#include "GlobalFunctions.h"
#include <cstdlib>

/**
 * @brief Returns a pseudo-random floating-point number between minVal and maxVal.
 *
 * @param minVal Minimum possible output value.
 * @param maxVal Maximum possible output value.
 * @return Interpolated random float in [minVal, maxVal].
 * @see FUN_00401200
 */
RwReal RandomRangeFloat(RwReal minVal, RwReal maxVal)
{
    if (minVal >= maxVal) return minVal;
    RwReal fraction = (RwReal)rand() / (RwReal)RAND_MAX;
    return minVal + fraction * (maxVal - minVal);
}

/**
 * @brief Returns a pseudo-random integer between minVal and maxVal (inclusive).
 *
 * @param minVal Minimum integer bound.
 * @param maxVal Maximum integer bound.
 * @return Random integer in [minVal, maxVal].
 * @see FUN_00401260
 */
RwInt32 RandomRangeInt(RwInt32 minVal, RwInt32 maxVal)
{
    if (minVal >= maxVal) return minVal;
    return minVal + (rand() % (maxVal - minVal + 1));
}
