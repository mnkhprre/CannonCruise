#pragma once

// ============================================================================
// CannonCruise - Animation Controller (AnimController.h)
// Original path: D:\Projects\CannonCruisePC\code\AnimController.h
// Tersine mühendislik ile Ghidra ve CC.asm analiz edilerek yeniden yazılmıştır.
// Binary ref: FUN_00401300 - FUN_00401600 (CAnimController methods)
// ============================================================================

#include <rwcore.h>
#include <rpworld.h>
#include <rphanim.h>
#include "framework/core/attributehandler/cattributehandler.h"
#include <vector>

namespace RWS
{

/**
 * @class CAnimController
 * @brief RenderWare HAnimHierarchy tabanlı animasyon oynatma ve harmanlama denetleyicisi.
 *
 * CActor gibi sınıflara dahil edilen bu denetleyici, RpHAnimAnimation nesnelerini
 * yönetir, hız/harmanlama değerlerini ayarlar ve her karede güncelleme yapar.
 * Aynı anda birden fazla animasyonu harmanlama (blend) desteklenir.
 *
 * **Bellek düzeni:**
 * - `m_anims`        — animasyon listesi
 * - `m_blends`       — harmanlama listesi
 * - `m_pHierarchy`   — RW HAnimHierarchy işaretçisi
 * - `m_currentAnim`  — etkin animasyon indeksi
 * - `m_currentTime`  — animasyon zamanlayıcısı (saniye)
 *
 * **Binary referansları:**
 * - Ctor: `FUN_00401300`
 * - SetTarget: `FUN_00401350`
 * - AddAnimation: `FUN_00401400`
 * - Update: `FUN_00401580`
 */
class CAnimController
{
public:
    /**
     * @brief Animasyon kaydı — oynatma hızı, bayraklar ve ID içerir.
     */
    struct AnimEntry
    {
        RpHAnimAnimation* pAnim;  ///< RW animasyon kaynağı
        RwReal speed;             ///< Oynatma hızı çarpanı (1.0 = normal)
        RwInt32 flags;            ///< Animasyon bayrakları (döngü, tek seferlik vb.)
        RwUInt32 id;              ///< Oyun tarafından atanan animasyon ID'si
    };

    /**
     * @brief Harmanlama kaydı — hierarchy, animasyon indeksi ve ağırlık içerir.
     */
    struct BlendEntry
    {
        RpHAnimHierarchy* pHierarchy; ///< Hedef hiyerarşi
        RwInt32 animIndex;            ///< Harmanlanan animasyonun listesi indeksi
        RwReal weight;                ///< Harmanlama ağırlığı (0.0–1.0)
    };

    /**
     * @brief Yapıcı — boş denetleyici başlatır.
     */
    CAnimController();

    /**
     * @brief Yıkıcı — tüm animasyon kaynaklarını serbest bırakır.
     */
    virtual ~CAnimController();

    /**
     * @brief Animasyon hedefinin ayarlanıp ayarlanmadığını döndürür.
     * @return RwBool — TRUE ise m_pHierarchy geçerlidir.
     */
    RwBool HasTarget() const { return m_pHierarchy != 0; }

    /**
     * @brief Animasyon hedefi olarak bir RpClump atar.
     * @param pClump Animasyon uygulanacak 3D model.
     */
    void SetTarget(RpClump* pClump);

    /**
     * @brief Animasyon kaynağını listeye ekler ve indeksini döndürür.
     * @param animId Oyun tarafından atanan animasyon ID'si.
     * @return RwInt32 — eklenen AnimEntry'nin indeksi.
     */
    RwInt32 AddAnimation(RwInt32 animId);

    /**
     * @brief İndekse göre animasyon işaretçisini döndürür.
     * @param index AnimEntry listesindeki indeks.
     * @return RpHAnimAnimation* — animasyon işaretçisi veya nullptr.
     */
    RpHAnimAnimation* GetAnimation(RwInt32 index);

    /**
     * @brief ID değerine göre animasyonu arar ve döndürür.
     * @param animId Aranan animasyon ID'si.
     * @return RpHAnimAnimation* — bulunan animasyon veya nullptr.
     */
    RpHAnimAnimation* FindAnimation(RwInt32 animId);

    /**
     * @brief RWS attribute paketinden animasyon verilerini işler.
     * @param packet Animasyon ID veya hız değeri içeren paket.
     */
    void HandleAttributes(const CAttributePacket& packet);

    /**
     * @brief İsimle animasyon kaynağı yükler ve listeye ekler.
     * @param name Kaynak dosya adı (örn. "run.anm").
     */
    void AddAnimResource(const RwChar* name);

    /**
     * @brief Belirli bir animasyonun oynatma hızını ayarlar.
     * @param index AnimEntry listesindeki indeks.
     * @param speed Yeni hız çarpanı.
     */
    void SetAnimSpeed(RwInt32 index, RwReal speed);

    /**
     * @brief Her karede çağrılır; animasyon zamanını ve hierarchy'yi günceller.
     * @param deltaTime Son kareden bu yana geçen süre (saniye).
     */
    void Update(RwReal deltaTime);

private:
    std::vector<AnimEntry> m_anims;    ///< Yüklü animasyon listesi
    std::vector<BlendEntry> m_blends;  ///< Aktif harmanlama listesi
    RpHAnimHierarchy* m_pHierarchy;    ///< RW animasyon hiyerarşisi
    RwInt32 m_currentAnim;             ///< Etkin animasyonun indeksi
    RwReal m_currentTime;              ///< Mevcut animasyon zamanı (saniye)
};

} // namespace RWS
