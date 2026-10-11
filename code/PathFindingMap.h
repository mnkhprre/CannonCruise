#ifndef PATHFINDINGMAP_H
#define PATHFINDINGMAP_H

// ============================================================================
// CannonCruise - Pathfinding Map (PathFindingMap.h)
// Original path: D:\Projects\CannonCruisePC\code\PathFindingMap.h
// Tersine mühendislik ile Ghidra ve CC.asm analiz edilerek yeniden yazılmıştır.
// Binary ref: FUN_004740c0 - FUN_004744b0 (CPathFindingMap methods)
// ============================================================================

#include "Astar.h"
#include <rwcore.h>
#include <vector>

/**
 * @class CPathFindingMap
 * @brief Seviyenin yürünebilir/engellenmiş hücrelerini depolayan ızgara haritası.
 *
 * Seviye yükleme sırasında ikili dosyadan (`*.pfm`) okunur; her hücre için
 * engellenmiş/serbest durum `m_Grid` bayt dizisinde tutulur.
 * CAstar bu haritayı kullanarak NPC yol bulma işlemi yapar.
 *
 * **Dosya formatı:**
 * - 4 bayt: genişlik (RwInt32)
 * - 4 bayt: yükseklik (RwInt32)
 * - width × height bayt: hücre durumu (0 = serbest, 1 = engellenmiş)
 *
 * **Binary referansları:**
 * - Ctor: `FUN_004740c0`
 * - Load: `FUN_00474100`
 * - IsCellBlocked: `FUN_00474340`
 */
class CPathFindingMap
{
public:
    /**
     * @brief Varsayılan yapıcı — boş harita başlatır.
     */
    CPathFindingMap();

    /**
     * @brief Yıkıcı — ızgara belleğini serbest bırakır.
     */
    ~CPathFindingMap();

    /**
     * @brief Yol bulma haritasını ikili dosyadan yükler.
     * @param pFilename Yüklenecek `.pfm` dosyasının yolu.
     * @return RwBool — TRUE ise başarıyla yüklendi.
     */
    RwBool Load(const char* pFilename);

    /**
     * @brief Harita verilerini bellekten temizler.
     */
    void Unload();

    /**
     * @brief Belirtilen ızgara hücresinin engellenmiş olup olmadığını döndürür.
     * @param x Kontrol edilecek X koordinatı (sütun).
     * @param z Kontrol edilecek Z koordinatı (satır).
     * @return RwBool — TRUE ise bu hücrede engel var.
     */
    RwBool IsCellBlocked(RwInt32 x, RwInt32 z) const;

    /**
     * @brief Haritanın sütun sayısını döndürür.
     * @return RwInt32 — Izgara genişliği.
     */
    RwInt32 GetWidth() const { return m_iWidth; }

    /**
     * @brief Haritanın satır sayısını döndürür.
     * @return RwInt32 — Izgara yüksekliği.
     */
    RwInt32 GetHeight() const { return m_iHeight; }

private:
    RwInt32 m_iWidth;              ///< Izgara sütun sayısı
    RwInt32 m_iHeight;             ///< Izgara satır sayısı
    std::vector<RwUInt8> m_Grid;   ///< Hücre durumları (0=serbest, 1=engel)
    CAstar m_Astar;                ///< Bu harita üzerinde çalışan A* örneği
};

#endif // PATHFINDINGMAP_H
