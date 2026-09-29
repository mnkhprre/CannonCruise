#ifndef RWS_CCLASSFACTORY_H
#define RWS_CCLASSFACTORY_H

// ============================================================================
// CannonCruise - RenderWare Studio Framework (cclassfactory.h)
// Original path: D:\Projects\CannonCruisePC\code\framework\core\attributehandler\cclassfactory.h
// ============================================================================

#include <rwcore.h>
#include <map>
#include <string>

namespace RWS
{
    typedef void* (*ClassCreationFunc)(void);

    struct ClassData
    {
        ClassCreationFunc m_pCreationFunc;
        RwUInt32 m_Size;
    };

    class CClassFactory
    {
    public:
        enum
        {
            m_AlignmentSize = 64
        };

        static CClassFactory* Instance();
        static void Delete();
        static void Shutdown();

        static void* Create(const char* pClassName);
        static RwBool ClassIsRegistered(const char* pClassName);
        static void* Register(const char* pClassName, ClassCreationFunc pCreateFunc, RwUInt32 size = 0, RwUInt32 flags = 0);
        static void UnRegister(const char* pClassName);
        static void UnRegisterAll();

    private:
        CClassFactory();
        ~CClassFactory();

        static CClassFactory* sm_pInstance;

        std::map<std::string, ClassData*> m_RegisteredClasses;
    };

    class CAutoRegisterClass
    {
    public:
        CAutoRegisterClass(const char* pClassName, ClassCreationFunc pCreateFunc, RwUInt32 size = 0, RwUInt32 flags = 1)
        {
            CClassFactory::Register(pClassName, pCreateFunc, size, flags);
        }
    };

    #define RWS_REGISTER_CLASS(className, createFunc, classSize) \
        static RWS::CAutoRegisterClass s_autoRegister_##className(#className, (RWS::ClassCreationFunc)createFunc, classSize)
}

#endif // RWS_CCLASSFACTORY_H
