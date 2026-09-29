#ifndef RWS_CRESOURCEMANAGER_H
#define RWS_CRESOURCEMANAGER_H

// ============================================================================
// CannonCruise - RenderWare Studio Framework (cresourcemanager.h)
// Original path: D:\Projects\CannonCruisePC\code\framework\core\resourcemanager\cresourcemanager.h
// Tersine mühendislik ile Ghidra ve CC.asm analiz edilerek yeniden yazılmıştır.
// ============================================================================

#include <rwcore.h>
#include <map>
#include <string>

namespace RWS
{
    struct CResource
    {
        std::string m_Type;
        std::string m_Name;
        std::string m_Path;
        void* m_pData;
        RwUInt32 m_RefCount;
        RwUInt32 m_Flags;

        CResource() : m_pData(0), m_RefCount(0), m_Flags(0) {}
    };

    class CResourceManager
    {
    public:
        static CResourceManager* Instance();
        static void Initialize();
        static void Shutdown();

        static void* FindResource(const char* pResId, const char* pStrType = 0);
        static RwBool RegisterResource(const char* pResId, const char* pStrType, const char* pStrName, void* pData);
        static void UnRegisterResource(const char* pResId);
        static void UnRegisterAll();

    private:
        CResourceManager();
        ~CResourceManager();

        static CResourceManager* sm_pInstance;

        std::map<std::string, CResource> m_Resources;
    };
}

#endif // RWS_CRESOURCEMANAGER_H
