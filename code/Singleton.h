#ifndef SINGLETON_H
#define SINGLETON_H

// ============================================================================
// CannonCruise - Engine Singleton Template (Singleton.h)
// Original path: D:\Projects\CannonCruisePC\code\Singleton.h
// Tersine mühendislik ile Ghidra ve CC.asm analiz edilerek yeniden yazılmıştır.
// ============================================================================

#include <cassert>

/**
 * @class CSingleton
 * @brief Tek örnek (singleton) tasarım deseni uygulayan CRTP şablon sınıfı.
 *
 * RenderWare Studio altyapısının standart Singleton implementasyonudur.
 * CAudioManager, CSceneManager, CSpeakManager gibi yönetici sınıfları
 * bu şablondan türeyerek tekil örnek garantisi sağlar.
 *
 * **Kullanım örneği:**
 * ```cpp
 * class CAudioManager : public CSingleton<CAudioManager> { ... };
 *
 * // Erişim:
 * CAudioManager::GetSingleton()->PlaySound("hit.wav");
 * ```
 *
 * **Garantiler:**
 * - Ctor: İkinci bir örnek oluşturulursa assert tetiklenir.
 * - Dtor: Henüz yok edilmemiş bir örnek yok edilmeye çalışılırsa assert tetiklenir.
 * - `GetSingleton()`: Örnek yoksa assert tetiklenir.
 * - `GetSingletonPtr()`: Örnek yoksa nullptr döndürür (assert yok).
 *
 * @tparam T Singleton yapılacak sınıf türü (CRTP deseni).
 */
template <typename T>
class CSingleton
{
public:
    /**
     * @brief Yapıcı — statik örnek işaretçisini `this` olarak ayarlar.
     * @pre Henüz bu türde bir örnek oluşturulmamış olmalıdır.
     */
    CSingleton()
    {
        assert(!ms_pSingleton && "!ms_pSingleton Failed");
        ms_pSingleton = static_cast<T*>(this);
    }

    /**
     * @brief Yıkıcı — statik örnek işaretçisini sıfırlar.
     * @pre ms_pSingleton geçerli olmalıdır.
     */
    virtual ~CSingleton()
    {
        assert(ms_pSingleton && "ms_pSingleton Failed");
        ms_pSingleton = 0;
    }

    /**
     * @brief Mevcut singleton örneğine referans döndürür.
     * @return T* — Singleton örneğine işaretçi.
     * @pre Örnek daha önce oluşturulmuş olmalıdır.
     */
    static T* GetSingleton()
    {
        assert(ms_pSingleton != 0 && "ms_pSingleton != NULL Failed");
        return ms_pSingleton;
    }

    /**
     * @brief Mevcut singleton örneğine işaretçi döndürür; assert yapmaz.
     * @return T* — Singleton örneğine işaretçi veya nullptr (örnek yoksa).
     */
    static T* GetSingletonPtr()
    {
        return ms_pSingleton;
    }

protected:
    static T* ms_pSingleton; ///< Tekil örneğe işaretçi (sınıf başına bir adet)

private:
    CSingleton(const CSingleton&);
    CSingleton& operator=(const CSingleton&);
};

template <typename T>
T* CSingleton<T>::ms_pSingleton = 0;

#endif // SINGLETON_H
