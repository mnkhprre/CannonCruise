#ifndef FIXEDARRAY_H
#define FIXEDARRAY_H

// ============================================================================
// CannonCruise - Engine FixedArray Template (FixedArray.h)
// Original path: D:\Projects\CannonCruisePC\code\FixedArray.h
// Reverse-engineered via Ghidra and CC.asm analysis.
// ============================================================================

#include <cassert>
#include <cstring>
#include <rwcore.h>

/**
 * @class CFixedArray
 * @brief Fixed-size, compile-time parameterized array template.
 *
 * Wrapper around standard C array for safe access; provides size,
 * index bounds checking and data pointer checks.
 * Requires no dynamic allocation and can be used on stack or as
 * member variable without heap.
 *
 * **Usage example:**
 * ```cpp
 * CFixedArray<RwV3d, 16> pathWaypoints; // fixed array holding 16 waypoints
 * pathWaypoints[0] = { 0.0f, 0.0f, 0.0f };
 * ```
 *
 * **Safety features:**
 * - `operator[]` index bounds checking (with assert)
 * - SIZE > 0 check
 *
 * @tparam T    Element type stored in array.
 * @tparam SIZE Array size (compile-time constant).
 */
template <typename T, RwUInt32 SIZE>
class CFixedArray
{
public:
    /**
     * @brief Default constructor — does not initialize elements (raw memory).
     */
    CFixedArray()
    {
    }

    /**
     * @brief Returns writable reference to element at specified index.
     * @param index Index of element to access (0-based).
     * @return T& — Reference to requested element.
     * @pre index < SIZE
     */
    T& operator[](RwUInt32 index)
    {
        assert(SIZE > 0 && "Can't access array with size 0 !");
        assert(index < SIZE && "CFixedArray() index out of bounds!");
        return m_Data[index];
    }

    /**
     * @brief Returns const reference to element at specified index.
     * @param index Index of element to access (0-based).
     * @return const T& — Const reference to requested element.
     * @pre index < SIZE
     */
    const T& operator[](RwUInt32 index) const
    {
        assert(SIZE > 0 && "Can't access array with size 0 !");
        assert(index < SIZE && "CFixedArray() index out of bounds!");
        return m_Data[index];
    }

    /**
     * @brief Returns fixed array size.
     * @return RwUInt32 — SIZE template parameter.
     */
    RwUInt32 GetSize() const
    {
        return SIZE;
    }

    /**
     * @brief Returns raw data pointer.
     * @return T* — Pointer to first element.
     */
    T* GetData()
    {
        return m_Data;
    }

    /**
     * @brief Returns raw data pointer as const.
     * @return const T* — Const pointer to first element.
     */
    const T* GetData() const
    {
        return m_Data;
    }

private:
    T m_Data[SIZE]; ///< Fixed-size raw data array
};

#endif // FIXEDARRAY_H
