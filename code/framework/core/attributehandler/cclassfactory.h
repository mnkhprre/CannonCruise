#ifndef RWS_CCLASSFACTORY_H
#define RWS_CCLASSFACTORY_H

// ============================================================================
// CannonCruise - RenderWare Studio Framework (cclassfactory.h)
// Original path: D:\Projects\CannonCruisePC\code\framework\core\attributehandler\cclassfactory.h
// Tersine mühendislik ile Ghidra ve CC.asm analiz edilerek yeniden yazılmıştır.
// Binary ref: FUN_00403000 - FUN_004032e0 (RWS::CClassFactory)
// ============================================================================

#include <rwcore.h>
#include <map>
#include <string>

namespace RWS
{
    // Forward declarations
    class CAttributePacket;

    /**
     * @typedef ClassCreationFunc
     * @brief Sınıf örneği oluşturmak için kullanılan fabrika fonksiyon işaretçisi.
     *
     * `CClassFactory::Register()` ile kaydedilir; `Create()` çağrıldığında çağrılır.
     */
    typedef void* (*ClassCreationFunc)(void);

    /**
     * @typedef ClassPacketCreationFunc
     * @brief Özellik paketi ile sınıf örneği oluşturmak için fonksiyon işaretçisi.
     */
    typedef void* (*ClassPacketCreationFunc)(const CAttributePacket& packet);

    /**
     * @struct ClassData
     * @brief Kayıtlı bir sınıfın fabrika verisini tutan yapı.
     */
    struct ClassData
    {
        ClassCreationFunc m_pCreationFunc; ///< Nesne oluşturma fonksiyonu
        RwUInt32          m_Size;          ///< Sınıf boyutu (bayt)
    };

    /**
     * @class CClassFactory
     * @brief RenderWare Studio nesne fabrikası — sınıf adına göre nesne oluşturur.
     *
     * `CAttributeHandler::ProcessPacket()` bir nesne oluşturma paketi aldığında
     * `CClassFactory::Create(pClassName)` çağrılarak uygun sınıf örneği oluşturulur.
     * Tüm oyun sınıfları `RWS_REGISTER_CLASS` makrosu aracılığıyla bu fabrikaya kaydolur.
     *
     * **Kullanım:**
     * ```cpp
     * // Sınıf tanımı içinde:
     * RWS_DECLARE_CLASS(CShip)
     * RWS_MAKENEWCLASS(CShip)
     *
     * // CPP dosyasında:
     * RWS_REGISTER_CLASS(CShip)
     * ```
     *
     * **Binary referansları:**
     * - Create: `FUN_00403000`
     * - Register: `FUN_00403080`
     * - ClassIsRegistered: `FUN_004030f0`
     */
    class CClassFactory
    {
    public:
        enum
        {
            m_AlignmentSize = 64 ///< Nesne bellek hizalaması (bayt)
        };

        /// @return Tekil CClassFactory örneğine işaretçi
        static CClassFactory* Instance();
        /// @brief CClassFactory örneğini siler
        static void Delete();
        /// @brief Fabrikayı kapatır ve kayıtlı tüm sınıfları temizler
        static void Shutdown();

        /**
         * @brief Sınıf adına göre yeni bir nesne oluşturur.
         * @param pClassName Oluşturulacak sınıfın adı (örn. "CShip").
         * @return void* — Oluşturulan nesne işaretçisi (tipe dönüştürülmeli).
         */
        static void* Create(const char* pClassName);

        /**
         * @brief Belirtilen sınıf adının fabrikaya kayıtlı olup olmadığını kontrol eder.
         * @param pClassName Kontrol edilecek sınıf adı.
         * @return TRUE ise sınıf kayıtlı.
         */
        static RwBool ClassIsRegistered(const char* pClassName);

        /**
         * @brief Fabrikaya yeni bir sınıf kaydeder.
         * @param pClassName   Kaydedilecek sınıf adı.
         * @param pCreateFunc  Nesne oluşturma fonksiyonu.
         * @param size         Sınıf boyutu (bayt, isteğe bağlı).
         * @param flags        Kayıt bayrakları (isteğe bağlı).
         * @return void* — Kayıt kaydı işaretçisi (genellikle NULL olmayan sentinel).
         */
        static void* Register(const char* pClassName, ClassCreationFunc pCreateFunc, RwUInt32 size = 0, RwUInt32 flags = 0);

