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
    // Forward declarations
    class CAttributePacket;

    typedef void* (*ClassCreationFunc)(void);
    typedef void* (*ClassPacketCreationFunc)(const CAttributePacket& packet);

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

    #define RWS_REGISTER_CLASS(className, ...) \
        static RWS::CAutoRegisterClass s_autoRegister_##className(#className, (RWS::ClassCreationFunc)[]() -> void* { return 0; }, sizeof(className))

    #define RWS_MAKENEWCLASS(className) \
        static void* MakeNewClass(const RWS::CAttributePacket& packet) { return new className(packet); }

    #define RWS_DECLARE_CLASS(className) \
        static const char* GetClassName() { return #className; }

    #define RWS_CLASS_INDEX(className) 0
}

#endif // RWS_CCLASSFACTORY_H

