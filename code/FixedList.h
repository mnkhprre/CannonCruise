#ifndef FIXEDLIST_H
#define FIXEDLIST_H

// ============================================================================
// CannonCruise - Engine FixedList Template (FixedList.h)
// Original path: D:\Projects\CannonCruisePC\code\FixedList.h
// Reverse-engineered via Ghidra and CC.asm analysis.
// ============================================================================

#include <cassert>
#include <rwcore.h>

/**
 * @class CFixedList
 * @brief Fixed-capacity, no-dynamic-allocation list template.
 *
 * Unlike CFixedArray, tracks active element count (`m_Count`);
 * `PushBack`/`PopBack` add/remove elements. Capacity overflow
 * checked via assert.
 *
 * **Usage example:**
 * ```cpp
 * CFixedList<RwV3d, 32> waypoints;
 * waypoints.PushBack({1.0f, 0.0f, 2.0f});
 * for (RwUInt32 i = 0; i < waypoints.GetCount(); ++i) { ... }
 * ```
 *
 * **Guarantees:**
 * - Capacity overflow: `assert(m_Count < CAPACITY)` (debug)
 * - Underflow: `assert(m_Count > 0)` (debug)
 *
 * @tparam T        Element type stored in list.
 * @tparam CAPACITY Maximum element count (compile-time constant).
 */
template <typename T, RwUInt32 CAPACITY>
class CFixedList
{
public:
    /**
     * @brief Constructor — initializes empty list (m_Count = 0).
     */
    CFixedList() : m_Count(0)
    {
    }

    /**
     * @brief Adds new element to end of list.
     * @param item Element to add.
     * @pre m_Count < CAPACITY (capacity overflow protected by assert)
     */
    void PushBack(const T& item)
    {
        assert(m_Count < CAPACITY && "FixedList overflow!");
        m_Data[m_Count++] = item;
    }

    /**
     * @brief Removes last element from list.
     * @pre m_Count > 0 (underflow protected by assert)
     */
    void PopBack()
    {
        assert(m_Count > 0 && "FixedList underflow!");
        --m_Count;
    }

    /**
     * @brief Clears list (sets m_Count = 0, memory not freed).
     */
    void Clear()
    {
        m_Count = 0;
    }

    /**
     * @brief Returns active element count.
     * @return RwUInt32 — number of valid elements in list.
     */
    RwUInt32 GetCount() const
    {
        return m_Count;
    }

    /**
     * @brief Returns maximum element capacity.
     * @return RwUInt32 — CAPACITY template parameter.
     */
    RwUInt32 GetCapacity() const
    {
        return CAPACITY;
    }

    /**
     * @brief Returns whether list is empty.
     * @return RwBool — TRUE if m_Count == 0.
     */
    RwBool IsEmpty() const
    {
        return m_Count == 0;
    }

    /**
     * @brief Returns whether list is full.
     * @return RwBool — TRUE if m_Count >= CAPACITY.
     */
    RwBool IsFull() const
    {
        return m_Count >= CAPACITY;
    }

    /**
     * @brief Returns writable reference to element at specified index.
     * @param index Index of element.
     * @return T& — Reference to requested element.
     * @pre index < m_Count
     */
    T& operator[](RwUInt32 index)
    {
        assert(index < m_Count && "FixedList index out of bounds!");
        return m_Data[index];
    }

    /**
     * @brief Returns const reference to element at specified index.
     * @param index Index of element.
     * @return const T& — Const reference to requested element.
     * @pre index < m_Count
     */
    const T& operator[](RwUInt32 index) const
    {
        assert(index < m_Count && "FixedList index out of bounds!");
        return m_Data[index];
    }

private:
    T m_Data[CAPACITY]; ///< Raw data array
    RwUInt32 m_Count;   ///< Active element count
};

#endif // FIXEDLIST_H
