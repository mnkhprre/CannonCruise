// ============================================================================
// CannonCruise - RenderWare Studio Framework (cclassfactory.cpp)
// Original path: D:\Projects\CannonCruisePC\code\framework\core\attributehandler\cclassfactory.cpp
// Tersine mühendislik ile Ghidra ve CC.asm analiz edilerek yeniden yazılmıştır.
//
// Binary Fonksiyonları:
//   FUN_00412870 — CClassFactory::Instance()
//   FUN_00412970 — CClassFactory::Delete() / Shutdown()
//   FUN_004129a0 — CClassFactory::~CClassFactory()
//   FUN_00412a60 — CClassFactory::Create(const char* pClassName)
//   FUN_00412cd0 — CClassFactory::ClassIsRegistered(const char* pClassName)
//   FUN_00412f10 — CClassFactory::Register(...)
//   FUN_00413700 — CClassFactory::Register(...) static wrapper
//   FUN_004139b0 — CClassFactory::UnRegisterAll()
//   FUN_00413ab0 — CClassFactory::Register(...) implementation
//   FUN_00413da0 — CClassFactory::UnRegister(const char* pClassName)
// ============================================================================

#include "cclassfactory.h"
#include <cassert>
#include <cstring>

namespace RWS
{
    // Singleton instance pointer (DAT_00635660)
    CClassFactory* CClassFactory::sm_pInstance = 0;

    // Case-insensitive comparator for class names
    struct CaseInsensitiveCompare
    {
        bool operator()(const std::string& a, const std::string& b) const
        {
#ifdef _WIN32
            return _stricmp(a.c_str(), b.c_str()) < 0;
#else
            return strcasecmp(a.c_str(), b.c_str()) < 0;
#endif
        }
    };

    // ========================================================================
    // CClassFactory Constructor
    // ========================================================================
    CClassFactory::CClassFactory()
    {
    }

    // ========================================================================
    // CClassFactory Destructor (FUN_004129a0)
    // ========================================================================
    CClassFactory::~CClassFactory()
    {
        UnRegisterAll();
    }

    // ========================================================================
    // CClassFactory::Instance (FUN_00412870)
    // ASM: 00412870 — Singleton al / yoksa oluştur (DAT_00635660)
    // ========================================================================
    CClassFactory* CClassFactory::Instance()
    {
        if (sm_pInstance == 0)
        {
            sm_pInstance = new CClassFactory();
        }
        return sm_pInstance;
    }

    // ========================================================================
    // CClassFactory::Delete / Shutdown (FUN_00412970)
    // ASM: 00412970 — Singleton'ı temizle ve sil
    // ========================================================================
    void CClassFactory::Delete()
    {
        if (sm_pInstance != 0)
        {
            delete sm_pInstance;
            sm_pInstance = 0;
        }
    }

    void CClassFactory::Shutdown()
    {
        Delete();
    }

    // ========================================================================
    // CClassFactory::ClassIsRegistered (FUN_00412cd0)
    // ASM: 00412cd0 — Sınıf kayıtlı mı kontrol et
    // ========================================================================
    RwBool CClassFactory::ClassIsRegistered(const char* pClassName)
    {
        if (!pClassName)
        {
            return FALSE;
        }

        CClassFactory* pFactory = Instance();
        for (std::map<std::string, ClassData*>::iterator it = pFactory->m_RegisteredClasses.begin();
             it != pFactory->m_RegisteredClasses.end(); ++it)
        {
#ifdef _WIN32
            if (_stricmp(it->first.c_str(), pClassName) == 0)
#else
            if (strcasecmp(it->first.c_str(), pClassName) == 0)
#endif
            {
                return TRUE;
            }
        }

        return FALSE;
    }

    // ========================================================================
    // CClassFactory::Create (FUN_00412a60)
    // ASM: 00412a60 — Sınıf adından yeni nesne üret
    // ========================================================================
    void* CClassFactory::Create(const char* pClassName)
    {
        if (!pClassName)
        {
            return 0;
        }

        CClassFactory* pFactory = Instance();
        for (std::map<std::string, ClassData*>::iterator it = pFactory->m_RegisteredClasses.begin();
             it != pFactory->m_RegisteredClasses.end(); ++it)
        {
#ifdef _WIN32
            if (_stricmp(it->first.c_str(), pClassName) == 0)
#else
            if (strcasecmp(it->first.c_str(), pClassName) == 0)
#endif
            {
                ClassData* pData = it->second;
                if (pData && pData->m_pCreationFunc)
                {
                    return pData->m_pCreationFunc();
                }
                return 0;
            }
        }

        return 0;
    }

    // ========================================================================
    // CClassFactory::Register (FUN_00412f10, FUN_00413700, FUN_00413ab0)
    // ASM: 00413700 / 00413ab0 — Sınıfı fabrikaya kaydet
    // ========================================================================
    void* CClassFactory::Register(const char* pClassName, ClassCreationFunc pCreateFunc, RwUInt32 size, RwUInt32 flags)
    {
        assert(pClassName != 0 && "Failed PRE-condition: pClassName");
        assert(!ClassIsRegistered(pClassName) && "Failed PRE-condition: !ClassIsRegistered(pClassName)");

        ClassData* pData = new ClassData();
        pData->m_pCreationFunc = pCreateFunc;
        pData->m_Size = 0;

        if (flags != 0)
        {
            pData->m_Size = size;
            // 64-byte hizalama (m_AlignmentSize)
            if ((size & (m_AlignmentSize - 1)) != 0)
            {
                pData->m_Size = (size - (size & (m_AlignmentSize - 1))) + m_AlignmentSize;
            }
        }

        CClassFactory* pFactory = Instance();
        pFactory->m_RegisteredClasses[pClassName] = pData;

        assert(ClassIsRegistered(pClassName) && "Failed POST-condition: ClassIsRegistered(pClassName)");

        return pData;
    }

    // ========================================================================
    // CClassFactory::UnRegister (FUN_00413da0)
    // ASM: 00413da0 — Belirli bir sınıfın kaydını sil
    // ========================================================================
    void CClassFactory::UnRegister(const char* pClassName)
    {
        if (!pClassName)
        {
            return;
        }

        CClassFactory* pFactory = Instance();
        for (std::map<std::string, ClassData*>::iterator it = pFactory->m_RegisteredClasses.begin();
             it != pFactory->m_RegisteredClasses.end(); ++it)
        {
#ifdef _WIN32
            if (_stricmp(it->first.c_str(), pClassName) == 0)
#else
            if (strcasecmp(it->first.c_str(), pClassName) == 0)
#endif
            {
                delete it->second;
                pFactory->m_RegisteredClasses.erase(it);
                break;
            }
        }
    }

    // ========================================================================
    // CClassFactory::UnRegisterAll (FUN_004139b0)
    // ASM: 004139b0 — Tüm kayıtlı sınıfları sil
    // ========================================================================
    void CClassFactory::UnRegisterAll()
    {
        CClassFactory* pFactory = Instance();
        for (std::map<std::string, ClassData*>::iterator it = pFactory->m_RegisteredClasses.begin();
             it != pFactory->m_RegisteredClasses.end(); ++it)
        {
            delete it->second;
        }
        pFactory->m_RegisteredClasses.clear();
    }
}