        /**
         * @brief Belirtilen sınıfın fabrika kaydını kaldırır.
         * @param pClassName Kaydı kaldırılacak sınıf adı.
         */
        static void UnRegister(const char* pClassName);

        /// @brief Tüm kayıtlı sınıfları fabrikadan kaldırır
        static void UnRegisterAll();

    private:
        CClassFactory();
        ~CClassFactory();

        static CClassFactory* sm_pInstance; ///< Tekil örnek işaretçisi

        std::map<std::string, ClassData*> m_RegisteredClasses; ///< Ad → ClassData eşleştirme tablosu
    };

    /**
     * @class CAutoRegisterClass
     * @brief Statik başlatma sırasında sınıfı CClassFactory'ye otomatik kaydeden yardımcı.
     *
     * `RWS_REGISTER_CLASS` makrosu bu sınıftan statik bir örnek oluşturarak
     * sınıfın fabrikaya `.cpp` dosyası yüklendiğinde kaydolmasını sağlar.
     */
    class CAutoRegisterClass
    {
    public:
        /**
         * @brief Sınıfı CClassFactory'ye kaydeder.
         * @param pClassName  Kaydedilecek sınıf adı.
         * @param pCreateFunc Nesne oluşturma fonksiyonu.
         * @param size        Sınıf boyutu (isteğe bağlı).
         * @param flags       Kayıt bayrakları (varsayılan: 1).
         */
        CAutoRegisterClass(const char* pClassName, ClassCreationFunc pCreateFunc, RwUInt32 size = 0, RwUInt32 flags = 1)
        {
            CClassFactory::Register(pClassName, pCreateFunc, size, flags);
        }
    };

    /**
     * @def RWS_REGISTER_CLASS(className[, factory[, size]])
     * @brief Sınıfı CClassFactory'ye statik başlatmayla kaydeden makro.
     *
     * Kullanım (codegen):
     * ```cpp
     * RWS_REGISTER_CLASS(CShip)  // ad kaydı — create sonradan BehaviorLoader
     * RWS_REGISTER_CLASS(CShip, []() -> void* { return new CShip(); },
     *                    sizeof(CShip))
     * ```
     * 3-arg biçim fabrika fonksiyonunu gerçekten kaydeder (Create çağırır);
     * 1-arg biçim binary ad-kaydı desenidir (create NULL).
     */
    #define RWS_GET_MACRO(_1, _2, _3, NAME, ...) NAME
    #define RWS_REGISTER_CLASS1(className)                                   \
        static RWS::CAutoRegisterClass s_autoRegister_##className(           \
            #className,                                                      \
            (RWS::ClassCreationFunc)[]() -> void* { return 0; },             \
            sizeof(className))
    #define RWS_REGISTER_CLASS2(className, factory)                          \
        static RWS::CAutoRegisterClass s_autoRegister_##className(           \
            #className, (RWS::ClassCreationFunc)(factory), sizeof(className))
    #define RWS_REGISTER_CLASS3(className, factory, size)                    \
        static RWS::CAutoRegisterClass s_autoRegister_##className(           \
            #className, (RWS::ClassCreationFunc)(factory), (RwUInt32)(size))
    #define RWS_REGISTER_CLASS(...)                                           \
        RWS_GET_MACRO(__VA_ARGS__, RWS_REGISTER_CLASS3,                      \
                      RWS_REGISTER_CLASS2, RWS_REGISTER_CLASS1)(__VA_ARGS__)

    /**
     * @def RWS_MAKENEWCLASS(className)
     * @brief Özellik paketi ile nesne oluşturan statik fabrika metodu tanımlar.
     */
    #define RWS_MAKENEWCLASS(className) \
        static void* MakeNewClass(const RWS::CAttributePacket& packet) { return new className(packet); }

    /**
     * @def RWS_DECLARE_CLASS(className)
     * @brief Sınıf adını döndüren `GetClassName()` metodunu tanımlar.
     */
    #define RWS_DECLARE_CLASS(className) \
        static const char* GetClassName() { return #className; }

    /**
     * @def RWS_CLASS_INDEX(className)
     * @brief Sınıfın özellik paketindeki indeksini döndürür (CannonCruise'da her zaman 0).
     */
    #define RWS_CLASS_INDEX(className) 0
}

#endif // RWS_CCLASSFACTORY_H

