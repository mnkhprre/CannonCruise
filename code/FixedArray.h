#ifndef FIXEDARRAY_H
#define FIXEDARRAY_H

// ============================================================================
// CannonCruise - Engine FixedArray Template (FixedArray.h)
// Original path: D:\Projects\CannonCruisePC\code\FixedArray.h
// Tersine mühendislik ile Ghidra ve CC.asm analiz edilerek yeniden yazılmıştır.
// ============================================================================

#include <cassert>
#include <cstring>
#include <rwcore.h>

template <typename T, RwUInt32 SIZE>
class CFixedArray
{
public:
    CFixedArray()
    {
    }

    T& operator[](RwUInt32 index)
    {
        assert(SIZE > 0 && "Can't access array with size 0 !");
        assert(index < SIZE && "CFixedArray() index out of bounds!");
        return m_Data[index];
    }

    const T& operator[](RwUInt32 index) const
    {
        assert(SIZE > 0 && "Can't access array with size 0 !");
        assert(index < SIZE && "CFixedArray() index out of bounds!");
        return m_Data[index];
    }

    RwUInt32 GetSize() const
    {
        return SIZE;
    }

    T* GetData()
    {
        return m_Data;
    }

    const T* GetData() const
    {
        return m_Data;
    }

private:
    T m_Data[SIZE];
};

#endif // FIXEDARRAY_H
