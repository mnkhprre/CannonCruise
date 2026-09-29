#ifndef SINGLETON_H
#define SINGLETON_H

// ============================================================================
// CannonCruise - Engine Singleton Template (Singleton.h)
// Original path: D:\Projects\CannonCruisePC\code\Singleton.h
// Tersine mühendislik ile Ghidra ve CC.asm analiz edilerek yeniden yazılmıştır.
// ============================================================================

#include <cassert>

template <typename T>
class CSingleton
{
public:
    CSingleton()
    {
        assert(!ms_pSingleton && "!ms_pSingleton Failed");
        ms_pSingleton = static_cast<T*>(this);
    }

    virtual ~CSingleton()
    {
        assert(ms_pSingleton && "ms_pSingleton Failed");
        ms_pSingleton = 0;
    }

    static T* GetSingleton()
    {
        assert(ms_pSingleton != 0 && "ms_pSingleton != NULL Failed");
        return ms_pSingleton;
    }

    static T* GetSingletonPtr()
    {
        return ms_pSingleton;
    }

protected:
    static T* ms_pSingleton;

private:
    CSingleton(const CSingleton&);
    CSingleton& operator=(const CSingleton&);
};

template <typename T>
T* CSingleton<T>::ms_pSingleton = 0;

#endif // SINGLETON_H
