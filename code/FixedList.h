#ifndef FIXEDLIST_H
#define FIXEDLIST_H

// ============================================================================
// CannonCruise - Engine FixedList Template (FixedList.h)
// Original path: D:\Projects\CannonCruisePC\code\FixedList.h
// Tersine mühendislik ile Ghidra ve CC.asm analiz edilerek yeniden yazılmıştır.
// ============================================================================

#include <cassert>
#include <rwcore.h>

template <typename T, RwUInt32 CAPACITY>
class CFixedList
{
public:
    CFixedList() : m_Count(0)
    {
    }

    void PushBack(const T& item)
    {
        assert(m_Count < CAPACITY && "FixedList overflow!");
        m_Data[m_Count++] = item;
    }

    void PopBack()
    {
        assert(m_Count > 0 && "FixedList underflow!");
        --m_Count;
    }

    void Clear()
    {
        m_Count = 0;
    }

    RwUInt32 GetCount() const
    {
        return m_Count;
    }

    RwUInt32 GetCapacity() const
    {
        return CAPACITY;
    }

    RwBool IsEmpty() const
    {
        return m_Count == 0;
    }

    RwBool IsFull() const
    {
        return m_Count >= CAPACITY;
    }

    T& operator[](RwUInt32 index)
    {
        assert(index < m_Count && "FixedList index out of bounds!");
        return m_Data[index];
    }

    const T& operator[](RwUInt32 index) const
    {
        assert(index < m_Count && "FixedList index out of bounds!");
        return m_Data[index];
    }

private:
    T m_Data[CAPACITY];
    RwUInt32 m_Count;
};

#endif // FIXEDLIST_H
