#ifndef ASTAR_H
#define ASTAR_H

// ============================================================================
// CannonCruise - A* Grid Pathfinding (Astar.h)
// Original path: D:\Projects\CannonCruisePC\code\Astar.h
// Tersine mühendislik ile Ghidra ve CC.asm analiz edilerek yeniden yazılmıştır.
// Binary ref: FUN_00473800 - FUN_00473f00 (CAstar methods)
// ============================================================================

#include "BinMinTree.h"
#include <rwcore.h>
#include <vector>

/**
 * @struct SPathNode
 * @brief A* algoritması için tek bir ızgara düğümünü temsil eder.
 *
 * Izgara koordinatları (x, z), g/h/f maliyetleri ve üst düğüm işaretçisi içerir.
 * CBinMinTree<SPathNode> açık listesinde öncelik sırasına göre sıralanır.
 */
struct SPathNode {
  RwInt32 x;          ///< Izgaradaki X koordinatı (sütun)
  RwInt32 z;          ///< Izgaradaki Z koordinatı (satır)
  RwReal g;           ///< Başlangıçtan bu düğüme kadar gerçek maliyet
  RwReal h;           ///< Bu düğümden hedefe tahmin edilen maliyet (sezgisel)
  RwReal f;           ///< Toplam maliyet: f = g + h
  SPathNode *pParent; ///< Yol izleme için üst düğüm işaretçisi

  /**
   * @brief Min-heap sıralaması için karşılaştırma operatörü.
   * @param other Karşılaştırılan diğer düğüm.
   * @return Bu düğümün f değeri daha küçükse true.
   */
  bool operator<(const SPathNode &other) const { return f < other.f; }
};

/**
 * @class CAstar
 * @brief 2B ızgara üzerinde A* yol bulma algoritması.
 *
 * CPathFindingMap tarafından kullanılır; NPC AI'larının (Soldier, Bird)
 * engelleri aşarak hedefe ulaşmasını sağlayan yolu hesaplar.
 * Izgara boyutları Initialize() ile belirlenir; her FindPath() çağrısı
 * bağımsız bir yol hesaplaması yapar.
 *
 * **Algoritma:**
 * - Açık liste: CBinMinTree<SPathNode> (min-heap)
 * - Sezgisel: Manhattan mesafesi
 * - Komşular: 4 yönlü (kuzey, güney, doğu, batı)
 *
 * **Binary referansları:**
 * - Ctor: `FUN_00473800`
 * - FindPath: `FUN_004738a0`
 * - Initialize: `FUN_00473850`
 */
class CAstar {
public:
  /**
   * @brief Varsayılan yapıcı — boş ızgara ile başlatır.
   */
  CAstar();

  /**
   * @brief Yıkıcı — açık liste belleğini serbest bırakır.
   */
  ~CAstar();

  /**
   * @brief Izgara boyutlarını ayarlar.
   * @param width  Izgaranın sütun sayısı (X ekseni).
   * @param height Izgaranın satır sayısı (Z ekseni).
   */
  void Initialize(RwInt32 width, RwInt32 height);

  /**
   * @brief Başlangıç noktasından hedef noktaya yol arar.
   * @param startX Başlangıç X koordinatı.
   * @param startZ Başlangıç Z koordinatı.
   * @param endX   Hedef X koordinatı.
   * @param endZ   Hedef Z koordinatı.
   * @param outPath Bulunan yolu 3D dünya koordinatları olarak dolduran çıkış vektörü.
   * @return RwBool — TRUE ise geçerli yol bulundu; FALSE ise ulaşılamaz.
   */
  RwBool FindPath(RwInt32 startX, RwInt32 startZ, RwInt32 endX, RwInt32 endZ,
                  std::vector<RwV3d> &outPath);

private:
  RwInt32 m_iWidth;             ///< Izgara genişliği (sütun sayısı)
  RwInt32 m_iHeight;            ///< Izgara yüksekliği (satır sayısı)
  CBinMinTree<SPathNode> m_OpenList; ///< A* açık listesi (min-heap)
};

#endif // ASTAR_H
